#include "RootWriter.h"

RootWriter::RootWriter(const std::string& filename) {
    file_ = std::make_unique<TFile>(filename.c_str(), "RECREATE");
    tree_ = new TTree("tree", "Converted data from Cap'n Proto");

    // Setup branches
    tree_->Branch("Mod", &data_.Mod, "Mod/b");
    tree_->Branch("Ch", &data_.Ch, "Ch/b");
    tree_->Branch("TimeStamp", &data_.TimeStamp, "TimeStamp/l");
    tree_->Branch("FineTS", &data_.FineTS, "FineTS/D");
    tree_->Branch("ChargeLong", &data_.ChargeLong, "ChargeLong/s");
    tree_->Branch("ChargeShort", &data_.ChargeShort, "ChargeShort/s");
    tree_->Branch("Extras", &data_.Extras, "Extras/i");
    tree_->Branch("RecordLength", &data_.RecordLength, "RecordLength/i");
    tree_->Branch("Trace1", &data_.Trace1);
    tree_->Branch("Trace2", &data_.Trace2);
    tree_->Branch("DTrace1", &data_.DTrace1);
    tree_->Branch("DTrace2", &data_.DTrace2);
}

void RootWriter::Fill(const TreeData& data) {
    data_ = data;
    tree_->Fill();
}

void RootWriter::Close() {
    if (file_ && file_->IsOpen()) {
        tree_->Write();
        file_->Close();
    }
}
