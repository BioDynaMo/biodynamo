#include "samples/common/inc/helper_math.h"
#include "gpu/displacement_op_cuda_kernel.h"

#define GpuErrchk(ans) { GpuAssert((ans), __FILE__, __LINE__); }
inline void GpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess)
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (code == cudaErrorInsufficientDriver) {
        printf("This probably means that no CUDA-compatible GPU has been detected. Consider setting the use_opencl flag to \"true\" in the bmd.toml file to use OpenCL instead.\n");
      }
      if (abort) exit(code);
   }
}

// __device__ float squared_euclidian_distance(float3 my_position, float3 nb_position) {
//   const float dx = positions[3*idx + 0] - positions[3*nidx + 0];
//   const float dy = positions[3*idx + 1] - positions[3*nidx + 1];
//   const float dz = positions[3*idx + 2] - positions[3*nidx + 2];
//   return (dx * dx + dy * dy + dz * dz);
// }

__device__ int3 get_box_coordinates(float3 pos, int32_t* grid_dimensions, uint32_t box_length) {
  int3 box_coords;
  box_coords.x = (floor(pos.x) - grid_dimensions[0]) / box_length;
  box_coords.y = (floor(pos.y) - grid_dimensions[1]) / box_length;
  box_coords.z = (floor(pos.z) - grid_dimensions[2]) / box_length;
  return box_coords;
}

__device__ int3 get_box_coordinates_2(uint32_t box_idx, uint32_t* num_boxes_axis_) {
  int3 box_coord;
  box_coord.z = box_idx / (num_boxes_axis_[0]*num_boxes_axis_[1]);
  uint32_t remainder = box_idx % (num_boxes_axis_[0]*num_boxes_axis_[1]);
  box_coord.y = remainder / num_boxes_axis_[0];
  box_coord.x = remainder % num_boxes_axis_[0];
  return box_coord;
}

__device__ uint32_t get_box_id_2(int3 bc, uint32_t* num_boxes_axis) {
  return bc.z * num_boxes_axis[0]*num_boxes_axis[1] + bc.y * num_boxes_axis[0] + bc.x;
}

__device__ uint32_t get_box_id(float3 pos, uint32_t* num_boxes_axis, int32_t* grid_dimensions, uint32_t box_length) {
  int3 box_coords = get_box_coordinates(pos, grid_dimensions, box_length);
  return get_box_id_2(box_coords, num_boxes_axis);
}

__device__ void compute_force(const float3& my_position, float my_diameter, float* positions, float* diameters, uint32_t nidx, float3& result) {
  float r1 = 0.5 * my_diameter;
  float r2 = 0.5 * diameters[nidx];
  // We take virtual bigger radii to have a distant interaction, to get a desired density.
  float additional_radius = 10.0 * 0.15;
  r1 += additional_radius;
  r2 += additional_radius;

  float3 comp;
  comp.x = my_position.x - positions[3*nidx + 0];
  comp.y = my_position.y - positions[3*nidx + 1];
  comp.z = my_position.z - positions[3*nidx + 2];
  float center_distance = length(comp);

  // the overlap distance (how much one penetrates in the other)
  float delta = r1 + r2 - center_distance;

  if (delta < 0) {
    return;
  }

  // to avoid a division by 0 if the centers are (almost) at the same location
  if (center_distance < 0.00000001) {
    result += make_float3(42.0, 42.0, 42.0);
    return;
  }

  // printf("Colliding cell [%d] and [%d]\n", idx, nidx);
  // printf("Delta for neighbor [%d] = %f\n", nidx, delta);

  // the force itself
  float r = (r1 * r2) / (r1 + r2);
  float gamma = 1; // attraction coeff
  float k = 2;     // repulsion coeff
  float f = k * delta - gamma * sqrt(r * delta);

  float module = f / center_distance;
  result += module*comp;
  // result.x += module * comp.x;
  // result.y += module * comp.y;
  // result.z += module * comp.z;
  // printf("%f, %f, %f\n", module * comp1, module * comp2, module * comp3);
  // printf("Force between cell (%u) [%f, %f, %f] & cell (%u) [%f, %f, %f] = %f, %f, %f\n", idx, positions[3*idx + 0], positions[3*idx + 1], positions[3*idx + 2], nidx, positions[3*nidx + 0], positions[3*nidx + 1], positions[3*nidx + 2], module * comp1, module * comp2, module * comp3);
}

