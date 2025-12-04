#include <iostream>
#include <string>
#include "CapnpReader.h"

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " <input.cap> [options]\n";
    std::cout << "Dump Cap'n Proto file information\n\n";
    std::cout << "Options:\n";
    std::cout << "  -v, --verbose    Show detailed information for all events\n";
    std::cout << "  -n NUM           Show only first NUM packets (default: all)\n";
    std::cout << "  -h, --help       Show this help message\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string inputFile;
    bool verbose = false;
    int maxPackets = -1;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-n" && i + 1 < argc) {
            maxPackets = std::atoi(argv[++i]);
        } else if (inputFile.empty()) {
            inputFile = arg;
        }
    }

    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified\n";
        printUsage(argv[0]);
        return 1;
    }

    std::cout << "Dumping Cap'n Proto file: " << inputFile << "\n";
    if (verbose) {
        std::cout << "Mode: Verbose (showing all events)\n";
    }
    if (maxPackets > 0) {
        std::cout << "Showing first " << maxPackets << " packets\n";
    }
    std::cout << std::string(60, '=') << "\n";

    CapnpReader reader;
    if (!reader.Open(inputFile)) {
        std::cerr << "Error: Cannot open input file " << inputFile << "\n";
        return 1;
    }

    int packetCount = 0;
    int totalEvents = 0;

    while (reader.HasNext()) {
        if (maxPackets > 0 && packetCount >= maxPackets) {
            break;
        }

        // Read packet to count events
        auto events = reader.ReadNextPacket();
        if (events.empty()) {
            break;
        }

        totalEvents += events.size();
        packetCount++;
    }

    // Reopen file to dump
    reader.Close();
    if (!reader.Open(inputFile)) {
        std::cerr << "Error: Cannot reopen file\n";
        return 1;
    }

    packetCount = 0;
    while (reader.HasNext()) {
        if (maxPackets > 0 && packetCount >= maxPackets) {
            break;
        }

        reader.DumpPacket(packetCount, verbose);
        packetCount++;
    }

    reader.Close();

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Summary:\n";
    std::cout << "Total packets: " << packetCount << "\n";
    std::cout << "Total events: " << totalEvents << "\n";

    return 0;
}
