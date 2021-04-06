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

#ifndef PIMPL_HELPERS_OPENCL_
#define PIMPL_HELPERS_OPENCL_
namespace cl {
class Kernel;
class Context;
class CommandQueue;
class Device;
class Program;
};      // namespace cl
#endif  // PIMPL_HELPERS_OPENCL_

#ifndef CORE_GPU_OPENCL_STATE_H_
#define CORE_GPU_OPENCL_STATE_H_

#include <memory>
#include <vector>

namespace bdm {

class OpenCLState {
 public:
  OpenCLState();

  /// Returns the OpenCL Context
  cl::Context* GetOpenCLContext();

  void AddKernel(std::string, cl::Kernel);

  cl::Kernel GetKernel(std::string);

  void SetInitialization(bool b);

  bool IsInitialized();

  /// Returns the OpenCL command queue
  cl::CommandQueue* GetOpenCLCommandQueue();

  /// Returns the OpenCL device (GPU) list
  std::vector<cl::Device>* GetOpenCLDeviceList();

  /// Returns the OpenCL program (kernel) list
  std::vector<cl::Program>* GetOpenCLProgramList();

  const char* GetErrorString(int error);

  int ClAssert(int const code, char const* const file, int const line,
               bool const abort);

 private:
  struct OpenCLImpl;
  // We need a destructor implementation because OpenCLImpl is forward declared
  struct OpenCLImplDestructor {
    void operator()(OpenCLImpl* p);
  };
  std::unique_ptr<OpenCLImpl, OpenCLImplDestructor> impl_;  //!
  // Flag to check if OpenCL was initialized correctly
  bool initialized_ = false;
};

}  // namespace bdm

#endif  // CORE_GPU_OPENCL_STATE_H_
