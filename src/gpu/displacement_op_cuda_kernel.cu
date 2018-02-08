#include "helper_math.h"
#include "gpu/displacement_op_cuda_kernel.h"

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
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

__device__ double norm(double3 v) {
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

__device__ double squared_euclidian_distance(double* positions, uint32_t idx, uint32_t nidx) {
  const double dx = positions[3*idx + 0] - positions[3*nidx + 0];
  const double dy = positions[3*idx + 1] - positions[3*nidx + 1];
  const double dz = positions[3*idx + 2] - positions[3*nidx + 2];
  return (dx * dx + dy * dy + dz * dz);
}

__device__ int3 get_box_coordinates(double3 pos, int32_t* grid_dimensions, uint32_t box_length) {
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

__device__ uint32_t get_box_id(double3 pos, uint32_t* num_boxes_axis, int32_t* grid_dimensions, uint32_t box_length) {
  int3 box_coords = get_box_coordinates(pos, grid_dimensions, box_length);
  return get_box_id_2(box_coords, num_boxes_axis);
}

__device__ void compute_force(double* positions, double* diameters, uint32_t idx, uint32_t nidx, double3* result) {
  double r1 = 0.5 * diameters[idx];
  double r2 = 0.5 * diameters[nidx];
  // We take virtual bigger radii to have a distant interaction, to get a desired density.
  double additional_radius = 10.0 * 0.15;
  r1 += additional_radius;
  r2 += additional_radius;

  double comp1 = positions[3*idx + 0] - positions[3*nidx + 0];
  double comp2 = positions[3*idx + 1] - positions[3*nidx + 1];
  double comp3 = positions[3*idx + 2] - positions[3*nidx + 2];
  double center_distance = sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);

  // the overlap distance (how much one penetrates in the other)
  double delta = r1 + r2 - center_distance;

  if (delta < 0) {
    return;
  }

  // to avoid a division by 0 if the centers are (almost) at the same location
  if (center_distance < 0.00000001) {
    result->x += 42.0;
    result->y += 42.0;
    result->z += 42.0;
    return;
  }

  // printf("Colliding cell [%d] and [%d]\n", idx, nidx);
  // printf("Delta for neighbor [%d] = %f\n", nidx, delta);

  // the force itself
  double r = (r1 * r2) / (r1 + r2);
  double gamma = 1; // attraction coeff
  double k = 2;     // repulsion coeff
  double f = k * delta - gamma * sqrt(r * delta);

  double module = f / center_distance;
  result->x += module * comp1;
  result->y += module * comp2;
  result->z += module * comp3;
  // printf("%f, %f, %f\n", module * comp1, module * comp2, module * comp3);
  // printf("Force between cell (%u) [%f, %f, %f] & cell (%u) [%f, %f, %f] = %f, %f, %f\n", idx, positions[3*idx + 0], positions[3*idx + 1], positions[3*idx + 2], nidx, positions[3*nidx + 0], positions[3*nidx + 1], positions[3*nidx + 2], module * comp1, module * comp2, module * comp3);
}

__device__ void default_force(double* positions,
                   double* diameters,
                   uint32_t idx, uint32_t start, uint16_t length,
                   uint32_t* successors,
                   double* squared_radius,
                   double3* result) {
  uint32_t nidx = start;
  for (uint16_t nb = 0; nb < length; nb++) {
    if (nidx != idx) {
      if (squared_euclidian_distance(positions, idx, nidx) < squared_radius[0]) {
        compute_force(positions, diameters, idx, nidx, result);
      }
    }
    // traverse linked-list
    nidx = successors[nidx];
  }
}

__global__ void collide(
       double* positions,
       double* diameters,
       double* tractor_force,
       double* adherence,
       uint32_t* box_id,
       double* mass,
       double* timestep,
       double* max_displacement,
       double* squared_radius,
       uint32_t* N,
       uint32_t* starts,
       uint16_t* lengths,
       uint32_t* successors,
       uint32_t* box_length,
       uint32_t* num_boxes_axis,
       int32_t* grid_dimensions,
       double* result) {
  uint32_t tidx = blockIdx.x * blockDim.x + threadIdx.x;
  if (tidx < N[0]) {
    result[3*tidx + 0] = timestep[0] * tractor_force[3*tidx + 0];
    result[3*tidx + 1] = timestep[0] * tractor_force[3*tidx + 1];
    result[3*tidx + 2] = timestep[0] * tractor_force[3*tidx + 2];
    // printf("cell_movement = (%f, %f, %f)\n", result[3*tidx + 0], result[3*tidx + 1], result[3*tidx + 2]);
    
    double3 collision_force = make_double3(0, 0, 0);

    // Moore neighborhood
    int3 box_coords = get_box_coordinates_2(box_id[tidx], num_boxes_axis);
    for (int z = -1; z <= 1; z++) {
      for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
          uint32_t bidx = get_box_id_2(box_coords + make_int3(x, y, z), num_boxes_axis);
          if (lengths[bidx] != 0) {
            default_force(positions, diameters, tidx, starts[bidx], lengths[bidx], successors, squared_radius, &collision_force);
          }
        }
      }
    }

    // Mass needs to non-zero!
    double mh = timestep[0] / mass[tidx];
    // printf("mh = %f\n", mh);

    if (norm(collision_force) > adherence[tidx]) {
      result[3*tidx + 0] += collision_force.x * mh;
      result[3*tidx + 1] += collision_force.y * mh;
      result[3*tidx + 2] += collision_force.z * mh;
      // printf("collision_force = (%f, %f, %f)\n", collision_force.x, collision_force.y, collision_force.z);
      // printf("cell_movement (1) = (%f, %f, %f)\n", result[3*tidx + 0], result[3*tidx + 1], result[3*tidx + 2]);

      if (norm(collision_force) * mh > max_displacement[0]) {
        result[3*tidx + 0] = max_displacement[0];
        result[3*tidx + 1] = max_displacement[0];
        result[3*tidx + 2] = max_displacement[0];
      }
      // printf("cell_movement (2) = (%f, %f, %f)\n", result[3*tidx + 0], result[3*tidx + 1], result[3*tidx + 2]);
    }
  }
}

