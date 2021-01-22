// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_PARAM_PARAM_GROUP_H_
#define CORE_PARAM_PARAM_GROUP_H_

#include <memory>
#include "core/util/root.h"
#include "cpptoml/cpptoml.h"

namespace bdm {

struct Param;

using ParamGroupUid = uint64_t;

// TODO(lukas) code duplication with `NewAgentEventUidGenerator`
/// This class generates unique ids for parameter groups. Thread safe.
class ParamGroupUidGenerator {
 public:
  ParamGroupUidGenerator(const ParamGroupUidGenerator&) = delete;

  static ParamGroupUidGenerator* Get();

  ParamGroupUid NewUid();

 private:
  ParamGroupUidGenerator();
  std::atomic<ParamGroupUid> counter_;
};

/// Interface for parameter groups.
struct ParamGroup {
  virtual ~ParamGroup();

  virtual ParamGroup* NewCopy() const = 0;

  virtual ParamGroupUid GetUid() const = 0;

 protected:
  /// Assign values from a toml config file.\n
  /// Can be ommited if toml file support is not required.
  virtual void AssignFromConfig(const std::shared_ptr<cpptoml::table>&);

 private:
  friend struct Param;
  BDM_CLASS_DEF(ParamGroup, 1);
};

#define BDM_PARAM_GROUP_HEADER(name, version_id)                   \
  static const ParamGroupUid kUid;                                 \
  name() {}                                                        \
  virtual ~name() {}                                               \
  ParamGroup* NewCopy() const override { return new name(*this); } \
  ParamGroupUid GetUid() const override { return kUid; }           \
                                                                   \
 private:                                                          \
  BDM_CLASS_DEF_OVERRIDE(name, version_id);                        \
                                                                   \
 public:

}  // namespace bdm

#endif  // CORE_PARAM_PARAM_GROUP_H_
