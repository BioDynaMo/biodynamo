#ifndef DISPLACEMENT_OP_CUDA_KERNEL_H_
#define DISPLACEMENT_OP_CUDA_KERNEL_H_

#include <math.h>
#include <stdint.h>
#include "stdio.h"

namespace bdm {

struct SimParams {
  uint32_t num_objects;
  int32_t grid_dimensions[3];
  uint32_t num_boxes_axis[3];
  uint32_t box_length;
  float timestep;
  float squared_radius;
  float max_displacement;
};

class DisplacementOpCudaKernel {
 public:
  DisplacementOpCudaKernel(uint32_t num_objects, uint32_t num_boxes);
  virtual ~DisplacementOpCudaKernel();

  void LaunchDisplacementKernel(
      float* positions, float* diameters, float* tractor_force,
      float* adherence, uint32_t* box_id, float* mass,
      uint32_t* starts, uint16_t* lengths, uint32_t* successors,
      float* cell_movements, SimParams host_params);

  void ResizeCellBuffers(uint32_t num_cells);
  void ResizeGridBuffers(uint32_t num_boxes);

 private:
  float* d_positions_ = NULL;
  float* d_diameters_ = NULL;
  float* d_mass_ = NULL;
  float* d_cell_movements_ = NULL;
  float* d_tractor_force_ = NULL;
  float* d_adherence_ = NULL;
  uint32_t* d_box_id_ = NULL;
  uint32_t* d_starts_ = NULL;
  uint16_t* d_lengths_ = NULL;
  uint32_t* d_successors_ = NULL;
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_CUDA_KERNEL_H_