bdm::DisplacementOpCudaKernel::DisplacementOpCudaKernel(uint32_t N, uint32_t num_boxes) {
  // printf("N = %u  |  num_boxes = %u\n", N, num_boxes);
  gpuErrchk(cudaMalloc(&d_positions, 3 * N * sizeof(double)));
  gpuErrchk(cudaMalloc(&d_diameters, N * sizeof(double)));
  gpuErrchk(cudaMalloc(&d_tractor_force, 3 * N * sizeof(double)));
  gpuErrchk(cudaMalloc(&d_adherence, N * sizeof(double)));
  gpuErrchk(cudaMalloc(&d_box_id, N * sizeof(uint32_t)));
  gpuErrchk(cudaMalloc(&d_mass, N * sizeof(double)));
  gpuErrchk(cudaMalloc(&d_timestep, sizeof(double)));
  gpuErrchk(cudaMalloc(&d_max_displacement, sizeof(double)));
  gpuErrchk(cudaMalloc(&d_squared_radius, sizeof(double)));
  gpuErrchk(cudaMalloc(&d_N, sizeof(uint32_t)));
  gpuErrchk(cudaMalloc(&d_starts, num_boxes * sizeof(uint32_t)));
  gpuErrchk(cudaMalloc(&d_lengths, num_boxes * sizeof(uint16_t)));
  gpuErrchk(cudaMalloc(&d_successors, N * sizeof(uint32_t)));
  gpuErrchk(cudaMalloc(&d_box_length, sizeof(uint32_t)));
  gpuErrchk(cudaMalloc(&d_num_boxes_axis, 3 * sizeof(uint32_t)));
  gpuErrchk(cudaMalloc(&d_grid_dimensions, 3 * sizeof(int32_t)));
  gpuErrchk(cudaMalloc(&d_cell_movements, 3 * N * sizeof(double)));
}

