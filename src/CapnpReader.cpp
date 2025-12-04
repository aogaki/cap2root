#include "CapnpReader.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>

bool CapnpReader::Open(const std::string& filename) {
    fd_ = open(filename.c_str(), O_RDONLY);
    return fd_ >= 0;
}

void CapnpReader::Close() {
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

bool CapnpReader::HasNext() const {
    return fd_ >= 0;
}

std::vector<TreeData> CapnpReader::ReadNextPacket() {
    std::vector<TreeData> results;

    if (fd_ < 0) {
        return results;
    }

    try {
        // Check if we can read more data
        char peek;
        ssize_t n = read(fd_, &peek, 1);
        if (n <= 0) {
            Close();
            return results;
        }
        // Put the byte back by seeking back
        lseek(fd_, -1, SEEK_CUR);

        capnp::PackedFdMessageReader message(fd_);
        auto packet = message.getRoot<DataPacket>();

        // Determine which event type is present
        switch (packet.which()) {
            case DataPacket::PLAIN_EVENTS: {
                auto events = packet.getPlainEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.ChargeLong = event.getEnergy();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeShort = 0;
                    data.FineTS = 0.0;
                    data.Extras = 0;
                    data.RecordLength = 0;
                    results.push_back(data);
                }
                break;
            }
            case DataPacket::PSD_EVENTS: {
                auto events = packet.getPsdEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.ChargeLong = event.getEnergy();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeShort = static_cast<uint16_t>(event.getPsd() * 1000); // Store PSD scaled
                    data.FineTS = event.getPsd();
                    data.Extras = 0;
                    data.RecordLength = 0;
                    results.push_back(data);
                }
                break;
            }
            case DataPacket::WAVE_EVENTS: {
                auto events = packet.getWaveEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.ChargeLong = event.getEnergy();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeShort = 0;
                    data.FineTS = 0.0;
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
            case DataPacket::DUAL_WAVE_EVENTS: {
                auto events = packet.getDualWaveEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.ChargeLong = event.getEnergy();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeShort = 0;
                    data.FineTS = 0.0;
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
            case DataPacket::RAW_TIME_EVENTS: {
                auto events = packet.getRawTimeEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.ChargeLong = event.getEnergy();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeShort = 0;
                    data.FineTS = event.getFineTimestamp();
                    data.Extras = 0;
                    data.RecordLength = 0;
                    results.push_back(data);
                }
                break;
            }
            case DataPacket::PSD_WAVE_EVENTS: {
                auto events = packet.getPsdWaveEvents();
                for (auto event : events) {
                    TreeData data;
                    data.Mod = event.getBoard();
                    data.Ch = event.getChannel();
                    data.ChargeLong = event.getEnergy();
                    data.TimeStamp = event.getTimestamp();
                    data.ChargeShort = static_cast<uint16_t>(event.getPsd() * 1000);
                    data.FineTS = event.getPsd();
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
            default:
                break;
        }
    } catch (const std::exception& e) {
        // EOF or read error - close and return what we have
        Close();
    }

    return results;
}

void CapnpReader::DumpPacket(int packetNum, bool verbose) {
    if (fd_ < 0) {
        std::cerr << "Error: File not open\n";
        return;
    }

    try {
        // Check if we can read more data
        char peek;
        ssize_t n = read(fd_, &peek, 1);
        if (n <= 0) {
            std::cout << "End of file\n";
            Close();
            return;
        }
        lseek(fd_, -1, SEEK_CUR);

        capnp::PackedFdMessageReader message(fd_);
        auto packet = message.getRoot<DataPacket>();

        std::cout << "\n=== Packet " << packetNum << " ===\n";

        // Determine event type and dump accordingly
        switch (packet.which()) {
            case DataPacket::PLAIN_EVENTS: {
                auto events = packet.getPlainEvents();
                std::cout << "Type: PlainEvent\n";
                std::cout << "Events: " << events.size() << "\n";

                if (verbose) {
                    std::cout << "\n" << std::setw(6) << "Index"
                              << std::setw(6) << "Mod"
                              << std::setw(6) << "Ch"
                              << std::setw(10) << "Energy"
                              << std::setw(16) << "Timestamp\n";
                    std::cout << std::string(44, '-') << "\n";

                    int idx = 0;
                    for (auto event : events) {
                        std::cout << std::setw(6) << idx++
                                  << std::setw(6) << (int)event.getBoard()
                                  << std::setw(6) << (int)event.getChannel()
                                  << std::setw(10) << event.getEnergy()
                                  << std::setw(16) << event.getTimestamp() << "\n";
                    }
                }
                break;
            }
            case DataPacket::PSD_EVENTS: {
                auto events = packet.getPsdEvents();
                std::cout << "Type: PsdEvent\n";
                std::cout << "Events: " << events.size() << "\n";

                if (verbose) {
                    std::cout << "\n" << std::setw(6) << "Index"
                              << std::setw(6) << "Mod"
                              << std::setw(6) << "Ch"
                              << std::setw(10) << "Energy"
                              << std::setw(16) << "Timestamp"
                              << std::setw(10) << "PSD\n";
                    std::cout << std::string(54, '-') << "\n";

                    int idx = 0;
                    for (auto event : events) {
                        std::cout << std::setw(6) << idx++
                                  << std::setw(6) << (int)event.getBoard()
                                  << std::setw(6) << (int)event.getChannel()
                                  << std::setw(10) << event.getEnergy()
                                  << std::setw(16) << event.getTimestamp()
                                  << std::setw(10) << std::fixed << std::setprecision(3)
                                  << event.getPsd() << "\n";
                    }
                }
                break;
            }
            case DataPacket::WAVE_EVENTS: {
                auto events = packet.getWaveEvents();
                std::cout << "Type: WaveEvent\n";
                std::cout << "Events: " << events.size() << "\n";

                if (verbose) {
                    std::cout << "\n" << std::setw(6) << "Index"
                              << std::setw(6) << "Mod"
                              << std::setw(6) << "Ch"
                              << std::setw(10) << "Energy"
                              << std::setw(16) << "Timestamp"
                              << std::setw(12) << "WaveSize\n";
                    std::cout << std::string(56, '-') << "\n";

                    int idx = 0;
                    for (auto event : events) {
                        std::cout << std::setw(6) << idx++
                                  << std::setw(6) << (int)event.getBoard()
                                  << std::setw(6) << (int)event.getChannel()
                                  << std::setw(10) << event.getEnergy()
                                  << std::setw(16) << event.getTimestamp()
                                  << std::setw(12) << event.getWaveform1().size() << "\n";
                    }
                }
                break;
            }
            case DataPacket::DUAL_WAVE_EVENTS: {
                auto events = packet.getDualWaveEvents();
                std::cout << "Type: DualWaveEvent\n";
                std::cout << "Events: " << events.size() << "\n";

                if (verbose) {
                    std::cout << "\n" << std::setw(6) << "Index"
                              << std::setw(6) << "Mod"
                              << std::setw(6) << "Ch"
                              << std::setw(10) << "Energy"
                              << std::setw(16) << "Timestamp"
                              << std::setw(10) << "Wave1"
                              << std::setw(10) << "Wave2\n";
                    std::cout << std::string(64, '-') << "\n";

                    int idx = 0;
                    for (auto event : events) {
                        std::cout << std::setw(6) << idx++
                                  << std::setw(6) << (int)event.getBoard()
                                  << std::setw(6) << (int)event.getChannel()
                                  << std::setw(10) << event.getEnergy()
                                  << std::setw(16) << event.getTimestamp()
                                  << std::setw(10) << event.getWaveform1().size()
                                  << std::setw(10) << event.getWaveform2().size() << "\n";
                    }
                }
                break;
            }
            case DataPacket::RAW_TIME_EVENTS: {
                auto events = packet.getRawTimeEvents();
                std::cout << "Type: RawTimeEvent\n";
                std::cout << "Events: " << events.size() << "\n";

                if (verbose) {
                    std::cout << "\n" << std::setw(6) << "Index"
                              << std::setw(6) << "Mod"
                              << std::setw(6) << "Ch"
                              << std::setw(10) << "Energy"
                              << std::setw(16) << "Timestamp"
                              << std::setw(10) << "FineTS\n";
                    std::cout << std::string(54, '-') << "\n";

                    int idx = 0;
                    for (auto event : events) {
                        std::cout << std::setw(6) << idx++
                                  << std::setw(6) << (int)event.getBoard()
                                  << std::setw(6) << (int)event.getChannel()
                                  << std::setw(10) << event.getEnergy()
                                  << std::setw(16) << event.getTimestamp()
                                  << std::setw(10) << event.getFineTimestamp() << "\n";
                    }
                }
                break;
            }
            case DataPacket::CROSS_EVENTS: {
                auto events = packet.getCrossEvents();
                std::cout << "Type: CrossEvent\n";
                std::cout << "Events: " << events.size() << "\n";

                if (verbose) {
                    std::cout << "\n" << std::setw(6) << "Index"
                              << std::setw(6) << "Mod"
                              << std::setw(6) << "Ch"
                              << std::setw(10) << "Energy"
                              << std::setw(16) << "Timestamp"
                              << std::setw(8) << "GoodTrg"
                              << std::setw(8) << "LostTrg\n";
                    std::cout << std::string(60, '-') << "\n";

                    int idx = 0;
                    for (auto event : events) {
                        std::cout << std::setw(6) << idx++
                                  << std::setw(6) << (int)event.getBoard()
                                  << std::setw(6) << (int)event.getChannel()
                                  << std::setw(10) << event.getEnergy()
                                  << std::setw(16) << event.getTimestamp()
                                  << std::setw(8) << (event.getGoodTrigger() ? "Yes" : "No")
                                  << std::setw(8) << (event.getLostTrigger() ? "Yes" : "No") << "\n";
                    }
                }
                break;
            }
            case DataPacket::PSD_WAVE_EVENTS: {
                auto events = packet.getPsdWaveEvents();
                std::cout << "Type: PsdWaveEvent\n";
                std::cout << "Events: " << events.size() << "\n";

                if (verbose) {
                    std::cout << "\n" << std::setw(6) << "Index"
                              << std::setw(6) << "Mod"
                              << std::setw(6) << "Ch"
                              << std::setw(10) << "Energy"
                              << std::setw(16) << "Timestamp"
                              << std::setw(10) << "PSD"
                              << std::setw(12) << "WaveSize\n";
                    std::cout << std::string(66, '-') << "\n";

                    int idx = 0;
                    for (auto event : events) {
                        std::cout << std::setw(6) << idx++
                                  << std::setw(6) << (int)event.getBoard()
                                  << std::setw(6) << (int)event.getChannel()
                                  << std::setw(10) << event.getEnergy()
                                  << std::setw(16) << event.getTimestamp()
                                  << std::setw(10) << std::fixed << std::setprecision(3)
                                  << event.getPsd()
                                  << std::setw(12) << event.getWaveform1().size() << "\n";
                    }
                }
                break;
            }
            case DataPacket::FULL_EVENTS: {
                auto events = packet.getFullEvents();
                std::cout << "Type: FullEvent\n";
                std::cout << "Events: " << events.size() << "\n";

                if (verbose) {
                    std::cout << "\n" << std::setw(6) << "Index"
                              << std::setw(6) << "Mod"
                              << std::setw(6) << "Ch"
                              << std::setw(10) << "Energy"
                              << std::setw(16) << "Timestamp"
                              << std::setw(10) << "PSD"
                              << std::setw(10) << "Wave1"
                              << std::setw(10) << "Wave2\n";
                    std::cout << std::string(74, '-') << "\n";

                    int idx = 0;
                    for (auto event : events) {
                        std::cout << std::setw(6) << idx++
                                  << std::setw(6) << (int)event.getBoard()
                                  << std::setw(6) << (int)event.getChannel()
                                  << std::setw(10) << event.getEnergy()
                                  << std::setw(16) << event.getTimestamp()
                                  << std::setw(10) << std::fixed << std::setprecision(3)
                                  << event.getPsd()
                                  << std::setw(10) << event.getWaveform1().size()
                                  << std::setw(10) << event.getWaveform2().size() << "\n";
                    }
                }
                break;
            }
            default:
                std::cout << "Type: Unknown\n";
                break;
        }

    } catch (const std::exception& e) {
        std::cout << "End of file or error\n";
        Close();
    }
}
