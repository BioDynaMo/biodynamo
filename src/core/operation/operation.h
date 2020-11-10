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

#ifndef CORE_OPERATION_OPERATION_H_
#define CORE_OPERATION_OPERATION_H_

#include <functional>
#include <string>
#include <vector>

#include "core/util/log.h"

namespace bdm {

class Agent;

enum OpComputeTarget { kCpu, kCuda, kOpenCl };

inline std::string OpComputeTargetString(OpComputeTarget t) {
  switch (t) {
    case OpComputeTarget::kCpu:
      return "kCpu";
    case OpComputeTarget::kCuda:
      return "kCuda";
    case OpComputeTarget::kOpenCl:
      return "kOpenCl";
    default:
      return "Invalid";
  }
}

struct OperationImpl {
  virtual ~OperationImpl() {}

  /// This function is run before the operator() call. It is useful to perform
  /// tasks such as data transfer from CPU -> GPU in GPU operations
  virtual void SetUp() {}

  /// This function is run after the operator() call. It is useful to perform
  /// tasks such as data transfer from GPU -> CPU in GPU operations
  virtual void TearDown() {}

  virtual void operator()(Agent *agent) = 0;

  virtual void operator()() = 0;

  /// Operation implementations can be cloned. This function should return a
  /// copy of the operation implementation
  virtual OperationImpl *Clone() = 0;

  /// Returns whether or not this operation is supposed to run on a GPU
  bool IsGpuOperation() { return target_ == kCuda || target_ == kOpenCl; }

  /// Returns whether or not this operations is a stand-alone operation
  virtual bool IsStandalone() = 0;

  /// The target that this operation implementation is supposed to run on
  OpComputeTarget target_ = kCpu;
};

/// Interface for implementing an operation
struct AgentOperationImpl : public OperationImpl {
  void operator()() override {
    Log::Fatal("AgentOperationImpl",
               "AgentOperationImpl do not support this function operator");
  }

  bool IsStandalone() override { return false; }
};

/// Interface for implementing an operation that should run on a GPU
struct StandaloneOperationImpl : public OperationImpl {
  void operator()(Agent *agent) override {
    Log::Fatal("StandaloneOperationImpl",
               "StandaloneOperationImpl do not support this function operator");
  }

  bool IsStandalone() override { return true; }
};

/// A BioDynaMo operation that is executed every `frequency_` timesteps. An
/// operation can have multiple implementations for various execution platform,
/// or "compute targets", such as CUDA or OpenCL, to target GPU hardware for
/// instance.
struct Operation {
  /// Construct an operation
  ///
  /// @param[in]  name  The name of the operation
  ///
  explicit Operation(const std::string &name);

  /// Construct an operation
  ///
  /// @param[in]  name       The name of the operation
  /// @param[in]  frequency  The frequency at which the operation is executed
  ///
  Operation(const std::string &name, uint32_t frequency);

  ~Operation();

  Operation *Clone();

  /// Operate on an individual agent. Typically this operator is
  /// called in a loop over all agents
  ///
  /// @param      agent    Handle to the agent
  ///
  void operator()(Agent *agent);

  /// Operate in a stand-alone fashion. Typically this operator is called for
  /// GPU operations, or operations that do not need to loop over simulation
  /// objects (such as updating diffusion grids)
  void operator()();

  /// Add an operation implementation for the specified compute target
  ///
  /// @param[in]  target  The compute target
  /// @param      impl    The implementation
  ///
  void AddOperationImpl(OpComputeTarget target, OperationImpl *impl);

  /// Returns the implementation corresponding to the template argument
  template <typename T>
  T *GetImplementation() {
    T *implementation = nullptr;
    // Go over the available implementations and return the requested one
    for (auto *imp : implementations_) {
      if (dynamic_cast<T *>(imp)) {
        implementation = dynamic_cast<T *>(imp);
      }
    }
    return implementation;
  }

  /// Check whether an implementation is available for the requested compute
  /// target
  ///
  /// @param[in]  target  The compute target
  ///
  /// @return     True if the specified compute target is supported, False
  ///             otherwise.
  ///
  bool IsComputeTargetSupported(OpComputeTarget target);

  /// Select which of the operation implementation should be used for this
  /// operation, by specifying the compute target
  ///
  /// @param[in]  target  The compute target
  ///
  void SelectComputeTarget(OpComputeTarget target);

  bool IsStandalone() {
    return implementations_[active_target_]->IsStandalone();
  }

  /// Forwards call to implementation's Setup function
  void SetUp();

  /// Forwards call to implementation's TearDown function
  void TearDown();

  /// Specifies how often this operation will be executed.\n
  /// 1: every timestep\n
  /// 2: every second timestep\n
  /// ...
  size_t frequency_ = 1;
  /// Operation name / unique identifier
  std::string name_;
  /// The compute target that this operation will be executed on
  OpComputeTarget active_target_ = kCpu;
  /// The different operation implementations for each supported compute target
  std::vector<OperationImpl *> implementations_;
};

}  // namespace bdm

#endif  // CORE_OPERATION_OPERATION_H_
