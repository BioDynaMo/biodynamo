#ifndef IO_UTIL_H_
#define IO_UTIL_H_

#include <fstream>
#include <iostream>

#include <Rtypes.h>
#include <TFile.h>
#include <TSystem.h>

namespace bdm {

/// This class stores the runtime variables of the system. This is useful for
/// check if the same system is used for continuing a simulation for example.
class RuntimeVariables {
 public:
  RuntimeVariables() { gSystem->GetSysInfo(&sysinfo_); }  // NOLINT
  explicit RuntimeVariables(TRootIOCtor *io_ctor) {
  }  // Constructor for ROOT I/O
  virtual ~RuntimeVariables() {}

  SysInfo_t GetSystemInfo() { return sysinfo_; }
  void SetSystemInfo(SysInfo_t &other) {  // NOLINT
    sysinfo_ = other;
  }
  // clang-format off
  void PrintSystemInfo() {
    std::cout << "OS:\t"    << sysinfo_.fOS       << std::endl
              << "Model:\t" << sysinfo_.fModel    << std::endl
              << "Arch:\t"  << sysinfo_.fCpuType  << std::endl
              << "#CPUs:\t" << sysinfo_.fCpus     << std::endl
              << "RAM:\t"   << sysinfo_.fPhysRam  << "MB"  << std::endl;
  }
  // clang-format on

  bool operator==(RuntimeVariables &other) {  // NOLINT
    if (sysinfo_.fOS != other.GetSystemInfo().fOS) {
      return false;
    }
    if (sysinfo_.fModel != other.GetSystemInfo().fModel) {
      return false;
    }
    if (sysinfo_.fCpuType != other.GetSystemInfo().fCpuType) {
      return false;
    }
    if (sysinfo_.fCpus != other.GetSystemInfo().fCpus) {
      return false;
    }
    if (sysinfo_.fPhysRam != other.GetSystemInfo().fPhysRam) {
      return false;
    }
    return true;
  }

  bool operator!=(RuntimeVariables &other) {  // NOLINT
    return !(*this == other);
  }

 private:
  SysInfo_t sysinfo_;
  ClassDef(RuntimeVariables, 1);  // NOLINT
};

inline bool FileExists(const char *file_name) {
  std::ifstream infile(file_name);
  return infile.good();
}

///
/// @brief      Gets the persistent object from the specified ROOT file.
///
/// @param[in]  root_file  The root file
/// @param[in]  obj_name   The object name
/// @param      empty_obj  The empty object
///
/// @tparam     T          { The object class type }
///
/// @return     The persistent object.
///
template <typename T>
bool GetPersistentObject(const char *root_file, const char *obj_name,
                         T &empty_obj) {  // NOLINT
  if (FileExists(root_file)) {
    TFile *f = TFile::Open(root_file);
    f->GetObject(obj_name, empty_obj);
    f->Close();
    return true;
  }
  return false;
}

// clang-format off
///
/// @brief      Writes a persistent object to the specified ROOT file.
///
/// @param[in]  root_file   The root file
/// @param[in]  obj_name    The object name
/// @param      pst_object  The persistent object
/// @param[in]  mode        The mode
///
/// @tparam     T           { The object class type }
///
/// Option | Details
/// -------|--------
/// new (default) | A new root file `root_file` is created. If file already exist, an error message is printed and the function returns.
/// recreate      | If file does not exist, it is created (like in "new"). If file already exist, the existing file is deleted before creating the new file.
/// update        | New classes are added to the existing directory. Existing classes with the same name are replaced by the new definition. If the directory dirname doest not exist, same effect as "new".
///
// clang-format on
template <typename T>
void WritePersistentObject(const char *root_file, const char *obj_name,
                           T &pst_object, const char *mode = "new") {  // NOLINT
  TFile *f = new TFile(root_file, mode);
  f->WriteObject(&pst_object, obj_name);
  f->Close();
}

}  // namespace bdm

#endif  // IO_UTIL_H_
