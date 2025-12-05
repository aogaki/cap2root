#ifndef CAPNPREADER_H
#define CAPNPREADER_H

#include <string>
#include <vector>
#include <memory>
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <capnp/serialize-packed.h>
#include <kj/io.h>
#include "eventProto.capnp.h"
#include "../TreeData.h"

class CapnpReader {
public:
    CapnpReader() = default;
    ~CapnpReader() { Close(); }

    bool Open(const std::string& filename);
    void Close();
    bool HasNext() const;
    std::vector<TreeData> ReadNextPacket();
    void DumpPacket(int packetNum, bool verbose = false);

private:
    int fd_ = -1;
    std::unique_ptr<kj::FdInputStream> fdStream_;
    std::unique_ptr<kj::BufferedInputStreamWrapper> bufferedStream_;
};

#endif
