#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
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

    int packetCount = 0;
    int totalEvents = 0;

    try {
        while (true) {
            // Try to read a packet
            char peek;
            ssize_t n = read(fd, &peek, 1);
            if (n <= 0) {
                break;  // EOF
            }
            lseek(fd, -1, SEEK_CUR);

            capnp::PackedFdMessageReader message(fd);
            auto packet = message.getRoot<DataPacket>();

            int eventCount = 0;
            switch (packet.which()) {
                case DataPacket::PLAIN_EVENTS:
                    eventCount = packet.getPlainEvents().size();
                    std::cout << "Packet " << packetCount << ": PlainEvent, " << eventCount << " events\n";
                    break;
                case DataPacket::PSD_EVENTS:
                    eventCount = packet.getPsdEvents().size();
                    std::cout << "Packet " << packetCount << ": PsdEvent, " << eventCount << " events\n";
                    break;
                case DataPacket::WAVE_EVENTS:
                    eventCount = packet.getWaveEvents().size();
                    std::cout << "Packet " << packetCount << ": WaveEvent, " << eventCount << " events\n";
                    break;
                case DataPacket::DUAL_WAVE_EVENTS:
                    eventCount = packet.getDualWaveEvents().size();
                    std::cout << "Packet " << packetCount << ": DualWaveEvent, " << eventCount << " events\n";
                    break;
                case DataPacket::RAW_TIME_EVENTS:
                    eventCount = packet.getRawTimeEvents().size();
                    std::cout << "Packet " << packetCount << ": RawTimeEvent, " << eventCount << " events\n";
                    break;
                case DataPacket::CROSS_EVENTS:
                    eventCount = packet.getCrossEvents().size();
                    std::cout << "Packet " << packetCount << ": CrossEvent, " << eventCount << " events\n";
                    break;
                case DataPacket::PSD_WAVE_EVENTS:
                    eventCount = packet.getPsdWaveEvents().size();
                    std::cout << "Packet " << packetCount << ": PsdWaveEvent, " << eventCount << " events\n";
                    break;
                case DataPacket::FULL_EVENTS:
                    eventCount = packet.getFullEvents().size();
                    std::cout << "Packet " << packetCount << ": FullEvent, " << eventCount << " events\n";
                    break;
                default:
                    std::cout << "Packet " << packetCount << ": Unknown type\n";
                    break;
            }

            totalEvents += eventCount;
            packetCount++;

            if (packetCount % 1000 == 0) {
                std::cout << "... processed " << packetCount << " packets, " << totalEvents << " events so far\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error or EOF: " << e.what() << "\n";
    }

    close(fd);

    std::cout << "\n=== FINAL TOTALS ===\n";
    std::cout << "Total packets: " << packetCount << "\n";
    std::cout << "Total events: " << totalEvents << "\n";
    std::cout << "Average events per packet: " << (packetCount > 0 ? (double)totalEvents / packetCount : 0) << "\n";

    return 0;
}
