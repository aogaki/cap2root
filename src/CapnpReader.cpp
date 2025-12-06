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

std::vector<std::unique_ptr<TreeData>> CapnpReader::ReadNextPacket() {
    std::vector<std::unique_ptr<TreeData>> results;

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
                results.reserve(events.size());
                for (auto event : events) {
                    auto data = std::make_unique<TreeData>();
                    data->Mod = event.getBoard();
                    data->Ch = event.getChannel();
                    data->TimeStamp = event.getTimestamp();
                    data->ChargeLong = event.getEnergy();
                    data->ChargeShort = 0;
                    data->Extras = 0;
                    data->RecordLength = 0;
                    data->FineTS = static_cast<double>(data->TimeStamp);
                    results.push_back(std::move(data));
                }
                break;
            }
            case 1: {  // PsdData
                auto psdData = message.getRoot<PsdData>();
                auto events = psdData.getEvents();
                results.reserve(events.size());
                for (auto event : events) {
                    auto data = std::make_unique<TreeData>();
                    data->Mod = event.getBoard();
                    data->Ch = event.getChannel();
                    data->ChargeLong = event.getEnergy();
                    data->TimeStamp = event.getTimestamp();
                    data->ChargeShort = static_cast<uint16_t>(event.getPsd() * 1000);
                    data->FineTS = static_cast<double>(data->TimeStamp);
                    data->Extras = 0;
                    data->RecordLength = 0;
                    results.push_back(std::move(data));
                }
                break;
            }
            case 2: {  // WaveData
                auto waveData = message.getRoot<WaveData>();
                auto events = waveData.getEvents();
                results.reserve(events.size());
                for (auto event : events) {
                    auto data = std::make_unique<TreeData>();
                    data->Mod = event.getBoard();
                    data->Ch = event.getChannel();
                    data->ChargeLong = event.getEnergy();
                    data->TimeStamp = event.getTimestamp();
                    data->ChargeShort = 0;
                    data->FineTS = static_cast<double>(data->TimeStamp);
                    data->Extras = 0;

                    auto wave = event.getWaveform1();
                    data->RecordLength = wave.size();
                    data->Trace1.reserve(data->RecordLength);
                    for (auto val : wave) {
                        data->Trace1.push_back(val);
                    }
                    results.push_back(std::move(data));
                }
                break;
            }
            case 3: {  // DualWaveData
                auto dwData = message.getRoot<DualWaveData>();
                auto events = dwData.getEvents();
                results.reserve(events.size());
                for (auto event : events) {
                    auto data = std::make_unique<TreeData>();
                    data->Mod = event.getBoard();
                    data->Ch = event.getChannel();
                    data->ChargeLong = event.getEnergy();
                    data->TimeStamp = event.getTimestamp();
                    data->ChargeShort = 0;
                    data->FineTS = static_cast<double>(data->TimeStamp);
                    data->Extras = 0;

                    auto wave1 = event.getWaveform1();
                    auto wave2 = event.getWaveform2();
                    data->RecordLength = wave1.size();
                    data->Trace1.reserve(data->RecordLength);
                    data->Trace2.reserve(data->RecordLength);
                    for (auto val : wave1) {
                        data->Trace1.push_back(val);
                    }
                    for (auto val : wave2) {
                        data->Trace2.push_back(val);
                    }
                    results.push_back(std::move(data));
                }
                break;
            }
            case 4: {  // FullData
                auto fullData = message.getRoot<FullData>();
                auto events = fullData.getEvents();
                results.reserve(events.size());
                for (auto event : events) {
                    auto data = std::make_unique<TreeData>();
                    data->Mod = event.getBoard();
                    data->Ch = event.getChannel();
                    data->ChargeLong = event.getEnergy();
                    data->TimeStamp = event.getTimestamp();
                    data->ChargeShort = static_cast<uint16_t>(event.getPsd() * 1000);
                    data->FineTS = static_cast<double>(data->TimeStamp);
                    data->Extras = 0;

                    auto wave1 = event.getWaveform1();
                    auto wave2 = event.getWaveform2();
                    data->RecordLength = wave1.size();
                    data->Trace1.reserve(data->RecordLength);
                    data->Trace2.reserve(data->RecordLength);
                    for (auto val : wave1) {
                        data->Trace1.push_back(val);
                    }
                    for (auto val : wave2) {
                        data->Trace2.push_back(val);
                    }
                    results.push_back(std::move(data));
                }
                break;
            }
            case 5: {  // RawTimeData
                auto rtData = message.getRoot<RawTimeData>();
                auto events = rtData.getEvents();
                results.reserve(events.size());
                for (auto event : events) {
                    auto data = std::make_unique<TreeData>();
                    data->Mod = event.getBoard();
                    data->Ch = event.getChannel();
                    data->ChargeLong = event.getEnergy();
                    data->TimeStamp = event.getTimestamp();
                    data->ChargeShort = 0;
                    data->FineTS = event.getFineTimestamp();
                    // If FineTS is empty (0), use TimeStamp as double
                    if (data->FineTS == 0.0) {
                        data->FineTS = static_cast<double>(data->TimeStamp);
                    }
                    data->Extras = 0;
                    data->RecordLength = 0;
                    results.push_back(std::move(data));
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

size_t CapnpReader::CountTotalEvents() {
    if (fd_ < 0 || !bufferedStream_) {
        return 0;
    }

    size_t totalEvents = 0;

    while (bufferedStream_->tryGetReadBuffer() != nullptr) {
        try {
            capnp::PackedMessageReader message(*bufferedStream_, {100000000, 64});

            // Read as PlainData to get the type
            auto plainData = message.getRoot<PlainData>();
            int evtType = plainData.getType();

            // Count events based on type
            switch (evtType) {
                case 0: {  // PlainData
                    totalEvents += plainData.getEvents().size();
                    break;
                }
                case 1: {  // PsdData
                    auto psdData = message.getRoot<PsdData>();
                    totalEvents += psdData.getEvents().size();
                    break;
                }
                case 2: {  // WaveData
                    auto waveData = message.getRoot<WaveData>();
                    totalEvents += waveData.getEvents().size();
                    break;
                }
                case 3: {  // DualWaveData
                    auto dwData = message.getRoot<DualWaveData>();
                    totalEvents += dwData.getEvents().size();
                    break;
                }
                case 4: {  // FullData
                    auto fullData = message.getRoot<FullData>();
                    totalEvents += fullData.getEvents().size();
                    break;
                }
                case 5: {  // RawTimeData
                    auto rtData = message.getRoot<RawTimeData>();
                    totalEvents += rtData.getEvents().size();
                    break;
                }
                default:
                    break;
            }
        } catch (const std::exception& e) {
            // EOF or error
            break;
        }
    }

    // Close and reopen to reset stream position
    Close();

    return totalEvents;
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
                      << std::setw(6) << (int)events[i]->Mod
                      << std::setw(6) << (int)events[i]->Ch
                      << std::setw(10) << events[i]->ChargeLong
                      << std::setw(16) << events[i]->TimeStamp << "\n";
        }
        if (events.size() > 10) {
            std::cout << "... (" << (events.size() - 10) << " more events)\n";
        }
    }
}
