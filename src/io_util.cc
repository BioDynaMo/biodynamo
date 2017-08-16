#include "io_util.h"
#include <fstream>
#include <iostream>

namespace bdm {

RuntimeVariables::RuntimeVariables() { gSystem->GetSysInfo(&sysinfo_); }

RuntimeVariables::RuntimeVariables(TRootIOCtor* io_ctor) {}

SysInfo_t RuntimeVariables::GetSystemInfo() const { return sysinfo_; }

void RuntimeVariables::SetSystemInfo(const SysInfo_t& other) {
  sysinfo_ = other;
}

void RuntimeVariables::PrintSystemInfo() {
  // clang-format off
  std::cout << "OS:\t"    << sysinfo_.fOS       << std::endl
            << "Model:\t" << sysinfo_.fModel    << std::endl
            << "Arch:\t"  << sysinfo_.fCpuType  << std::endl
            << "#CPUs:\t" << sysinfo_.fCpus     << std::endl
            << "RAM:\t"   << sysinfo_.fPhysRam  << "MB"  << std::endl;
  // clang-format on
}

bool RuntimeVariables::operator==(const RuntimeVariables& other) const {
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

bool RuntimeVariables::operator!=(const RuntimeVariables& other) const {
  return !(*this == other);
}

TFileRaii::TFileRaii(const std::string& filename, const char* mode)
    : file_(new TFile(filename.c_str(), mode)) {}

TFileRaii::TFileRaii(TFile* file) : file_(file) {}

TFileRaii::~TFileRaii() {
  file_->Close();
  delete file_;
}

TFile* TFileRaii::Get() { return file_; }

bool FileExists(const std::string& file_name) {
  std::ifstream infile(file_name);
  return infile.good();
}

}  // namespace bdm
