#ifndef DISPLACEMENT_OP_OPENCL_H_
#define DISPLACEMENT_OP_OPENCL_H_

#ifdef USE_OPENCL
#define __CL_ENABLE_EXCEPTIONS
#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#endif

#include "gpu/gpu_helper.h"
#include "grid.h"
#include "resource_manager.h"

namespace bdm {

using std::array;

/// Defines the 3D physical interactions between physical objects
template <typename TGrid = Grid<>,
          typename TResourceManager = ResourceManager<>>
class DisplacementOpOpenCL {
 public:
  DisplacementOpOpenCL() {}
  ~DisplacementOpOpenCL() {}

  template <typename TContainer>
  void operator()(TContainer* cells, uint16_t type_idx) const {
#ifdef USE_OPENCL
    auto& grid = TGrid::GetInstance();
    auto rm = TResourceManager::Get();
    auto context = rm->GetOpenCLContext();
    auto queue = rm->GetOpenCLCommandQueue();
    auto programs = rm->GetOpenCLProgramList();

    std::vector<cl_double> mass(cells->size());
    std::vector<array<cl_double, 3>> cell_movements(cells->size());
    std::vector<cl_uint> gpu_starts;
    std::vector<cl_ushort> gpu_lengths;
    std::vector<cl_uint> successors(cells->size());
    cl_uint box_length;
    std::array<cl_uint, 3> num_boxes_axis;
    std::array<cl_int, 3> grid_dimensions;
    cl_double squared_radius =
        grid.GetLargestObjectSize() * grid.GetLargestObjectSize();

    // We need to create a mass vector, because it is not stored by default in
    // a cell container
    cells->FillMassVector(&mass);
    grid.GetSuccessors(&successors);
    grid.GetBoxInfo(&gpu_starts, &gpu_lengths);
    grid.GetGridInfo(&box_length, num_boxes_axis, grid_dimensions);

    // Allocate GPU buffers
    cl::Buffer positions_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                             cells->size() * 3 * sizeof(cl_double),
                             cells->GetPositionPtr());
    cl::Buffer diameters_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                             cells->size() * sizeof(cl_double),
                             cells->GetDiameterPtr());
    cl::Buffer tractor_force_arg(
        *context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        cells->size() * 3 * sizeof(cl_double), cells->GetTractorForcePtr());
    cl::Buffer adherence_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                             cells->size() * sizeof(cl_double),
                             cells->GetAdherencePtr());
    cl::Buffer box_id_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                          cells->size() * sizeof(cl_uint),
                          cells->GetBoxIdPtr());
    cl::Buffer mass_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                        cells->size() * sizeof(cl_double), mass.data());
    cl::Buffer cell_movements_arg(
        *context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
        cells->size() * 3 * sizeof(cl_double), cell_movements.data()->data());
    cl::Buffer starts_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                          gpu_starts.size() * sizeof(cl_uint),
                          gpu_starts.data());
    cl::Buffer lengths_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                           gpu_lengths.size() * sizeof(cl_short),
                           gpu_lengths.data());
    cl::Buffer successors_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                              successors.size() * sizeof(cl_uint),
                              successors.data());
    cl::Buffer nba_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                       3 * sizeof(cl_uint), num_boxes_axis.data());
    cl::Buffer gd_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      3 * sizeof(cl_int), grid_dimensions.data());

    // Create the kernel object from our program
    // TODO(ahmad): generalize the program selection, in case we have more than
    // one. We can maintain an unordered map of programs maybe
    cl::Kernel collide((*programs)[0], "collide");

    // Set kernel parameters
    collide.setArg(0, positions_arg);
    collide.setArg(1, diameters_arg);
    collide.setArg(2, tractor_force_arg);
    collide.setArg(3, adherence_arg);
    collide.setArg(4, box_id_arg);
    collide.setArg(5, mass_arg);
    collide.setArg(6, Param::simulation_time_step_);
    collide.setArg(7, Param::simulation_max_displacement_);
    collide.setArg(8, squared_radius);

    collide.setArg(9, static_cast<cl_int>(cells->size()));
    collide.setArg(10, starts_arg);
    collide.setArg(11, lengths_arg);
    collide.setArg(12, successors_arg);
    collide.setArg(13, box_length);
    collide.setArg(14, nba_arg);
    collide.setArg(15, gd_arg);
    collide.setArg(16, cell_movements_arg);

    // The amount of threads for each work group (analogous to CUDA thread
    // block)
    int block_size = 256;

    auto num_objects = cells->size();

    try {
      // The global size determines the total number of threads that will be
      // spawned on the GPU, in groups of local_size
      cl::NDRange global_size =
          cl::NDRange(num_objects + (block_size - (num_objects % block_size)));
      cl::NDRange local_size = cl::NDRange(block_size);
      queue->enqueueNDRangeKernel(collide, cl::NullRange, global_size,
                                  local_size);
    } catch (const cl::Error& err) {
      Log::Error("DisplacementOpOpenCL", err.what(), "(", err.err(), ") = ",
                 GetErrorString(err.err()));
      throw;
    }

    try {
      queue->enqueueReadBuffer(cell_movements_arg, CL_TRUE, 0,
                               cells->size() * 3 * sizeof(cl_double),
                               cell_movements.data()->data());
    } catch (const cl::Error& err) {
      Log::Error("DisplacementOpOpenCL", err.what(), "(", err.err(), ") = ",
                 GetErrorString(err.err()));
      throw;
    }

// set new positions after all updates have been calculated
// otherwise some cells would see neighbors with already updated positions
// which would lead to inconsistencies
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      auto&& cell = (*cells)[i];
      cell.UpdateMassLocation(cell_movements[i]);
      if (Param::bound_space_) {
        ApplyBoundingBox(&cell, Param::min_bound_, Param::max_bound_);
      }
      cell.SetPosition(cell.GetMassLocation());

      // Reset biological movement to 0.
      cell.SetTractorForce({0, 0, 0});
    }
#endif
  }
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_OPENCL_H_