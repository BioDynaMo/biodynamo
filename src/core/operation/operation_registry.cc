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

#include "core/operation/operation_registry.h"

namespace bdm {

OperationRegistry::~OperationRegistry() {
  for (auto &pair : operations_) {
    delete pair.second;
  }
}

OperationRegistry *OperationRegistry::GetInstance() {
  static OperationRegistry operation_registry;
  return &operation_registry;
}

Operation *OperationRegistry::GetOperation(const std::string &op_name) {
  auto search = operations_.find(op_name);
  if (search == operations_.end()) {
    std::string msg = "Operation not found in registry: " + op_name;
    Log::Fatal("OperationRegistry::GetOperation", msg);
  }
  return search->second;
}

bool OperationRegistry::AddOperationImpl(const std::string &op_name,
                                         OpComputeTarget target,
                                         OperationImpl *impl,
                                         size_t frequency) {
  auto op = operations_.find(op_name);
  // If operation doesn't exist yet, make a new operation under given name
  if (op == operations_.end()) {
    operations_.insert(
        std::make_pair(op_name, new Operation(op_name, frequency)));
    op = operations_.find(op_name);
    op->second->AddOperationImpl(target, impl);
  } else if (op->second->implementations_.size() >=
             static_cast<size_t>(target + 1)) {
    // If operation exists, check if the implementation already exists too
    if (op->second->implementations_[target]) {
      Log::Fatal("OperationRegistry::AddOperationImpl", "Operation '", op_name,
                 "' with implementation '", OpComputeTargetString(target),
                 "' already exists in the registry!");
    }
  } else {
    // Add the implementation to the existing operation
    op->second->AddOperationImpl(target, impl);
  }
  return true;
}

OperationRegistry::OperationRegistry() {}

}  // namespace bdm
