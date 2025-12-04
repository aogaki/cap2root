#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <kj/io.h>
#include "eventProto.capnp.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file.cap>\n";
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        std::cerr << "Cannot open file\n";
        return 1;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    std::cout << "File size: " << file_size << " bytes ("
              << (file_size / (1024.0 * 1024.0)) << " MB)\n\n";

    kj::FdInputStream fdStream(fd);
    kj::BufferedInputStreamWrapper bufferedStream(fdStream);

    int messageCount = 0;
    int totalEvents = 0;

    try {
        while (bufferedStream.tryGetReadBuffer() != nullptr) {
            off_t start_pos = lseek(fd, 0, SEEK_CUR);

            capnp::PackedMessageReader message(bufferedStream, {100000000, 64});

            // First, read as PlainData to get the type
            auto data = message.getRoot<PlainData>();
            int evtType = data.getType();

            std::cout << "Message " << messageCount << ": type=" << (int)evtType;

            // Now read the actual events based on type
            int eventCount = 0;
            switch (evtType) {
                case 0: {  // PlainData
                    auto events = data.getEvents();
                    eventCount = events.size();
                    std::cout << " (PlainData), events=" << eventCount << "\n";
                    break;
                }
                case 1: {  // PsdData
                    auto psdData = message.getRoot<PsdData>();
                    auto events = psdData.getEvents();
                    eventCount = events.size();
                    std::cout << " (PsdData), events=" << eventCount << "\n";
                    break;
                }
                case 2: {  // WaveData
                    auto waveData = message.getRoot<WaveData>();
                    auto events = waveData.getEvents();
                    eventCount = events.size();
                    std::cout << " (WaveData), events=" << eventCount;
                    if (eventCount > 0) {
                        std::cout << ", wave size=" << events[0].getWaveform1().size();
                    }
                    std::cout << "\n";
                    break;
                }
                case 3: {  // DualWaveData
                    auto dwData = message.getRoot<DualWaveData>();
                    auto events = dwData.getEvents();
                    eventCount = events.size();
                    std::cout << " (DualWaveData), events=" << eventCount << "\n";
                    break;
                }
                case 4: {  // FullData
                    auto fullData = message.getRoot<FullData>();
                    auto events = fullData.getEvents();
                    eventCount = events.size();
                    std::cout << " (FullData), events=" << eventCount << "\n";
                    break;
                }
                case 5: {  // RawTimeData
                    auto rtData = message.getRoot<RawTimeData>();
                    auto events = rtData.getEvents();
                    eventCount = events.size();
                    std::cout << " (RawTimeData), events=" << eventCount << "\n";
                    break;
                }
                default:
                    std::cout << " (Unknown type!)\n";
                    break;
            }

            totalEvents += eventCount;
            messageCount++;

            off_t end_pos = lseek(fd, 0, SEEK_CUR);
            if (messageCount <= 5) {
                std::cout << "  Position: " << start_pos << " -> " << end_pos
                          << " (" << (end_pos - start_pos) << " bytes)\n";
            }

            if (messageCount % 1000 == 0) {
                std::cout << "... processed " << messageCount << " messages, "
                          << totalEvents << " events\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n";
    }

    close(fd);

    std::cout << "\n=== FINAL TOTALS ===\n";
    std::cout << "Total messages: " << messageCount << "\n";
    std::cout << "Total events: " << totalEvents << "\n";
    if (messageCount > 0) {
        std::cout << "Average events per message: " << (double)totalEvents / messageCount << "\n";
    }

    return 0;
}
