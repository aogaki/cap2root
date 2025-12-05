#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#ifdef __linux__
#include <parallel/algorithm>
#endif
#include "CapnpReader.h"
#include "RootWriter.h"

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " <input.cap> <output.root>\n";
    std::cout << "Convert Cap'n Proto files to ROOT format\n";
    std::cout << "Events are sorted by timestamp before writing.\n";
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

    // Read all events into memory
    std::cout << "Reading events from Cap'n Proto file...\n";
    std::vector<TreeData> allEvents;
    int packetCount = 0;

    while (reader.HasNext()) {
        auto events = reader.ReadNextPacket();
        if (events.empty()) {
            break;
        }

        allEvents.insert(allEvents.end(), events.begin(), events.end());
        packetCount++;

        if (packetCount % 100 == 0) {
            std::cout << "Read " << packetCount << " packets, "
                      << allEvents.size() << " events\r" << std::flush;
        }
    }

    reader.Close();

    std::cout << "\nRead complete. Total events: " << allEvents.size() << "\n";
    std::cout << "Sorting events by timestamp...\n";

    // Sort by timestamp
#ifdef __linux__
    __gnu_parallel::sort(allEvents.begin(), allEvents.end(),
                         [](const TreeData& a, const TreeData& b) {
                             return a.TimeStamp < b.TimeStamp;
                         });
    std::cout << "Sorting complete (GNU parallel sort used).\n";
#else
    std::sort(allEvents.begin(), allEvents.end(),
              [](const TreeData& a, const TreeData& b) {
                  return a.TimeStamp < b.TimeStamp;
              });
    std::cout << "Sorting complete.\n";
#endif
    std::cout << "Writing to ROOT file...\n";

    // Write sorted events to ROOT file
    RootWriter writer(outputFile);

    for (size_t i = 0; i < allEvents.size(); i++) {
        writer.Fill(allEvents[i]);

        if ((i + 1) % 100000 == 0) {
            std::cout << "Written " << (i + 1) << " / " << allEvents.size()
                      << " events\r" << std::flush;
        }
    }

    writer.Close();

    std::cout << "\nConversion complete!\n";
    std::cout << "Total packets read: " << packetCount << "\n";
    std::cout << "Total events written: " << allEvents.size() << "\n";

    return 0;
}
