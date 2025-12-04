#ifndef ROOTWRITER_H
#define ROOTWRITER_H

#include <string>
#include <memory>
#include "TFile.h"
#include "TTree.h"
#include "../TreeData.h"

class RootWriter {
public:
    explicit RootWriter(const std::string& filename);
    ~RootWriter() { Close(); }

    void Fill(const TreeData& data);
    void Close();

private:
    std::unique_ptr<TFile> file_;
    TTree* tree_;  // Owned by TFile, don't delete
    TreeData data_;
};

#endif
