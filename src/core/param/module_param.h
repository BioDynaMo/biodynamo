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

#ifndef CORE_PARAM_MODULE_PARAM_H_
#define CORE_PARAM_MODULE_PARAM_H_

#include <memory>
#include "core/util/root.h"
#include "cpptoml/cpptoml.h"

namespace bdm {

struct Param;

using ModuleParamUid = uint64_t;

// TODO(lukas) code duplication with `UniqueEventIdFactory`
/// This class generates unique ids for module parameters. Thread safe.
class ModuleParamUidGenerator {
 public:
  ModuleParamUidGenerator(const ModuleParamUidGenerator&) = delete;

  static ModuleParamUidGenerator* Get();

  ModuleParamUid NewUid();

 private:
  ModuleParamUidGenerator();
  std::atomic<ModuleParamUid> counter_;
};

/// Interface for module parameters.
struct ModuleParam {
  virtual ~ModuleParam();

  virtual ModuleParam* GetCopy() const = 0;

  virtual ModuleParamUid GetUid() const = 0;

 protected:
  /// Assign values from a toml config file.\n
  /// Can be ommited if toml file support is not required.
  virtual void AssignFromConfig(const std::shared_ptr<cpptoml::table>&);

 private:
  friend struct Param;
  BDM_CLASS_DEF(ModuleParam, 1);
};

#define BDM_MODULE_PARAM_HEADER(name, version_id)                   \
  static const ModuleParamUid kUid;                                 \
  name() {}                                                         \
  virtual ~name() {}                                                \
  ModuleParam* GetCopy() const override { return new name(*this); } \
  ModuleParamUid GetUid() const override { return kUid; }           \
                                                                    \
 private:                                                           \
  BDM_CLASS_DEF_OVERRIDE(name, version_id);                         \
                                                                    \
 public:

}  // namespace bdm

#endif  // CORE_PARAM_MODULE_PARAM_H_
