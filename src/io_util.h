#ifndef IO_UTIL_H_
#define IO_UTIL_H_

#include <fstream>

#include <TFile.h>
#include <Rtypes.h>
#include <TSystem.h>

namespace bdm {

class RuntimeVariables {
 public:
  RuntimeVariables() { gSystem->GetSysInfo(&sysinfo_); }
  RuntimeVariables(TRootIOCtor*) {}  // Constructor for ROOT I/O
  virtual ~RuntimeVariables() {}

  SysInfo_t GetSystemInfo() { return sysinfo_; }
  void PrintSystemInfo() {
    std::cout << "OS:\t"    << sysinfo_.fOS       << std::endl
              << "Model:\t" << sysinfo_.fModel    << std::endl
              << "Arch:\t"  << sysinfo_.fCpuType  << std::endl
              << "#CPUs:\t" << sysinfo_.fCpus     << std::endl
              << "RAM:\t"   << sysinfo_.fPhysRam << "MB"  << std::endl;
  }

  bool operator==(RuntimeVariables& other) {
    if (sysinfo_.fOS != other.GetSystemInfo().fOS)
      return false;
    if (sysinfo_.fModel != other.GetSystemInfo().fModel)
      return false;
    if (sysinfo_.fCpuType != other.GetSystemInfo().fCpuType)
      return false;
    if (sysinfo_.fCpus != other.GetSystemInfo().fCpus)
      return false;
    if (sysinfo_.fPhysRam != other.GetSystemInfo().fPhysRam)
      return false;
    return true;
  }

 private:
  SysInfo_t sysinfo_;
  ClassDef(RuntimeVariables, 1);
};

inline bool FileExists(const char *file_name) {
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
