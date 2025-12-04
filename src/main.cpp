#include <iostream>
#include <string>
#include "CapnpReader.h"
#include "RootWriter.h"

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " <input.cap> <output.root>\n";
    std::cout << "Convert Cap'n Proto files to ROOT format\n";
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    std::cout << "Converting " << inputFile << " to " << outputFile << "...\n";

    CapnpReader reader;
    if (!reader.Open(inputFile)) {
        std::cerr << "Error: Cannot open input file " << inputFile << "\n";
        return 1;
    }

    RootWriter writer(outputFile);

    int packetCount = 0;
    int eventCount = 0;

    while (reader.HasNext()) {
        auto events = reader.ReadNextPacket();
        if (events.empty()) {
            break; // End of file
        }

        for (const auto& event : events) {
            writer.Fill(event);
            eventCount++;
        }
        packetCount++;

        if (packetCount % 100 == 0) {
            std::cout << "Processed " << packetCount << " packets, "
                      << eventCount << " events\r" << std::flush;
        }
    }

    reader.Close();
    writer.Close();

    std::cout << "\nConversion complete!\n";
    std::cout << "Total packets: " << packetCount << "\n";
    std::cout << "Total events: " << eventCount << "\n";

    return 0;
}
