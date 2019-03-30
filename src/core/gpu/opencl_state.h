#ifndef CORE_GPU_GPU_OPENCL_STATE_H_
#define CORE_GPU_GPU_OPENCL_STATE_H_

#ifdef USE_OPENCL

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include <vector>

/// Class containing the state of OpenCL objects required to run OpenCL kernels.
/// Currently only support for one GPU device
class OpenCLState {
 public:
  static OpenCLState& GetInstance() {
    static OpenCLState instance;
    return instance;
  }

  OpenCLState() {}

  OpenCLState(OpenCLState const&);
  void operator=(OpenCLState const&);

  cl::Context* GetOpenCLContext() { return &opencl_context_; }
  cl::CommandQueue* GetOpenCLCommandQueue() { return &opencl_command_queue_; }
  std::vector<cl::Device>* GetOpenCLDeviceList() { return &opencl_devices_; }
  std::vector<cl::Program>* GetOpenCLProgramList() { return &opencl_programs_; }

 private:
  cl::Context opencl_context_;             //!
  cl::CommandQueue opencl_command_queue_;  //!
  std::vector<cl::Device> opencl_devices_;    //!
  std::vector<cl::Program> opencl_programs_;  //!
};

#endif

#endif  // CORE_GPU_GPU_OPENCL_STATE_H_
