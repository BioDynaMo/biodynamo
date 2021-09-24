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

#ifndef CORE_DIFFUSION_DIFFUSION_GRID_H_
#define CORE_DIFFUSION_DIFFUSION_GRID_H_

#include <array>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "core/container/math_array.h"
#include "core/container/parallel_resize_vector.h"
#include "core/util/log.h"
#include "core/util/root.h"
#include "core/util/spinlock.h"

namespace bdm {

class DiffusionGrid {
 public:
  DiffusionGrid() {}
  explicit DiffusionGrid(TRootIOCtor* p) {}
  DiffusionGrid(int substance_id, std::string substance_name, double dc,
                double mu, int resolution = 11)
      : substance_(substance_id),
        substance_name_(std::move(substance_name)),
        dc_({{1 - dc, dc / 6, dc / 6, dc / 6, dc / 6, dc / 6, dc / 6}}),
        mu_(mu),
        resolution_(resolution) {}

  virtual ~DiffusionGrid() {}

  virtual void Initialize();

  /// Updates the grid dimensions, based on the given threshold values. The
  /// diffusion grid dimensions need always be larger than the neighbor grid
  /// dimensions, so that each simulation object can obtain its local
  /// concentration / gradient
  virtual void Update();

  void Diffuse(double dt);

  virtual void DiffuseWithClosedEdge(double dt) = 0;
  virtual void DiffuseWithOpenEdge(double dt) = 0;

  /// Calculates the gradient for each box in the diffusion grid.
  /// The gradient is calculated in each direction (x, y, z) as following:
  ///
  /// c(x + box_length_) - c(x - box_length) / (2 * box_length_),
  ///
  /// where c(x) implies the concentration at position x
  ///
  /// At the edges the gradient is the same as the box next to it
  void CalculateGradient();

  /// Initialize the diffusion grid according to the initialization functions
  void RunInitializers();

  /// Increase the concentration at specified box with specified amount
  void ChangeConcentrationBy(const Double3& position, double amount);
  void ChangeConcentrationBy(size_t idx, double amount);

  /// Get the concentration at specified position
  double GetConcentration(const Double3& position) const;

  /// Get the (normalized) gradient at specified position
  // TODO: virtual because of test
  virtual void GetGradient(const Double3& position, Double3* gradient) const;

  std::array<uint32_t, 3> GetBoxCoordinates(const Double3& position) const;

  size_t GetBoxIndex(const std::array<uint32_t, 3>& box_coord) const;

  /// Calculates the box index of the substance at specified position
  size_t GetBoxIndex(const Double3& position) const;

  void SetDecayConstant(double mu) { mu_ = mu; }

  void SetConcentrationThreshold(double t) { concentration_threshold_ = t; }

  /// Return the last timestep `dt` that was used to run `Diffuse(dt)`
  double GetLastTimestep() { return last_dt_; }

  double GetConcentrationThreshold() const { return concentration_threshold_; }

  const double* GetAllConcentrations() const { return c1_.data(); }

  const double* GetAllGradients() const { return gradients_.data()->data(); }

  std::array<size_t, 3> GetNumBoxesArray() const {
    std::array<size_t, 3> ret;
    ret[0] = resolution_;
    ret[1] = resolution_;
    ret[2] = resolution_;
    return ret;
  }

  size_t GetNumBoxes() const { return total_num_boxes_; }

  double GetBoxLength() const { return box_length_; }

  int GetSubstanceId() const { return substance_; }

  const std::string& GetSubstanceName() const { return substance_name_; }

  double GetDecayConstant() const { return mu_; }

  const int32_t* GetDimensionsPtr() const { return grid_dimensions_.data(); }

  std::array<int32_t, 6> GetDimensions() const {
    std::array<int32_t, 6> ret;
    ret[0] = grid_dimensions_[0];
    ret[1] = grid_dimensions_[1];
    ret[2] = grid_dimensions_[0];
    ret[3] = grid_dimensions_[1];
    ret[4] = grid_dimensions_[0];
    ret[5] = grid_dimensions_[1];
    return ret;
  }

