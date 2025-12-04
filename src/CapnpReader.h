#ifndef CAPNPREADER_H
#define CAPNPREADER_H

#include <string>
#include <vector>
#include <fstream>
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <capnp/serialize-packed.h>
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
};

#endif
