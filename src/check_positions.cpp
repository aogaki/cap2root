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

    // Get file size
    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    std::cout << "File size: " << file_size << " bytes ("
              << (file_size / (1024.0 * 1024.0)) << " MB)\n\n";

    int messageCount = 0;

    try {
        while (true) {
            off_t start_pos = lseek(fd, 0, SEEK_CUR);
            std::cout << "Message " << messageCount << " starts at offset: " << start_pos << "\n";

            char peek;
            ssize_t n = read(fd, &peek, 1);
            if (n <= 0) {
                std::cout << "EOF reached at offset: " << start_pos << "\n";
                break;
            }
            lseek(fd, -1, SEEK_CUR);

            capnp::PackedFdMessageReader message(fd);
            auto packet = message.getRoot<DataPacket>();

            off_t end_pos = lseek(fd, 0, SEEK_CUR);
            std::cout << "Message " << messageCount << " ends at offset: " << end_pos << "\n";
            std::cout << "Message " << messageCount << " size: " << (end_pos - start_pos) << " bytes\n";

            int eventCount = 0;
            switch (packet.which()) {
                case DataPacket::PLAIN_EVENTS:
                    eventCount = packet.getPlainEvents().size();
                    std::cout << "Type: PlainEvent, Events: " << eventCount << "\n";
                    break;
                default:
                    std::cout << "Type: Other\n";
                    break;
            }

            std::cout << "\n";
            messageCount++;
        }
    } catch (const std::exception& e) {
        off_t error_pos = lseek(fd, 0, SEEK_CUR);
        std::cerr << "Error at offset " << error_pos << ": " << e.what() << "\n";
    }

    close(fd);

    std::cout << "\nTotal messages successfully read: " << messageCount << "\n";

    return 0;
}
