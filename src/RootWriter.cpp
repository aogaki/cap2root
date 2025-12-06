#include "RootWriter.h"

RootWriter::RootWriter(const std::string &filename)
{
  file_ = std::make_unique<TFile>(filename.c_str(), "RECREATE");

  // Set compression level (0=none, 1=fastest, 9=best compression)
  // Using level 1 for fast compression
  file_->SetCompressionLevel(1);

  tree_ = new TTree("ELIADE_Tree", "Converted data from ROSPHER");

  // Optimize TTree performance
  tree_->SetAutoSave(0);           // Disable AutoSave (save only at Close)
  tree_->SetAutoFlush(-30000000);  // Flush every 30MB instead of default

  // Setup branches with larger basket sizes (default is 32KB, using 2MB)
  const int basketSize = 2000000;

  tree_->Branch("Mod", &data_.Mod, "Mod/b", basketSize);
  tree_->Branch("Ch", &data_.Ch, "Ch/b", basketSize);
  tree_->Branch("TimeStamp", &data_.TimeStamp, "TimeStamp/l", basketSize);
  tree_->Branch("FineTS", &data_.FineTS, "FineTS/D", basketSize);
  tree_->Branch("ChargeLong", &data_.ChargeLong, "ChargeLong/s", basketSize);
  tree_->Branch("ChargeShort", &data_.ChargeShort, "ChargeShort/s", basketSize);
  tree_->Branch("RecordLength", &data_.RecordLength, "RecordLength/i",
                basketSize);
  tree_->Branch("Signal", data_.Trace1.data(), "Signal[RecordLength]/s",
                basketSize);
}

void RootWriter::Fill(const TreeData &data)
{
  data_ = data;
  tree_->Fill();
}

void RootWriter::Close()
{
  if (file_ && file_->IsOpen()) {
    tree_->Write();
    file_->Close();
  }
}
