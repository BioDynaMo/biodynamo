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
  /// Assign values from config file to variables
  virtual void AssignFromConfig(const std::shared_ptr<cpptoml::table>&) = 0;

 private:
  friend struct Param;
  BDM_CLASS_DEF(ModuleParam, 1);
};

}  // namespace bdm

#endif  // CORE_PARAM_MODULE_PARAM_H_