void bdm::DisplacementOpCudaKernel::displacement_op_cuda(double* positions, double* diameters, double* tractor_force, double* adherence, uint32_t* box_id, double* mass, double* timestep, double* max_displacement, double* squared_radius, uint32_t* N, uint32_t* starts, uint16_t* lengths, uint32_t* successors, uint32_t* box_length, uint32_t* num_boxes_axis, int32_t* grid_dimensions, double* cell_movements) {
  uint32_t num_boxes = num_boxes_axis[0] * num_boxes_axis[1] * num_boxes_axis[2];

  gpuErrchk(cudaMemcpy(d_positions, 		positions, 3 * N[0] * sizeof(double), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_diameters, 		diameters, N[0] * sizeof(double), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_tractor_force, 	tractor_force, 3 * N[0] * sizeof(double), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_adherence,     adherence, N[0] * sizeof(double), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_box_id, 		box_id, N[0] * sizeof(uint32_t), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_mass, 				mass, N[0] * sizeof(double), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_timestep, 			timestep, sizeof(double), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_max_displacement,  max_displacement, sizeof(double), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_squared_radius, 	squared_radius, sizeof(double), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_N, 				N, sizeof(uint32_t), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_starts, 			starts, num_boxes * sizeof(uint32_t), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_lengths, 			lengths, num_boxes * sizeof(uint16_t), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_successors, 		successors, N[0] * sizeof(uint32_t), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_box_length, 		box_length, sizeof(uint32_t), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_num_boxes_axis, 	num_boxes_axis, 3 * sizeof(uint32_t), cudaMemcpyHostToDevice));
  gpuErrchk(cudaMemcpy(d_grid_dimensions, 	grid_dimensions, 3 * sizeof(uint32_t), cudaMemcpyHostToDevice));

  int blockSize = 128;
  int minGridSize;
  int gridSize;

  // Get a near-optimal occupancy with the following thread organization
  cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, collide, 0, N[0]);
  gridSize = (N[0] + blockSize - 1) / blockSize;

  // printf("gridSize = %d  |  blockSize = %d\n", gridSize, blockSize);
  collide<<<gridSize, blockSize>>>(d_positions, d_diameters, d_tractor_force, d_adherence, d_box_id, d_mass, d_timestep, d_max_displacement, d_squared_radius, d_N, d_starts, d_lengths, d_successors, d_box_length, d_num_boxes_axis, d_grid_dimensions, d_cell_movements);

  // We need to wait for the kernel to finish before reading back the result
  cudaDeviceSynchronize();
  cudaMemcpy(cell_movements, d_cell_movements, 3 * N[0] * sizeof(double), cudaMemcpyDeviceToHost);
}

void bdm::DisplacementOpCudaKernel::resize_cell_buffers(uint32_t N) {
  cudaFree(d_positions);
  cudaFree(d_diameters);
  cudaFree(d_tractor_force);
  cudaFree(d_adherence);
  cudaFree(d_box_id);
  cudaFree(d_mass);
  cudaFree(d_successors);
  cudaFree(d_cell_movements);

  cudaMalloc(&d_positions, 3 * N * sizeof(double));
  cudaMalloc(&d_diameters, N * sizeof(double));
  cudaMalloc(&d_tractor_force, 3 * N * sizeof(double));
  cudaMalloc(&d_adherence, N * sizeof(double));
  cudaMalloc(&d_box_id, N * sizeof(uint32_t));
  cudaMalloc(&d_mass, N * sizeof(double));
  cudaMalloc(&d_successors, N * sizeof(uint32_t));
  cudaMalloc(&d_cell_movements, 3 * N * sizeof(double));
}

void bdm::DisplacementOpCudaKernel::resize_grid_buffers(uint32_t num_boxes) {
  cudaFree(d_starts);
  cudaFree(d_lengths);

  cudaMalloc(&d_starts, num_boxes * sizeof(uint32_t));
  cudaMalloc(&d_lengths, num_boxes * sizeof(uint16_t));
}

bdm::DisplacementOpCudaKernel::~DisplacementOpCudaKernel() {
  cudaFree(d_positions);
  cudaFree(d_diameters);
  cudaFree(d_tractor_force);
  cudaFree(d_adherence);
  cudaFree(d_box_id);
  cudaFree(d_mass);
  cudaFree(d_timestep);
  cudaFree(d_max_displacement);
  cudaFree(d_squared_radius);
  cudaFree(d_N);
  cudaFree(d_starts);
  cudaFree(d_lengths);
  cudaFree(d_successors);
  cudaFree(d_num_boxes_axis);
  cudaFree(d_grid_dimensions);
  cudaFree(d_cell_movements);
}
