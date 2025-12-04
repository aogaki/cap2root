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

    int messageCount = 0;
    int totalEvents = 0;

    std::cout << "Trying to read as PlainData structure...\n\n";

    try {
        while (true) {
            char peek;
            ssize_t n = read(fd, &peek, 1);
            if (n <= 0) {
                break;
            }
            lseek(fd, -1, SEEK_CUR);

            capnp::PackedFdMessageReader message(fd);
            auto data = message.getRoot<PlainData>();

            auto events = data.getEvents();
            std::cout << "Message " << messageCount << ": type=" << (int)data.getType()
                      << ", events=" << events.size() << "\n";

            totalEvents += events.size();
            messageCount++;

            if (messageCount % 100 == 0) {
                std::cout << "... processed " << messageCount << " messages, " << totalEvents << " events\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error or EOF: " << e.what() << "\n";
    }

    close(fd);

    std::cout << "\n=== FINAL TOTALS ===\n";
    std::cout << "Total messages: " << messageCount << "\n";
    std::cout << "Total events: " << totalEvents << "\n";

    return 0;
}
