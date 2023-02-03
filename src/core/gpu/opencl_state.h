// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

  /// Returns the OpenCL command queue
  cl::CommandQueue* GetOpenCLCommandQueue();

  /// Returns the OpenCL device (GPU) list
  std::vector<cl::Device>* GetOpenCLDeviceList();

  /// Returns the OpenCL program (kernel) list
  std::vector<cl::Program>* GetOpenCLProgramList();

  std::vector<bool>* GetFp64Support();

  void SelectGpu(int gpu);

  /// Returns whether or not a GPU without support for double-precision was
  /// found
  bool HasSupportForDouble();

  /// Disable support for double-precision floating point operations
  void DisableSupportForDouble();

  /// Enable support for double-precision floating point operations
  void EnableSupportForDouble();

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
};

}  // namespace bdm

#endif  // CORE_GPU_OPENCL_STATE_H_
