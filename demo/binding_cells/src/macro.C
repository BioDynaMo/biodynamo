{
  TFile *f = TFile::Open("results.root");
  TTree *t = nullptr;
  f->GetObject("binding_cells", t);
  t->Print();
  t->Show(0);

  TTreeReader reader("binding_cells", f);
  TTreeReaderValue<vector<int>> veca(reader, "activity");
}
