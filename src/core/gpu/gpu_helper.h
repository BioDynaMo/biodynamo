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

#ifndef CORE_GPU_GPU_HELPER_H_
#define CORE_GPU_GPU_HELPER_H_

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef USE_OPENCL
#define __CL_ENABLE_EXCEPTIONS
#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#endif

#ifdef USE_CUDA
#include "cuda_runtime_api.h"
#endif

#include "core/param/param.h"
#include "core/simulation.h"
#include "core/util/log.h"

namespace bdm {

#ifdef USE_CUDA
static void FindGpuDevicesCuda() {
  auto* param = Simulation::GetActive()->GetParam();

  int n_devices = 0;

  cudaGetDeviceCount(&n_devices);
  if (n_devices == 0) {
    Log::Fatal("FindGpuDevicesCuda",
               "No CUDA-compatible GPU found on this machine!");
    return;
  }

  Log::Info("", "Found ", n_devices, " CUDA-compatible GPU device(s): ");

  for (int i = 0; i < n_devices; i++) {
    cudaSetDevice(i);
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, i);
    Log::Info("", "  [", i, "] ", prop.name);
  }

  cudaSetDevice(param->preferred_gpu_);
  cudaDeviceProp prop;
  cudaGetDeviceProperties(&prop, param->preferred_gpu_);
  Log::Info("", "Selected GPU [", param->preferred_gpu_, "]: ", prop.name);
}
#endif

#ifdef USE_OPENCL
class OpenCLState {
 public:
  static OpenCLState* GetInstance() {
    static OpenCLState kInstance;
    return &kInstance;
  }

  /// Returns the OpenCL Context
  cl::Context* GetOpenCLContext() { return &opencl_context_; }

  /// Returns the OpenCL command queue
  cl::CommandQueue* GetOpenCLCommandQueue() { return &opencl_command_queue_; }

  /// Returns the OpenCL device (GPU) list
  std::vector<cl::Device>* GetOpenCLDeviceList() { return &opencl_devices_; }

  /// Returns the OpenCL program (kernel) list
  std::vector<cl::Program>* GetOpenCLProgramList() { return &opencl_programs_; }

 private:
   cl::Context opencl_context_;             //!
   cl::CommandQueue opencl_command_queue_;  //!
   // Currently only support for one GPU device
   std::vector<cl::Device> opencl_devices_;    //!
   std::vector<cl::Program> opencl_programs_;  //!
};

static inline cl_int ClAssert(cl_int const code, char const* const file,
                              int const line, bool const abort);
static const char* GetErrorString(cl_int error);
#define ClOk(err) ClAssert(err, __FILE__, __LINE__, true);

static void CompileOpenCLKernels() {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  auto* ocl_state = OpenCLState::GetInstance();

  std::vector<cl::Program>* all_programs = ocl_state->GetOpenCLProgramList();
  cl::Context* context = ocl_state->GetOpenCLContext();
  std::vector<cl::Device>* devices = ocl_state->GetOpenCLDeviceList();
  // Compile OpenCL program for found device
  // TODO(ahmad): create more convenient way to compile all OpenCL kernels, by
  // going through a list of header files. Also, create a stringifier that goes
  // from .cl --> .h, since OpenCL kernels must be input as a string here
  std::ifstream cl_file(BDM_SRC_DIR "/core/gpu/displacement_op_opencl_kernel.cl");
  if (cl_file.fail()) {
    Log::Error("CompileOpenCLKernels", "Kernel file does not exists!");
  }
  std::stringstream buffer;
  buffer << cl_file.rdbuf();

  cl::Program displacement_op_program(
      *context,
      cl::Program::Sources(1, std::make_pair(buffer.str().c_str(),
                                             strlen(buffer.str().c_str()))));

  all_programs->push_back(displacement_op_program);

  Log::Info("", "Compiling OpenCL kernels...");

  std::string options;
  if (param->opencl_debug_) {
    Log::Info("", "Building OpenCL kernels with debugging symbols");
    options = "-g -O0";
  } else {
    Log::Info("", "Building OpenCL kernels without debugging symbols");
  }

  for (auto& prog : *all_programs) {
    try {
      prog.build(*devices, options.c_str());
    } catch (const cl::Error&) {
      Log::Error("CompileOpenCLKernels", "OpenCL compilation error: ",
                 prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>((*devices)[0]));
    }
  }
}

