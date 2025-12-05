#include "CapnpReader.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>

bool CapnpReader::Open(const std::string& filename) {
    fd_ = open(filename.c_str(), O_RDONLY);
    if (fd_ < 0) {
        return false;
    }

    fdStream_ = std::make_unique<kj::FdInputStream>(fd_);
    bufferedStream_ = std::make_unique<kj::BufferedInputStreamWrapper>(*fdStream_);

    return true;
}

void CapnpReader::Close() {
    if (fd_ >= 0) {
        bufferedStream_.reset();
        fdStream_.reset();
        close(fd_);
        fd_ = -1;
    }
}

bool CapnpReader::HasNext() const {
    if (fd_ < 0 || !bufferedStream_) {
        return false;
    }
    return bufferedStream_->tryGetReadBuffer() != nullptr;
}

std::vector<TreeData> CapnpReader::ReadNextPacket() {
    std::vector<TreeData> results;

    if (fd_ < 0 || !bufferedStream_) {
        return results;
    }

    try {
        if (bufferedStream_->tryGetReadBuffer() == nullptr) {
            Close();
            return results;
        }

        capnp::PackedMessageReader message(*bufferedStream_, {100000000, 64});

        // First read as PlainData to get the type
        auto plainData = message.getRoot<PlainData>();
        int evtType = plainData.getType();

        // Process based on type
        switch (evtType) {
            case 0: {  // PlainData
                auto events = plainData.getEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeLong = event.getEnergy();
                    data.ChargeShort = 0;
                    data.Extras = 0;
                    data.RecordLength = 0;
                    // FineTS should be TimeStamp * 1000
                    data.FineTS = static_cast<double>(data.TimeStamp) * 1000.0;
                    results.push_back(data);
                }
                break;
            }
            case 1: {  // PsdData
                auto psdData = message.getRoot<PsdData>();
                auto events = psdData.getEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.ChargeLong = event.getEnergy();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeShort = static_cast<uint16_t>(event.getPsd() * 1000);
                    // FineTS should be TimeStamp * 1000, not PSD value
                    data.FineTS = static_cast<double>(data.TimeStamp) * 1000.0;
                    data.Extras = 0;
                    data.RecordLength = 0;
                    results.push_back(data);
                }
                break;
            }
            case 2: {  // WaveData
                auto waveData = message.getRoot<WaveData>();
                auto events = waveData.getEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.ChargeLong = event.getEnergy();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeShort = 0;
                    data.FineTS = data.TimeStamp * 1000.0;
                    data.Extras = 0;

                    auto wave = event.getWaveform1();
                    data.RecordLength = wave.size();
                    data.Trace1.resize(data.RecordLength);
                    for (size_t i = 0; i < wave.size(); i++) {
                        data.Trace1[i] = wave[i];
                    }
                    results.push_back(data);
                }
                break;
            }
            case 3: {  // DualWaveData
                auto dwData = message.getRoot<DualWaveData>();
                auto events = dwData.getEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.ChargeLong = event.getEnergy();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeShort = 0;
                    data.FineTS = data.TimeStamp * 1000.0;
                    data.Extras = 0;

                    auto wave1 = event.getWaveform1();
                    auto wave2 = event.getWaveform2();
                    data.RecordLength = wave1.size();
                    data.Trace1.resize(data.RecordLength);
                    data.Trace2.resize(data.RecordLength);
                    for (size_t i = 0; i < wave1.size(); i++) {
                        data.Trace1[i] = wave1[i];
                    }
                    for (size_t i = 0; i < wave2.size(); i++) {
                        data.Trace2[i] = wave2[i];
                    }
                    results.push_back(data);
                }
                break;
            }
            case 4: {  // FullData
                auto fullData = message.getRoot<FullData>();
                auto events = fullData.getEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.ChargeLong = event.getEnergy();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeShort = static_cast<uint16_t>(event.getPsd() * 1000);
                    // FineTS should be TimeStamp * 1000, not PSD value
                    data.FineTS = static_cast<double>(data.TimeStamp) * 1000.0;
                    data.Extras = 0;

                    auto wave1 = event.getWaveform1();
                    auto wave2 = event.getWaveform2();
                    data.RecordLength = wave1.size();
                    data.Trace1.resize(data.RecordLength);
                    data.Trace2.resize(data.RecordLength);
                    for (size_t i = 0; i < wave1.size(); i++) {
                        data.Trace1[i] = wave1[i];
                    }
                    for (size_t i = 0; i < wave2.size(); i++) {
                        data.Trace2[i] = wave2[i];
                    }
                    results.push_back(data);
                }
                break;
            }
            case 5: {  // RawTimeData
                auto rtData = message.getRoot<RawTimeData>();
                auto events = rtData.getEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.ChargeLong = event.getEnergy();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeShort = 0;
                    data.FineTS = event.getFineTimestamp();
                    // If FineTS is empty (0), use TimeStamp * 1000
                    if (data.FineTS == 0.0) {
                        data.FineTS = data.TimeStamp * 1000.0;
                    }
                    data.Extras = 0;
                    data.RecordLength = 0;
                    results.push_back(data);
                }
                break;
            }
            default:
                std::cerr << "Warning: Unknown event type " << evtType << "\n";
                break;
        }
    } catch (const std::exception& e) {
        // EOF or error
        Close();
    }

    return results;
}

void CapnpReader::DumpPacket(int packetNum, bool verbose) {
    // Simplified dump - just show summary
    auto events = ReadNextPacket();
    if (events.empty()) {
        std::cout << "End of file\n";
        return;
    }

    std::cout << "\n=== Packet " << packetNum << " ===\n";
    std::cout << "Events: " << events.size() << "\n";

    if (verbose && !events.empty()) {
        std::cout << "\n" << std::setw(6) << "Index"
                  << std::setw(6) << "Mod"
                  << std::setw(6) << "Ch"
                  << std::setw(10) << "Energy"
                  << std::setw(16) << "Timestamp\n";
        std::cout << std::string(44, '-') << "\n";

        for (size_t i = 0; i < std::min(events.size(), size_t(10)); i++) {
            std::cout << std::setw(6) << i
                      << std::setw(6) << (int)events[i].Mod
                      << std::setw(6) << (int)events[i].Ch
                      << std::setw(10) << events[i].ChargeLong
                      << std::setw(16) << events[i].TimeStamp << "\n";
        }
        if (events.size() > 10) {
            std::cout << "... (" << (events.size() - 10) << " more events)\n";
        }
    }
}
