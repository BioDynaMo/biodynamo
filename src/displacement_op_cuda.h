#ifndef DISPLACEMENT_OP_CUDA_H_
#define DISPLACEMENT_OP_CUDA_H_

#include <vector>

#include "displacement_op.h"
#include "gpu/displacement_op_cuda_kernel.h"
#include "log.h"
#include "resource_manager.h"

namespace bdm {

using std::array;

/// Defines the 3D physical interactions between physical objects
template <typename TGrid = Grid<>>
class DisplacementOpCuda {
 public:
  DisplacementOpCuda() {}
  ~DisplacementOpCuda() {}

  template <typename TContainer>
  void operator()(TContainer* cells, uint16_t type_idx) {
    auto& grid = TGrid::GetInstance();

    std::vector<std::array<double, 3>> cell_movements(cells->size());
    std::vector<double> mass(cells->size());
    std::vector<uint32_t> starts;
    std::vector<uint16_t> lengths;
    std::vector<uint32_t> successors(cells->size());
    uint32_t box_length;
    uint32_t num_objects = cells->size();
    std::array<uint32_t, 3> num_boxes_axis;
    std::array<int32_t, 3> grid_dimensions;
    double squared_radius =
        grid.GetLargestObjectSize() * grid.GetLargestObjectSize();

    // We need to create a mass vector, because it is not stored by default in
    // a cell container
    cells->FillMassVector(&mass);
    grid.GetSuccessors(&successors);
    grid.GetBoxInfo(&starts, &lengths);
    grid.GetGridInfo(&box_length, &num_boxes_axis, &grid_dimensions);

    // If this is the first time we perform physics on GPU using CUDA
    if (cdo_ == nullptr) {
      // Allocate 25% more memory so we don't need to reallocate GPU memory
      // for every (small) change
      uint32_t new_num_objects = static_cast<uint32_t>(1.25 * num_objects);
      uint32_t new_num_boxes = static_cast<uint32_t>(1.25 * starts.size());

      // Store these extende buffer sizes for future reference
      num_objects_ = new_num_objects;
      num_boxes_ = new_num_boxes;

      // Allocate required GPU memory
      cdo_ = new DisplacementOpCudaKernel(new_num_objects, new_num_boxes);
    } else {
      // If the number of simulation objects increased
      if (num_objects >= num_objects_) {
        Log::Info("DisplacementOpCuda",
                  "\nThe number of cells increased signficantly (from ",
                  num_objects_, " to ", num_objects,
                  "), so we allocate bigger GPU buffers\n");
        uint32_t new_num_objects = static_cast<uint32_t>(1.25 * num_objects);
        num_objects_ = new_num_objects;
        cdo_->ResizeCellBuffers(new_num_objects);
      }

      // If the neighbor grid size increased
      if (starts.size() >= num_boxes_) {
        Log::Info("DisplacementOpCuda",
                  "\nThe number of boxes increased signficantly (from ",
                  num_boxes_, " to ", "), so we allocate bigger GPU buffers\n");
        uint32_t new_num_boxes = static_cast<uint32_t>(1.25 * starts.size());
        num_boxes_ = new_num_boxes;
        cdo_->ResizeGridBuffers(new_num_boxes);
      }
    }

    cdo_->LaunchDisplacementKernel(
        cells->GetPositionPtr(), cells->GetDiameterPtr(),
        cells->GetTractorForcePtr(), cells->GetAdherencePtr(),
        cells->GetBoxIdPtr(), mass.data(), &(Param::simulation_time_step_),
        &(Param::simulation_max_displacement_), &squared_radius, &num_objects,
        starts.data(), lengths.data(), successors.data(), &box_length,
        num_boxes_axis.data(), grid_dimensions.data(),
        cell_movements.data()->data());

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
  }

 private:
  DisplacementOpCudaKernel* cdo_ = nullptr;
  uint32_t num_boxes_ = 0;
  uint32_t num_objects_ = 0;
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_CUDA_H_