static void FindGpuDevicesOpenCL() {
  try {
    // We keep the context and device list in the resource manager to be
    // accessible elsewhere to create command queues and buffers from
    auto* sim = Simulation::GetActive();
    auto* param = sim->GetParam();
    auto* ocl_state = OpenCLState::GetInstance();

    cl::Context* context = ocl_state->GetOpenCLContext();
    cl::CommandQueue* queue = ocl_state->GetOpenCLCommandQueue();
    std::vector<cl::Device>* devices = ocl_state->GetOpenCLDeviceList();

    // Get list of OpenCL platforms.
    std::vector<cl::Platform> platform;
    // If we get stuck here, it might be because the DISPLAY envar is not set.
    // Set it to 0 to avoid getting stuck. It's an AMD specific issue
    cl::Platform::get(&platform);

    if (platform.empty()) {
      Log::Error("FindGpuDevicesOpenCL", "No OpenCL platforms found");
    }

    // Go over all available platforms and devices until first device is found
    for (auto p = platform.begin(); p != platform.end(); p++) {
      std::vector<cl::Device> pldev;

      try {
        p->getDevices(CL_DEVICE_TYPE_GPU, &pldev);

        for (auto d = pldev.begin(); d != pldev.end(); d++) {
          if (!d->getInfo<CL_DEVICE_AVAILABLE>())  // NOLINT
            continue;

          // The OpenCL extension available on this device
          std::string ext = d->getInfo<CL_DEVICE_EXTENSIONS>();

          devices->push_back(*d);
        }
      } catch (...) {
        Log::Error("FindGpuDevicesOpenCL",
                   "Found bad OpenCL platform! Continuing to next one");
        devices->clear();
        continue;
      }
    }

    if (devices->empty()) {
      Log::Fatal("FindGpuDevicesCuda",
               "No CUDA-compatible GPU found on this machine!");
      return;
    }

    *context = cl::Context(*devices);

    Log::Info("", "Found ", devices->size(),
              " OpenCL-compatible GPU device(s): ");

    for (size_t i = 0; i < devices->size(); i++) {
      Log::Info("", "  [", i, "] ", (*devices)[i].getInfo<CL_DEVICE_NAME>());
    }

    int selected_gpu = param->preferred_gpu_;
    Log::Info("", "Selected GPU [", selected_gpu, "]: ",
              (*devices)[selected_gpu].getInfo<CL_DEVICE_NAME>());

    // Create command queue for that GPU
    cl_int queue_err;
    *queue = cl::CommandQueue(*context, (*devices)[param->preferred_gpu_],
                              CL_QUEUE_PROFILING_ENABLE, &queue_err);
    ClOk(queue_err);

    // Compile the OpenCL kernels
    CompileOpenCLKernels();
  } catch (const cl::Error& err) {
    Log::Error("FindGpuDevicesOpenCL", "OpenCL error: ", err.what(), "(",
               err.err(), ")");
  }
}
#endif

static void InitializeGPUEnvironment() {
  auto* param = Simulation::GetActive()->GetParam();
  if (param->use_opencl_) {
#ifdef USE_OPENCL
    FindGpuDevicesOpenCL();
#else
    Log::Fatal("InitializeGPUEnvironment",
               "You tried to use the GPU (OpenCL) version of BioDynaMo, but no "
               "OpenCL installation was detected on this machine. Switching to "
               "the CPU version...");
#endif
  } else {
#ifdef USE_CUDA
    FindGpuDevicesCuda();
#else
    Log::Fatal("InitializeGPUEnvironment",
               "You tried to use the GPU (CUDA) version of BioDynaMo, but no "
               "CUDA installation was detected on this machine. Switching to "
               "the CPU version...");
#endif
  }
}

