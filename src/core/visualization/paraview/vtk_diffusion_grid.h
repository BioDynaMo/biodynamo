// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifndef CORE_VISUALIZATION_PARAVIEW_VTK_DIFFUSION_GRID_H_
#define CORE_VISUALIZATION_PARAVIEW_VTK_DIFFUSION_GRID_H_

// std
#include <algorithm>
#include <array>
#include <string>
#include <vector>
// Paraview
#include <vtkCPDataDescription.h>
#include <vtkImageData.h>
// BioDynaMo
#include "core/diffusion/diffusion_grid.h"

namespace bdm {

class ParaviewAdaptorTest_GenerateSimulationInfoJson_Test;

/// Adds additional data members to the `vtkImageData` required by
/// `ParaviewAdaptor` to visualize diffusion grid.
class VtkDiffusionGrid {
 public:
  VtkDiffusionGrid(const std::string& name,
                   vtkCPDataDescription* data_description);

  ~VtkDiffusionGrid();

  bool IsUsed() const;
  void Update(const DiffusionGrid* grid);
  void WriteToFile(uint64_t step) const;

 private:
  std::vector<vtkImageData*> data_;
  std::string name_;
  bool used_ = false;
  int concentration_array_idx_ = -1;
  int gradient_array_idx_ = -1;

  // The following data members are needed to partition a diffusion grid into
  // multiple
  // vtkImageData objects for parallel processing.
  // ParaView calls the partition pieces.

  uint64_t num_pieces_;
  uint64_t piece_boxes_z_;
  uint64_t piece_boxes_z_last_;
  std::array<int, 6> whole_extent_;
  std::vector<std::array<int, 6>> piece_extents_;

  /// Calculate in how many pieces the vtkImageData should be split
  /// and how thick the z-layer slices are.
  void Dissect(uint64_t boxes_z, uint64_t num_pieces_target);

  void CalcPieceExtents(const std::array<size_t, 3>& num_boxes);

  friend class ParaviewAdaptorTest_GenerateSimulationInfoJson_Test;
};

}  // namespace bdm

#endif  // CORE_VISUALIZATION_PARAVIEW_VTK_DIFFUSION_GRID_H_
