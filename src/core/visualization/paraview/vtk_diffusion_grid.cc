// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#include "core/visualization/paraview/vtk_diffusion_grid.h"
// ParaView
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkDoubleArray.h>
#include <vtkExtentTranslator.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
// BioDynaMo
#include "core/param/param.h"
#include "core/simulation.h"
#include "core/util/thread_info.h"
#include "core/visualization/paraview/parallel_vti_writer.h"

namespace bdm {

// -----------------------------------------------------------------------------
VtkDiffusionGrid::VtkDiffusionGrid(const std::string& name,
                                   vtkCPDataDescription* data_description) {
  auto* param = Simulation::GetActive()->GetParam();
  if (param->export_visualization) {
    auto* tinfo = ThreadInfo::GetInstance();
    data_.resize(tinfo->GetMaxThreads());
  } else {
    data_.resize(1);
  }

#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < data_.size(); ++i) {
    data_[i] = vtkImageData::New();
  }
  name_ = name;

  // get visualization config
  const Param::VisualizeDiffusion* vd = nullptr;
  for (auto& entry : param->visualize_diffusion) {
    if (entry.name == name) {
      vd = &entry;
      break;
    }
  }

  for (uint64_t i = 0; i < data_.size(); ++i) {
    // Add attribute data
    if (vd->concentration) {
      vtkNew<vtkDoubleArray> concentration;
      concentration->SetName("Substance Concentration");
      concentration_array_idx_ =
          data_[i]->GetPointData()->AddArray(concentration.GetPointer());
    }
    if (vd->gradient) {
      vtkNew<vtkDoubleArray> gradient;
      gradient->SetName("Diffusion Gradient");
      gradient->SetNumberOfComponents(3);
      gradient_array_idx_ =
          data_[i]->GetPointData()->AddArray(gradient.GetPointer());
    }
  }

  if (!param->export_visualization) {
    data_description->AddInput(name.c_str());
    data_description->GetInputDescriptionByName(name.c_str())
        ->SetGrid(data_[0]);
  }
}

// -----------------------------------------------------------------------------
VtkDiffusionGrid::~VtkDiffusionGrid() {
  name_ = "";
  for (auto& el : data_) {
    el->Delete();
  }
  data_.clear();
}

// -----------------------------------------------------------------------------
bool VtkDiffusionGrid::IsUsed() const { return used_; }

// -----------------------------------------------------------------------------
void VtkDiffusionGrid::Update(const DiffusionGrid* grid) {
  used_ = true;

  auto num_boxes = grid->GetNumBoxesArray();
  auto grid_dimensions = grid->GetDimensions();
  auto box_length = grid->GetBoxLength();
  auto total_boxes = grid->GetNumBoxes();

  auto* tinfo = ThreadInfo::GetInstance();
  whole_extent_ = {{0, std::max(static_cast<int>(num_boxes[0]) - 1, 0), 0,
                    std::max(static_cast<int>(num_boxes[1]) - 1, 0), 0,
                    std::max(static_cast<int>(num_boxes[2]) - 1, 0)}};
  Dissect(num_boxes[2], tinfo->GetMaxThreads());
  CalcPieceExtents(num_boxes);
  uint64_t xy_num_boxes = num_boxes[0] * num_boxes[1];
  double origin_x = grid_dimensions[0];
  double origin_y = grid_dimensions[2];
  double origin_z = grid_dimensions[4];

  // do not partition data for insitu visualization
  if (data_.size() == 1) {
    data_[0]->SetOrigin(origin_x, origin_y, origin_z);
    data_[0]->SetDimensions(num_boxes[0], num_boxes[1], num_boxes[2]);
    data_[0]->SetSpacing(box_length, box_length, box_length);

    if (concentration_array_idx_ != -1) {
      auto* co_ptr = const_cast<double*>(grid->GetAllConcentrations());
      auto elements = static_cast<vtkIdType>(total_boxes);
      auto* array = static_cast<vtkDoubleArray*>(
          data_[0]->GetPointData()->GetArray(concentration_array_idx_));
      array->SetArray(co_ptr, elements, 1);
    }
    if (gradient_array_idx_ != -1) {
      auto gr_ptr = const_cast<double*>(grid->GetAllGradients());
      auto elements = static_cast<vtkIdType>(total_boxes * 3);
      auto* array = static_cast<vtkDoubleArray*>(
          data_[0]->GetPointData()->GetArray(gradient_array_idx_));
      array->SetArray(gr_ptr, elements, 1);
    }
    return;
  }

#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < num_pieces_; ++i) {
    uint64_t piece_elements;
    auto* e = piece_extents_[i].data();
    if (i < num_pieces_ - 1) {
      piece_elements = piece_boxes_z_ * xy_num_boxes;
      data_[i]->SetDimensions(num_boxes[0], num_boxes[1], piece_boxes_z_);
      data_[i]->SetExtent(e[0], e[1], e[2], e[3], e[4],
                          e[4] + piece_boxes_z_ - 1);
    } else {
      piece_elements = piece_boxes_z_last_ * xy_num_boxes;
      data_[i]->SetDimensions(num_boxes[0], num_boxes[1], piece_boxes_z_last_);
      data_[i]->SetExtent(e[0], e[1], e[2], e[3], e[4],
                          e[4] + piece_boxes_z_last_ - 1);
    }
    double piece_origin_z = origin_z + box_length * piece_boxes_z_ * i;
    data_[i]->SetOrigin(origin_x, origin_y, piece_origin_z);
    data_[i]->SetSpacing(box_length, box_length, box_length);

    if (concentration_array_idx_ != -1) {
      auto* co_ptr = const_cast<double*>(grid->GetAllConcentrations());
      auto elements = static_cast<vtkIdType>(piece_elements);
      auto* array = static_cast<vtkDoubleArray*>(
          data_[i]->GetPointData()->GetArray(concentration_array_idx_));
      if (i < num_pieces_ - 1) {
        array->SetArray(co_ptr + (elements * i), elements, 1);
      } else {
        array->SetArray(co_ptr + total_boxes - elements, elements, 1);
      }
    }
    if (gradient_array_idx_ != -1) {
      auto gr_ptr = const_cast<double*>(grid->GetAllGradients());
      auto elements = static_cast<vtkIdType>(piece_elements * 3);
      auto* array = static_cast<vtkDoubleArray*>(
          data_[i]->GetPointData()->GetArray(gradient_array_idx_));
      if (i < num_pieces_ - 1) {
        array->SetArray(gr_ptr + (elements * i), elements, 1);
      } else {
        array->SetArray(gr_ptr + total_boxes - elements, elements, 1);
      }
    }
  }
}