  std::array<int32_t, 3> GetGridSize() const {
    std::array<int32_t, 3> ret;
    ret[0] = grid_dimensions_[1] - grid_dimensions_[0];
    ret[1] = grid_dimensions_[1] - grid_dimensions_[0];
    ret[2] = grid_dimensions_[1] - grid_dimensions_[0];
    return ret;
  }

  const std::array<double, 7>& GetDiffusionCoefficients() const { return dc_; }

  int GetResolution() const { return resolution_; }

  double GetBoxVolume() const { return box_volume_; }

  template <typename F>
  void AddInitializer(F function) {
    initializers_.push_back(function);
  }

  // retrun true if substance concentration and gradient don't evolve over time
  bool IsFixedSubstance() {
    return (mu_ == 0 && dc_[1] == 0 && dc_[2] == 0 && dc_[3] == 0 &&
            dc_[4] == 0 && dc_[5] == 0 && dc_[6] == 0);
  }

 private:
  friend class RungeKuttaGrid;
  friend class EulerGrid;
  friend class StencilGrid;
  friend class TestGrid;  // class used for testing (e.g. initialization)

  void ParametersCheck(double dt);

  /// Copies the concentration and gradients values to the new
  /// (larger) grid. In the 2D case it looks like the following:
  ///
  ///                             [0 0  0  0]
  ///               [v1 v2]  -->  [0 v1 v2 0]
  ///               [v3 v4]  -->  [0 v3 v4 0]
  ///                             [0 0  0  0]
  ///
  /// The dimensions are doubled in this case from 2x2 to 4x4
  /// If the dimensions would be increased from 2x2 to 3x3, it will still
  /// be increased to 4x4 in order for GetBoxIndex to function correctly
  ///
  void CopyOldData(const ParallelResizeVector<double>& old_c1,
                   const ParallelResizeVector<Double3>& old_gradients,
                   size_t old_resolution);

  /// The id of the substance of this grid
  int substance_ = 0;
  /// The name of the substance of this grid
  std::string substance_name_ = "";
  /// The side length of each box
  double box_length_ = 0;
  /// the volume of each box
  double box_volume_ = 0;
  /// Lock for each voxel used to prevent race conditions between
  /// multiple threads
  mutable ParallelResizeVector<Spinlock> locks_ = {};
  /// The array of concentration values
  ParallelResizeVector<double> c1_ = {};
  /// An extra concentration data buffer for faster value updating
  ParallelResizeVector<double> c2_ = {};
  /// The array of gradients (x, y, z)
  ParallelResizeVector<Double3> gradients_ = {};
  /// The maximum concentration value that a box can have
  double concentration_threshold_ = 1e15;
  /// The diffusion coefficients [cc, cw, ce, cs, cn, cb, ct]
  std::array<double, 7> dc_ = {{0}};
  /// The decay constant
  double mu_ = 0;
  /// The grid dimensions of the diffusion grid (cubic shaped)
  std::array<int32_t, 2> grid_dimensions_ = {{0}};
  /// The number of boxes at each axis [x, y, z] (same along each axis)
  uint64_t num_boxes_axis_ = 0;
  /// The total number of boxes in the diffusion grid
  size_t total_num_boxes_ = 0;
  /// The resolution of the diffusion grid (i.e. number of boxes along each
  /// axis)
  size_t resolution_ = 0;
  /// The last timestep `dt` used for the diffusion grid update `Diffuse(dt)`
  double last_dt_ = 0.0;
  /// If false, grid dimensions are even; if true, they are odd
  bool parity_ = false;
  /// A list of functions that initialize this diffusion grid
  /// ROOT currently doesn't support IO of std::function
  std::vector<std::function<double(double, double, double)>> initializers_ =
      {};  //!
  // Turn to true after gradient initialization
  bool init_gradient_ = false;

  BDM_CLASS_DEF(DiffusionGrid, 1);
};

}  // namespace bdm

#endif  // CORE_DIFFUSION_DIFFUSION_GRID_H_
