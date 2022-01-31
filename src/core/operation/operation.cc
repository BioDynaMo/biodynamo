// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#include "core/operation/operation.h"

namespace bdm {

Operation::Operation(const std::string &name) : name_(name) {}

Operation::Operation(const std::string &name, uint32_t frequency)
    : frequency_(frequency), name_(name) {}

Operation::~Operation() {
  for (auto *imp : implementations_) {
    if (imp) {
      delete imp;
    }
  }
}

Operation *Operation::Clone() {
  auto *clone = new Operation(*this);
  // Deep copy of the implementations
  int i = 0;
  for (auto *imp : implementations_) {
    if (imp) {
      clone->implementations_[i] = imp->Clone();
    } else {
      clone->implementations_[i] = nullptr;
    }
    i++;
  }
  return clone;
}

void Operation::operator()(Agent *agent) {
  (*implementations_[active_target_])(agent);
}

void Operation::operator()() { (*implementations_[active_target_])(); }

void Operation::AddOperationImpl(OpComputeTarget target, OperationImpl *impl) {
  if (implementations_.size() < static_cast<size_t>(target + 1)) {
    implementations_.resize(target + 1, nullptr);
  }
  implementations_[target] = impl;
  impl->target_ = target;
}

bool Operation::IsComputeTargetSupported(OpComputeTarget target) {
  if (implementations_.size() < static_cast<size_t>(target + 1)) {
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

void Operation::SetUp() { implementations_[active_target_]->SetUp(); }

void Operation::TearDown() { implementations_[active_target_]->TearDown(); }

}  // namespace bdm
