// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include <fstream>
#include <iostream>

#include "core/util/io.h"
#include "core/util/log.h"

namespace bdm {

RuntimeVariables::RuntimeVariables() { gSystem->GetSysInfo(&sysinfo_); }

RuntimeVariables::RuntimeVariables(TRootIOCtor* io_ctor) {}

SysInfo_t RuntimeVariables::GetSystemInfo() const { return sysinfo_; }

void RuntimeVariables::SetSystemInfo(const SysInfo_t& other) {
  sysinfo_ = other;
}

void RuntimeVariables::PrintSystemInfo() {
  // clang-format off
  Log::Info("RuntimeVariables", "OS:\t", sysinfo_.fOS, "\n",
            "Model:\t", sysinfo_.fModel,    "\n",
            "Arch:\t",  sysinfo_.fCpuType,  "\n",
            "#CPUs:\t", sysinfo_.fCpus,     "\n",
            "RAM:\t",   sysinfo_.fPhysRam,  "MB", "\n");
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
