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

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef USE_OPENCL
#ifdef __APPLE__
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include "cl2.hpp"
#else
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 210
#define CL_HPP_TARGET_OPENCL_VERSION 210
#include <CL/cl2.hpp>
#endif
#endif  // USE_OPENCL

#ifdef USE_CUDA
#include "cuda_runtime_api.h"
#endif  // USE_CUDA

#include "core/gpu/gpu_helper.h"
#include "core/param/param.h"
#include "core/simulation.h"
#include "core/util/log.h"

namespace bdm {

GpuHelper* GpuHelper::GetInstance() {
  static GpuHelper kInstance;
  return &kInstance;
}
#ifdef USE_CUDA
void GpuHelper::FindGpuDevicesCuda() {
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

  cudaSetDevice(param->preferred_gpu);
  cudaDeviceProp prop;
  cudaGetDeviceProperties(&prop, param->preferred_gpu);
  Log::Info("", "Selected GPU [", param->preferred_gpu, "]: ", prop.name);
}
#endif  // USE_CUDA

#ifdef USE_OPENCL
#define ClOk(err)                                                              \
  Simulation::GetActive()->GetOpenCLState()->ClAssert(err, __FILE__, __LINE__, \
                                                      true);

void GpuHelper::CompileOpenCLKernels() {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  auto* ocl_state = sim->GetOpenCLState();

  std::vector<cl::Program>* all_programs = ocl_state->GetOpenCLProgramList();
  cl::Context* context = ocl_state->GetOpenCLContext();
  std::vector<cl::Device>* devices = ocl_state->GetOpenCLDeviceList();
  // Compile OpenCL program for found device
  // TODO(ahmad): create more convenient way to compile all OpenCL kernels, by
  // going through a list of header files. Also, create a stringifier that
  // goes from .cl --> .h, since OpenCL kernels must be input as a string here
  std::string bdmsys = std::getenv("BDMSYS");
  std::ifstream cl_file(
      bdmsys + "/include/core/gpu/mechanical_forces_op_opencl_kernel.cl");
  if (cl_file.fail()) {
    Log::Error("CompileOpenCLKernels", "Kernel file does not exists!");
  }
  std::stringstream buffer;
  buffer << cl_file.rdbuf();

  cl::Program mechanical_forces_op_program(*context, buffer.str());

  all_programs->push_back(mechanical_forces_op_program);

  Log::Info("", "Compiling OpenCL kernels...");

  std::string options;
  if (param->opencl_debug) {
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

void GpuHelper::FindGpuDevicesOpenCL() {
  try {
    // We keep the context and device list in the resource manager to be
    // accessible elsewhere to create command queues and buffers from
    auto* sim = Simulation::GetActive();
    auto* param = sim->GetParam();
    auto* ocl_state = sim->GetOpenCLState();

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

    // Go over all available platforms and devices until all compatible
    // devices are found
    for (auto p = platform.begin(); p != platform.end(); p++) {
      std::vector<cl::Device> pldev;

      try {
        // Only select GPU's
        p->getDevices(CL_DEVICE_TYPE_GPU, &pldev);

        for (auto d = pldev.begin(); d != pldev.end(); d++) {
          if (!d->getInfo<CL_DEVICE_AVAILABLE>())  // NOLINT
            continue;

          // Only select GPU's with real_t support
          if (!d->getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>())  // NOLINT
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
      Log::Fatal("FindGpuDevicesOpenCL",
                 "No OpenCL compatible GPU's found on this machine!");
      return;
    }

    *context = cl::Context(*devices);

    Log::Info("", "Found ", devices->size(),
              " OpenCL-compatible GPU device(s): ");

    for (size_t i = 0; i < devices->size(); i++) {
      Log::Info("", "  [", i, "] ", (*devices)[i].getInfo<CL_DEVICE_NAME>());
    }

    int selected_gpu = param->preferred_gpu;
    Log::Info("", "Selected GPU [", selected_gpu,
              "]: ", (*devices)[selected_gpu].getInfo<CL_DEVICE_NAME>());

    // Create command queue for that GPU
    cl_int queue_err;
    *queue = cl::CommandQueue(*context, (*devices)[selected_gpu],
                              CL_QUEUE_PROFILING_ENABLE, &queue_err);
    ClOk(queue_err);

    // Compile the OpenCL kernels
    CompileOpenCLKernels();
  } catch (const cl::Error& err) {
    Log::Error("FindGpuDevicesOpenCL", "OpenCL error: ", err.what(), "(",
               err.err(), ")");
  }
}
#endif  // defined(USE_OPENCL)

void GpuHelper::InitializeGPUEnvironment() {
#if (defined(USE_CUDA) || defined(USE_OPENCL))
  auto* param = Simulation::GetActive()->GetParam();
  if (param->compute_target == "opencl") {
#ifdef USE_OPENCL
    GpuHelper::FindGpuDevicesOpenCL();
#else
    Log::Fatal("InitializeGPUEnvironment",
               "You tried to use the GPU (OpenCL) version of BioDynaMo, but no "
               "OpenCL installation was detected on this machine. Switching to "
               "the CPU version...");
#endif  // USE_OPENCL
  } else {
#ifdef USE_CUDA
    GpuHelper::FindGpuDevicesCuda();
#else
    Log::Fatal("InitializeGPUEnvironment",
               "You tried to use the GPU (CUDA) version of BioDynaMo, but no "
               "CUDA installation was detected on this machine. Switching to "
               "the CPU version...");
#endif  // USE_CUDA
  }
#endif  // defined(USE_CUDA) || defined(USE_OPENCL)
}

}  // namespace bdm
