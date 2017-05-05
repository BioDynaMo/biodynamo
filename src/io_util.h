#ifndef IO_UTIL_H_
#define IO_UTIL_H_

#include <fstream>

#include <TFile.h>

namespace bdm {

bool FileExists(const char *file_name) {
  std::ifstream infile(file_name);
  return infile.good();
}

template <typename T>
bool GetPersistentObject(const char *root_file, const char *objName,
                         T &empty_obj) {
  if (FileExists(root_file)) {
    TFile *f = TFile::Open(root_file);
    f->GetObject(objName, empty_obj);
    // todo check for null
    f->Close();
    return true;
  }
  return false;
}

template <typename T>
void WritePersistentObject(const char *root_file, const char *objName,
                           T &pst_object, const char *mode) {
  TFile *f = new TFile(root_file, mode);
  f->WriteObject(&pst_object, objName);
  f->Close();
}

}  // namespace bdm

#endif  // IO_UTIL_H_
