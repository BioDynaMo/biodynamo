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

#ifndef CORE_OPERATION_VTUNE_HELPER_H_
#define CORE_OPERATION_VTUNE_HELPER_H_

#include "core/util/vtune.h"

namespace bdm {

/// Wrapper class to track the execution of a task with VTune.
/// Usage:
/// ...
///   VTuneTask foo("my_task_name");
///   foo.Start();
///   MyOp op; op();
///   foo.Stop();
/// ...
/// In VTune you will see the above operation in the analysis results marked
/// with "my_task_name"
class VTuneTask {
 public:
  explicit VTuneTask(std::string task_name) {
    domain_ = __itt_domain_create("BDMTraces.MyDomain");
    task_ = __itt_string_handle_create(task_name.c_str());
  }

  void Start() { __itt_task_begin(domain_, __itt_null, __itt_null, task_); }

  void Stop() { __itt_task_end(domain_); }

 private:
  __itt_domain* domain_;
  __itt_string_handle* task_;
};

}  // namespace bdm

#endif  // CORE_OPERATION_VTUNE_HELPER_H_