__device__ void GetMooreBoxIds(uint32_t box_idx, uint32_t* ret, uint32_t* num_boxes_axis) {
  const int3 moore_offset[27] = {
    make_int3(-1, -1, -1), make_int3(0, -1, -1), make_int3(1, -1, -1),
    make_int3(-1, 0, -1),  make_int3(0, 0, -1),  make_int3(1, 0, -1),
    make_int3(-1, 1, -1),  make_int3(0, 1, -1),  make_int3(1, 1, -1),
    make_int3(-1, -1, 0),  make_int3(0, -1, 0),  make_int3(1, -1, 0),
    make_int3(-1, 0, 0),   make_int3(0, 0, 0),   make_int3(1, 0, 0),
    make_int3(-1, 1, 0),   make_int3(0, 1, 0),   make_int3(1, 1, 0),
    make_int3(-1, -1, 1),  make_int3(0, -1, 1),  make_int3(1, -1, 1),
    make_int3(-1, 0, 1),   make_int3(0, 0, 1),   make_int3(1, 0, 1),
    make_int3(-1, 1, 1),   make_int3(0, 1, 1),   make_int3(1, 1, 1)};

  int3 box_coords = get_box_coordinates_2(box_idx, num_boxes_axis);
  for (unsigned i = 0; i < 27; i++) {
    ret[i] = get_box_id_2(box_coords + moore_offset[i], num_boxes_axis);
  }
}

__constant__ bdm::SimParams params;

__global__ void collide(
       float* positions,
       float* diameters,
       float* tractor_force,
       float* adherence,
       uint32_t* box_id,
       float* mass,
       uint32_t* starts,
       uint16_t* lengths,
       uint32_t* successors,
       float* result) {
  __shared__ uint32_t moore_boxes[27];
  uint32_t tidx = blockIdx.x * blockDim.x + threadIdx.x;
  if (tidx < params.num_objects) {
    float3 collision_force = make_float3(
                                params.timestep * tractor_force[3*tidx + 0],
                                params.timestep * tractor_force[3*tidx + 1],
                                params.timestep * tractor_force[3*tidx + 2]);

    float3 my_position;
    my_position = make_float3(positions[3 * tidx], positions[3 * tidx + 1],
      positions[3 * tidx + 2]);
    float my_diameter = diameters[tidx];

    GetMooreBoxIds(box_id[tidx], &moore_boxes[0], params.num_boxes_axis);
    for (int i = 0; i < 27; i++) {
      uint32_t bidx = moore_boxes[i];
      uint32_t nidx = starts[bidx];
      for (uint16_t nb = 0; nb < lengths[bidx]; nb++) {
        if (nidx != tidx) {
          if (dot(my_position, make_float3(positions[3 * nidx], positions[3 * nidx + 1],
            positions[3 * nidx + 2])) < params.squared_radius) {
            compute_force(my_position, my_diameter, positions, diameters, nidx, collision_force);
          }
        }
        // traverse linked-list
        nidx = successors[nidx];
      }
    }

    // Mass needs to non-zero!
    float mh = params.timestep / mass[tidx];

    if (length(collision_force) > adherence[tidx]) {
      result[3*tidx + 0] = collision_force.x * mh;
      result[3*tidx + 1] = collision_force.y * mh;
      result[3*tidx + 2] = collision_force.z * mh;

      if (length(collision_force) * mh > params.max_displacement) {
        result[3*tidx + 0] = params.max_displacement;
        result[3*tidx + 1] = params.max_displacement;
        result[3*tidx + 2] = params.max_displacement;
      }
    }
  }
}

bdm::DisplacementOpCudaKernel::DisplacementOpCudaKernel(uint32_t num_objects, uint32_t num_boxes) {
  GpuErrchk(cudaMalloc(&d_positions_, 3 * num_objects * sizeof(float)));
  GpuErrchk(cudaMalloc(&d_diameters_, num_objects * sizeof(float)));
  GpuErrchk(cudaMalloc(&d_tractor_force_, 3 * num_objects * sizeof(float)));
  GpuErrchk(cudaMalloc(&d_adherence_, num_objects * sizeof(float)));
  GpuErrchk(cudaMalloc(&d_box_id_, num_objects * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_mass_, num_objects * sizeof(float)));
  GpuErrchk(cudaMalloc(&d_starts_, num_boxes * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_lengths_, num_boxes * sizeof(uint16_t)));
  GpuErrchk(cudaMalloc(&d_successors_, num_objects * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_cell_movements_, 3 * num_objects * sizeof(float)));
}

