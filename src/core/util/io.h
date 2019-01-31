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

#ifndef CORE_UTIL_IO_H_
#define CORE_UTIL_IO_H_

#include <string>

#include <TFile.h>
#include <TSystem.h>
#include "core/util/root.h"

namespace bdm {

/// This class stores the runtime variables of the system. This is useful for
/// check if the same system is used for continuing a simulation for example.
class RuntimeVariables {
 public:
  RuntimeVariables();
  // Constructor for ROOT I/O
  explicit RuntimeVariables(TRootIOCtor* io_ctor);

  SysInfo_t GetSystemInfo() const;
  void SetSystemInfo(const SysInfo_t& other);

  void PrintSystemInfo();

  bool operator==(const RuntimeVariables& other) const;

  bool operator!=(const RuntimeVariables& other) const;

 private:
  SysInfo_t sysinfo_;
  BDM_CLASS_DEF_NV(RuntimeVariables, 1);  // NOLINT
};

/// Automatically close a TFile object using RAII pattern
class TFileRaii {
 public:
  TFileRaii(const std::string& filename, const char* mode);
  explicit TFileRaii(TFile* file);
  ~TFileRaii();
  TFile* Get();

 private:
  TFile* file_;
};

/// ROOT cannot write a single integral type (int, double, ...). Therefore,
/// this wrapper is needed
template <typename T>
class IntegralTypeWrapper {
 public:
  explicit IntegralTypeWrapper(const T& data) : data_(data) {}
  explicit IntegralTypeWrapper(TRootIOCtor* io_ctor) {}
  const T& Get() const { return data_; }

 private:
  T data_;
  BDM_CLASS_DEF_NV(IntegralTypeWrapper, 1);
};

bool FileExists(const std::string& file_name);

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
bool GetPersistentObject(const char* root_file, const char* obj_name,
                         T*& empty_obj) {  // NOLINT
  if (FileExists(root_file)) {
    TFileRaii file(TFile::Open(root_file));
    file.Get()->GetObject(obj_name, empty_obj);
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
/// new (default) | A new root file `root_file` is created. If file already exists, an error message is printed and the function returns.
/// recreate      | If file does not exist, it is created (like in "new"). If file already exist, the existing file is deleted before creating the new file.
/// update        | New classes are added to the existing directory. Existing classes with the same name are replaced by the new definition. If the directory dirname doest not exist, same effect as "new".
///
// clang-format on
template <typename T>
void WritePersistentObject(const char* root_file, const char* obj_name,
                           const T& pst_object, const char* mode = "new") {
  TFileRaii file(root_file, mode);
  file.Get()->WriteObject(&pst_object, obj_name);
}

}  // namespace bdm

#endif  // CORE_UTIL_IO_H_