// -----------------------------------------------------------------------------
void VtkDiffusionGrid::WriteToFile(uint64_t step) const {
  auto* sim = Simulation::GetActive();
  auto filename_prefix = Concat(name_, "-", step);

  ParallelVtiWriter writer;
  writer(sim->GetOutputDir(), filename_prefix, data_, num_pieces_,
         whole_extent_, piece_extents_);
}

// -----------------------------------------------------------------------------
void VtkDiffusionGrid::Dissect(uint64_t boxes_z, uint64_t num_pieces_target) {
  if (num_pieces_target == 1) {
    piece_boxes_z_last_ = boxes_z;
    piece_boxes_z_ = 1;
    num_pieces_ = 1;
  } else if (boxes_z <= num_pieces_target) {
    piece_boxes_z_last_ = 2;
    piece_boxes_z_ = 1;
    num_pieces_ = std::max<unsigned long long>(1ULL, boxes_z - 1);
  } else {
    auto boxes_per_piece = static_cast<double>(boxes_z) / num_pieces_target;
    piece_boxes_z_ = static_cast<uint64_t>(std::ceil(boxes_per_piece));
    num_pieces_ = boxes_z / piece_boxes_z_;
    piece_boxes_z_last_ = boxes_z - (num_pieces_ - 1) * piece_boxes_z_;
  }
  assert(num_pieces_ > 0);
  assert(piece_boxes_z_last_ >= 1);
  assert((num_pieces_ - 1) * piece_boxes_z_ + piece_boxes_z_last_ == boxes_z);
}

// -----------------------------------------------------------------------------
void VtkDiffusionGrid::CalcPieceExtents(
    const std::array<size_t, 3>& num_boxes) {
  piece_extents_.resize(num_pieces_);
  if (num_pieces_ == 1) {
    piece_extents_[0] = whole_extent_;
    return;
  }
  int c = piece_boxes_z_;
  piece_extents_[0] = {{0, static_cast<int>(num_boxes[0]) - 1, 0,
                        static_cast<int>(num_boxes[1]) - 1, 0, c}};
  for (uint64_t i = 1; i < num_pieces_ - 1; ++i) {
    piece_extents_[i] = {{0, static_cast<int>(num_boxes[0]) - 1, 0,
                          static_cast<int>(num_boxes[1]) - 1, c,
                          c + static_cast<int>(piece_boxes_z_)}};
    c += piece_boxes_z_;
  }
  piece_extents_.back() = {{0, static_cast<int>(num_boxes[0]) - 1, 0,
                            static_cast<int>(num_boxes[1]) - 1, c,
                            static_cast<int>(num_boxes[2]) - 1}};
}

}  // namespace bdm
