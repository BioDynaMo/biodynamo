#ifndef DISPLACEMENT_OP_CUDA_KERNEL_H_
#define DISPLACEMENT_OP_CUDA_KERNEL_H_

#include <math.h>
#include <stdint.h>
#include "stdio.h"

namespace bdm {

class DisplacementOpCudaKernel {
 public:
 	DisplacementOpCudaKernel(uint32_t N, uint32_t num_boxes);
 	virtual ~DisplacementOpCudaKernel();

	void displacement_op_cuda(double* positions, double* diameter, double* tractor_force, double* adherence, uint32_t* box_id, double* mass, double* timestep, double* max_displacement, double* squared_radius, uint32_t* N, uint32_t* starts, uint16_t* lengths, uint32_t* successors, uint32_t* box_length, uint32_t* num_boxes_axis, int32_t* grid_dimensions, double* cell_movements);

  void resize_cell_buffers(uint32_t N);
  void resize_grid_buffers(uint32_t num_boxes);

 private:
  double* d_positions = NULL;
  double* d_diameters = NULL;
  double* d_mass = NULL;
  double* d_timestep = NULL;
  double* d_max_displacement = NULL;
  double* d_squared_radius = NULL;
  uint32_t* d_N = NULL;
  double* d_cell_movements = NULL;
  double* d_tractor_force = NULL;
  double* d_adherence = NULL;
  uint32_t* d_box_id = NULL;
  uint32_t* d_starts = NULL;
  uint16_t* d_lengths = NULL;
  uint32_t* d_successors = NULL;
  uint32_t* d_box_length = NULL;
  uint32_t* d_num_boxes_axis = NULL;
  int32_t* d_grid_dimensions = NULL;
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_CUDA_KERNEL_H_