#ifdef USE_OPENCL
const char* GetErrorString(cl_int error) {
  switch (error) {
    // run-time and JIT compiler errors
    case 0:
      return "CL_SUCCESS";
    case -1:
      return "CL_DEVICE_NOT_FOUND";
    case -2:
      return "CL_DEVICE_NOT_AVAILABLE";
    case -3:
      return "CL_COMPILER_NOT_AVAILABLE";
    case -4:
      return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5:
      return "CL_OUT_OF_RESOURCES";
    case -6:
      return "CL_OUT_OF_HOST_MEMORY";
    case -7:
      return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8:
      return "CL_MEM_COPY_OVERLAP";
    case -9:
      return "CL_IMAGE_FORMAT_MISMATCH";
    case -10:
      return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11:
      return "CL_BUILD_PROGRAM_FAILURE";
    case -12:
      return "CL_MAP_FAILURE";
    case -13:
      return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14:
      return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15:
      return "CL_COMPILE_PROGRAM_FAILURE";
    case -16:
      return "CL_LINKER_NOT_AVAILABLE";
    case -17:
      return "CL_LINK_PROGRAM_FAILURE";
    case -18:
      return "CL_DEVICE_PARTITION_FAILED";
    case -19:
      return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30:
      return "CL_INVALID_VALUE";
    case -31:
      return "CL_INVALID_DEVICE_TYPE";
    case -32:
      return "CL_INVALID_PLATFORM";
    case -33:
      return "CL_INVALID_DEVICE";
    case -34:
      return "CL_INVALID_CONTEXT";
    case -35:
      return "CL_INVALID_QUEUE_PROPERTIES";
    case -36:
      return "CL_INVALID_COMMAND_QUEUE";
    case -37:
      return "CL_INVALID_HOST_PTR";
    case -38:
      return "CL_INVALID_MEM_OBJECT";
    case -39:
      return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40:
      return "CL_INVALID_IMAGE_SIZE";
    case -41:
      return "CL_INVALID_SAMPLER";
    case -42:
      return "CL_INVALID_BINARY";
    case -43:
      return "CL_INVALID_BUILD_OPTIONS";
    case -44:
      return "CL_INVALID_PROGRAM";
    case -45:
      return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46:
      return "CL_INVALID_KERNEL_NAME";
    case -47:
      return "CL_INVALID_KERNEL_DEFINITION";
    case -48:
      return "CL_INVALID_KERNEL";
    case -49:
      return "CL_INVALID_ARG_INDEX";
    case -50:
      return "CL_INVALID_ARG_VALUE";
    case -51:
      return "CL_INVALID_ARG_SIZE";
    case -52:
      return "CL_INVALID_KERNEL_ARGS";
    case -53:
      return "CL_INVALID_WORK_DIMENSION";
    case -54:
      return "CL_INVALID_WORK_GROUP_SIZE";
    case -55:
      return "CL_INVALID_WORK_ITEM_SIZE";
    case -56:
      return "CL_INVALID_GLOBAL_OFFSET";
    case -57:
      return "CL_INVALID_EVENT_WAIT_LIST";
    case -58:
      return "CL_INVALID_EVENT";
    case -59:
      return "CL_INVALID_OPERATION";
    case -60:
      return "CL_INVALID_GL_OBJECT";
    case -61:
      return "CL_INVALID_BUFFER_SIZE";
    case -62:
      return "CL_INVALID_MIP_LEVEL";
    case -63:
      return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64:
      return "CL_INVALID_PROPERTY";
    case -65:
      return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66:
      return "CL_INVALID_COMPILER_OPTIONS";
    case -67:
      return "CL_INVALID_LINKER_OPTIONS";
    case -68:
      return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000:
      return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001:
      return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002:
      return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003:
      return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004:
      return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005:
      return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default:
      return "Unknown OpenCL error";
  }
}

cl_int ClAssert(cl_int const code, char const* const file, int const line,
                bool const abort) {
  if (code != CL_SUCCESS) {
    char const* const err_str = GetErrorString(code);

    fprintf(stderr, "\"%s\", line %d: ClAssert (%d) = \"%s\"", file, line, code,
            err_str);

    if (abort) {
      // stop profiling and reset device here if necessary
      exit(code);
    }
  }

  return code;
}
#endif

}  // namespace bdm

#endif  // CORE_GPU_GPU_HELPER_H_
