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

#include "core/operation/operation.h"

namespace bdm {

Operation::Operation(const std::string &name) : name_(name) {}

Operation::Operation(const std::string &name, uint32_t frequency)
    : frequency_(frequency), name_(name) {}

Operation::~Operation() {
  for (auto i : implementations_) {
    delete i;
  }
}

void Operation::operator()(SimObject *so) {
  (*implementations_[active_target_])(so);
}
void Operation::operator()() {
  auto op_impl = implementations_[active_target_];
  if (op_impl->IsGpuOperation()) {
    static_cast<OperationImplGpu *>(op_impl)->InitializeGpuData();
  }
  (*op_impl)();
  if (op_impl->IsGpuOperation()) {
    static_cast<OperationImplGpu *>(op_impl)->UpdateCpuData();
  }
}

void Operation::AddOperationImpl(OpComputeTarget target, OperationImpl *impl) {
  if (implementations_.size() <= target) {
    implementations_.resize(target + 1, nullptr);
  }
  implementations_[target] = impl;
}

OperationImpl *Operation::GetOperationImpl(OpComputeTarget target) {
  if (implementations_.size() <= target) {
    return nullptr;
  }
  return implementations_[target];
}

OperationImpl *Operation::GetActiveOperationImpl() {
  return implementations_[active_target_];
}

bool Operation::IsComputeTargetSupported(OpComputeTarget target) {
  if (implementations_.size() <= target) {
    return false;
  }
  return implementations_[target] != nullptr;
}

void Operation::SelectComputeTarget(OpComputeTarget target) {
  if (!IsComputeTargetSupported(target)) {
    Log::Fatal("Operation::SelectComputeTarget",
               "Compute target not supported");
  }
  active_target_ = target;
}

}  // namespace bdm