void bdm::DisplacementOpCudaKernel::LaunchDisplacementKernel(float* positions, float* diameters, float* tractor_force,
                    float* adherence, uint32_t* box_id, float* mass,
                    uint32_t* starts, uint16_t* lengths, uint32_t* successors,
                    float* cell_movements, SimParams host_params) {
  uint32_t num_boxes = host_params.num_boxes_axis[0] * host_params.num_boxes_axis[1] * host_params.num_boxes_axis[2];

  GpuErrchk(cudaMemcpy(d_positions_, 		positions, 3 * host_params.num_objects * sizeof(float), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_diameters_, 		diameters, host_params.num_objects * sizeof(float), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_tractor_force_, 	tractor_force, 3 * host_params.num_objects * sizeof(float), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_adherence_,     adherence, host_params.num_objects * sizeof(float), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_box_id_, 		box_id, host_params.num_objects * sizeof(uint32_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_mass_, 				mass, host_params.num_objects * sizeof(float), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_starts_, 			starts, num_boxes * sizeof(uint32_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_lengths_, 			lengths, num_boxes * sizeof(uint16_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_successors_, 		successors, host_params.num_objects * sizeof(uint32_t), cudaMemcpyHostToDevice));

  cudaMemcpyToSymbol(params, &host_params, sizeof(SimParams));

  int blockSize = 128;
  int minGridSize;
  int gridSize;

  // Get a near-optimal occupancy with the following thread organization
  cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, collide, 0, host_params.num_objects);
  gridSize = (host_params.num_objects + blockSize - 1) / blockSize;

  // printf("gridSize = %d  |  blockSize = %d\n", gridSize, blockSize);
  collide<<<gridSize, blockSize>>>(d_positions_, d_diameters_, d_tractor_force_,
    d_adherence_, d_box_id_, d_mass_, d_starts_, d_lengths_, d_successors_,
    d_cell_movements_);

  // We need to wait for the kernel to finish before reading back the result
  cudaDeviceSynchronize();
  cudaMemcpy(cell_movements, d_cell_movements_, 3 * host_params.num_objects * sizeof(float), cudaMemcpyDeviceToHost);
}

void bdm::DisplacementOpCudaKernel::ResizeCellBuffers(uint32_t num_cells) {
  cudaFree(d_positions_);
  cudaFree(d_diameters_);
  cudaFree(d_tractor_force_);
  cudaFree(d_adherence_);
  cudaFree(d_box_id_);
  cudaFree(d_mass_);
  cudaFree(d_successors_);
  cudaFree(d_cell_movements_);

  cudaMalloc(&d_positions_, 3 * num_cells * sizeof(float));
  cudaMalloc(&d_diameters_, num_cells * sizeof(float));
  cudaMalloc(&d_tractor_force_, 3 * num_cells * sizeof(float));
  cudaMalloc(&d_adherence_, num_cells * sizeof(float));
  cudaMalloc(&d_box_id_, num_cells * sizeof(uint32_t));
  cudaMalloc(&d_mass_, num_cells * sizeof(float));
  cudaMalloc(&d_successors_, num_cells * sizeof(uint32_t));
  cudaMalloc(&d_cell_movements_, 3 * num_cells * sizeof(float));
}

void bdm::DisplacementOpCudaKernel::ResizeGridBuffers(uint32_t num_boxes) {
  cudaFree(d_starts_);
  cudaFree(d_lengths_);

  cudaMalloc(&d_starts_, num_boxes * sizeof(uint32_t));
  cudaMalloc(&d_lengths_, num_boxes * sizeof(uint16_t));
}

bdm::DisplacementOpCudaKernel::~DisplacementOpCudaKernel() {
  cudaFree(d_positions_);
  cudaFree(d_diameters_);
  cudaFree(d_tractor_force_);
  cudaFree(d_adherence_);
  cudaFree(d_box_id_);
  cudaFree(d_mass_);
  cudaFree(d_starts_);
  cudaFree(d_lengths_);
  cudaFree(d_successors_);
  cudaFree(d_cell_movements_);
}
