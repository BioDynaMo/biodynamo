// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "core/grid.h"

#include <assert.h>
#include <omp.h>

#include <algorithm>
#include <limits>
#ifdef LINUX
#include <parallel/algorithm>
#endif  // LINUX
#include <utility>
#include <vector>

#include <morton/morton.h>

#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/util/log.h"

namespace bdm {

Grid::Box::Box() : start_(SoHandle()), length_(0) {}

Grid::Box::Box(const Box& other) : Box() {}

Grid::Box& Grid::Box::operator=(const Box& other) {
  start_ = other.start_.load(std::memory_order_relaxed);
  length_ = other.length_.load(std::memory_order_relaxed);
  return *this;
}

bool Grid::Box::IsEmpty() const { return length_ == 0; }

void Grid::Box::AddObject(SoHandle so, SimObjectVector<SoHandle>* successors) {
  length_++;
  auto old_start = std::atomic_exchange(&start_, so);
  if (old_start != SoHandle()) {
    (*successors)[so] = old_start;
  }
}

Grid::Box::Iterator::Iterator(Grid* grid, const Box* box)
    : grid_(grid), current_value_(box->start_), countdown_(box->length_) {}

bool Grid::Box::Iterator::IsAtEnd() { return countdown_ <= 0; }

Grid::Box::Iterator& Grid::Box::Iterator::operator++() {
  countdown_--;
  if (countdown_ > 0) {
    current_value_ = grid_->successors_[current_value_];
  }
  return *this;
}

SoHandle Grid::Box::Iterator::operator*() const { return current_value_; }

Grid::Box::Iterator Grid::Box::begin() const {  // NOLINT
  return Iterator(Simulation::GetActive()->GetGrid(), this);
}

Grid::NeighborIterator::NeighborIterator(
    const FixedSizeVector<const Box*, 27>& neighbor_boxes)
    : neighbor_boxes_(neighbor_boxes),
      // start iterator from box 0
      box_iterator_(neighbor_boxes_[0]->begin()) {
  // if first box is empty
  if (neighbor_boxes_[0]->IsEmpty()) {
    ForwardToNonEmptyBox();
  }
}

bool Grid::NeighborIterator::IsAtEnd() const { return is_end_; }

SoHandle Grid::NeighborIterator::operator*() const { return *box_iterator_; }

/// Version where empty neighbor boxes are allowed
Grid::NeighborIterator& Grid::NeighborIterator::operator++() {
  ++box_iterator_;
  // if iterator of current box has come to an end, continue with next box
  if (box_iterator_.IsAtEnd()) {
    return ForwardToNonEmptyBox();
  }
  return *this;
}

Grid::NeighborIterator& Grid::NeighborIterator::ForwardToNonEmptyBox() {
  // increment box id until non empty box has been found
  while (++box_idx_ < neighbor_boxes_.size()) {
    // box is empty or uninitialized (padding box) -> continue
    if (neighbor_boxes_[box_idx_]->IsEmpty()) {
      continue;
    }
    // a non-empty box has been found
    box_iterator_ = neighbor_boxes_[box_idx_]->begin();
    return *this;
  }
  // all remaining boxes have been empty; reached end
  is_end_ = true;
  return *this;
}

Grid::Grid() {}

void Grid::Initialize(Adjacency adjacency) {
  adjacency_ = adjacency;

  UpdateGrid();
  initialized_ = true;
}

Grid::~Grid() {}

void Grid::ClearGrid() {
  boxes_.clear();
  box_length_ = 1;
  largest_object_size_ = 0;
  num_boxes_axis_ = {{0}};
  num_boxes_xy_ = 0;
  int32_t inf = std::numeric_limits<int32_t>::max();
  grid_dimensions_ = {inf, -inf, inf, -inf, inf, -inf};
  threshold_dimensions_ = {inf, -inf};
  successors_.clear();
  has_grown_ = false;
}

void Grid::UpdateGrid() {
  auto* rm = Simulation::GetActive()->GetResourceManager();

  if (rm->GetNumSimObjects() != 0) {
    ClearGrid();

    auto inf = Math::kInfinity;
    std::array<double, 6> tmp_dim = {{inf, -inf, inf, -inf, inf, -inf}};
    CalculateGridDimensions(&tmp_dim);
    RoundOffGridDimensions(tmp_dim);

    auto los = ceil(largest_object_size_);
    assert(los > 0 &&
           "The largest object size was found to be 0. Please check if your "
           "cells are correctly initialized.");
    box_length_ = los;

    for (int i = 0; i < 3; i++) {
      int dimension_length =
          grid_dimensions_[2 * i + 1] - grid_dimensions_[2 * i];
      int r = dimension_length % box_length_;
      // If the grid is not perfectly divisible along each dimension by the
      // resolution, extend the grid so that it is
      if (r != 0) {
        // std::abs for the case that box_length_ > dimension_length
        grid_dimensions_[2 * i + 1] += (box_length_ - r);
      } else {
        // Else extend the grid dimension with one row, because the outmost
        // object lies exactly on the border
        grid_dimensions_[2 * i + 1] += box_length_;
      }
    }

    // Pad the grid to avoid out of bounds check when search neighbors
    for (int i = 0; i < 3; i++) {
      grid_dimensions_[2 * i] -= box_length_;
      grid_dimensions_[2 * i + 1] += box_length_;
    }

    // Calculate how many boxes fit along each dimension
    for (int i = 0; i < 3; i++) {
      int dimension_length =
          grid_dimensions_[2 * i + 1] - grid_dimensions_[2 * i];
      assert((dimension_length % box_length_ == 0) &&
             "The grid dimensions are not a multiple of its box length");
      num_boxes_axis_[i] = dimension_length / box_length_;
    }

    num_boxes_xy_ = num_boxes_axis_[0] * num_boxes_axis_[1];
    auto total_num_boxes = num_boxes_xy_ * num_boxes_axis_[2];

    CheckGridGrowth();

    if (boxes_.size() != total_num_boxes) {
      boxes_.resize(total_num_boxes, Box());
    }

    successors_.reserve();

    // Assign simulation objects to boxes
    rm->ApplyOnAllElementsParallelDynamic(
        1000, [this](SimObject* sim_object, SoHandle soh) {
          const auto& position = sim_object->GetPosition();
          auto idx = this->GetBoxIndex(position);
          auto box = this->GetBoxPointer(idx);
          box->AddObject(soh, &successors_);
          sim_object->SetBoxIdx(idx);
        });
    auto* param = Simulation::GetActive()->GetParam();
    if (param->bound_space_) {
      int min = param->min_bound_;
      int max = param->max_bound_;
      threshold_dimensions_ = {min, max};
    }

    if (nb_mutex_builder_ != nullptr) {
      nb_mutex_builder_->Update();
    }
  } else {
    // There are no sim objects in this simulation
    auto* param = Simulation::GetActive()->GetParam();

    bool uninitialized = boxes_.size() == 0;
    if (uninitialized && param->bound_space_) {
      // Simulation has never had any simulation objects
      // Initialize grid dimensions with `Param::min_bound_` and
      // `Param::max_bound_`
      // This is required for the DiffusionGrid
      int min = param->min_bound_;
      int max = param->max_bound_;
      grid_dimensions_ = {min, max, min, max, min, max};
      threshold_dimensions_ = {min, max};
      has_grown_ = true;
    } else if (!uninitialized) {
      // all simulation objects have been removed in the last iteration
      // grid state remains the same, but we have to set has_grown_ to false
      // otherwise the DiffusionGrid will attempt to resize
      has_grown_ = false;
    } else {
      Log::Fatal(
          "Grid",
          "You tried to initialize an empty simulation without bound space. "
          "Therefore we cannot determine the size of the simulation space. "
          "Please add simulation objects, or set Param::bound_space_, "
          "Param::min_bound_, and Param::max_bound_.");
    }
  }
}

double Grid::SquaredEuclideanDistance(const std::array<double, 3>& pos1,
                                      const std::array<double, 3>& pos2) const {
  const double dx = pos2[0] - pos1[0];
  const double dy = pos2[1] - pos1[1];
  const double dz = pos2[2] - pos1[2];
  return (dx * dx + dy * dy + dz * dz);
}

bool Grid::WithinSquaredEuclideanDistance(
    double squared_radius, const std::array<double, 3>& pos1,
    const std::array<double, 3>& pos2) const {
  const double dx = pos2[0] - pos1[0];
  const double dx2 = dx * dx;
  if (dx2 > squared_radius) {
    return false;
  }

  const double dy = pos2[1] - pos1[1];
  const double dy2_plus_dx2 = dy * dy + dx2;
  if (dy2_plus_dx2 > squared_radius) {
    return false;
  }

  const double dz = pos2[2] - pos1[2];
  const double distance = dz * dz + dy2_plus_dx2;
  return distance < squared_radius;
}

void Grid::UpdateBoxZOrder() {
  // iterate boxes in Z-order / morton order
  // TODO(lukas) this is a very quick attempt to test an idea
  // improve performance of this brute force solution
  zorder_sorted_boxes_.resize(boxes_.size());
#pragma omp parallel for collapse(3)
  for (uint32_t x = 0; x < num_boxes_axis_[0]; x++) {
    for (uint32_t y = 0; y < num_boxes_axis_[1]; y++) {
      for (uint32_t z = 0; z < num_boxes_axis_[2]; z++) {
        auto box_idx = GetBoxIndex(std::array<uint32_t, 3>{x, y, z});
        auto morton = libmorton::morton3D_64_encode(x, y, z);
        zorder_sorted_boxes_[box_idx] =
            std::pair<uint32_t, const Box*>{morton, &boxes_[box_idx]};
      }
    }
  }
#ifdef LINUX
  __gnu_parallel::sort(
      zorder_sorted_boxes_.begin(), zorder_sorted_boxes_.end(),
      [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
#else
  std::sort(
      zorder_sorted_boxes_.begin(), zorder_sorted_boxes_.end(),
      [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
#endif  // LINUX
}

void Grid::IterateZOrder(const std::function<void(const SoHandle&)>& lambda) {
  UpdateBoxZOrder();
  for (uint64_t i = 0; i < zorder_sorted_boxes_.size(); i++) {
    auto it = zorder_sorted_boxes_[i].second->begin();
    while (!it.IsAtEnd()) {
      lambda(*it);
      ++it;
    }
  }
}

void Grid::ForEachNeighbor(const std::function<void(const SimObject*)>& lambda,
                           const SimObject& query) const {
  const auto& position = query.GetPosition();
  auto idx = GetBoxIndex(position);

  FixedSizeVector<const Box*, 27> neighbor_boxes;
  GetMooreBoxes(&neighbor_boxes, idx);

  auto* rm = Simulation::GetActive()->GetResourceManager();

  NeighborIterator ni(neighbor_boxes);
  while (!ni.IsAtEnd()) {
    auto* sim_object = rm->GetSimObjectWithSoHandle(*ni);
    if (sim_object != &query) {
      lambda(sim_object);
    }
    ++ni;
  }
}

void Grid::ForEachNeighborWithinRadius(
    const std::function<void(const SimObject*)>& lambda, const SimObject& query,
    double squared_radius) {
  const auto& position = query.GetPosition();
  auto idx = query.GetBoxIdx();

  FixedSizeVector<const Box*, 27> neighbor_boxes;
  GetMooreBoxes(&neighbor_boxes, idx);

  auto* rm = Simulation::GetActive()->GetResourceManager();

  NeighborIterator ni(neighbor_boxes);
  while (!ni.IsAtEnd()) {
    // Do something with neighbor object
    auto* sim_object = rm->GetSimObjectWithSoHandle(*ni);
    if (sim_object != &query) {
      const auto& neighbor_position = sim_object->GetPosition();
      if (this->WithinSquaredEuclideanDistance(squared_radius, position,
                                               neighbor_position)) {
        lambda(sim_object);
      }
    }
    ++ni;
  }
}

size_t Grid::GetBoxIndex(const std::array<double, 3>& position) const {
  std::array<uint32_t, 3> box_coord;
  box_coord[0] = (floor(position[0]) - grid_dimensions_[0]) / box_length_;
  box_coord[1] = (floor(position[1]) - grid_dimensions_[2]) / box_length_;
  box_coord[2] = (floor(position[2]) - grid_dimensions_[4]) / box_length_;

  return GetBoxIndex(box_coord);
}

double Grid::GetLargestObjectSize() const { return largest_object_size_; }

const std::array<int32_t, 6>& Grid::GetDimensions() const {
  return grid_dimensions_;
}

const std::array<int32_t, 2>& Grid::GetDimensionThresholds() const {
  return threshold_dimensions_;
}

uint64_t Grid::GetNumBoxes() const { return boxes_.size(); }

uint32_t Grid::GetBoxLength() { return box_length_; }

bool Grid::HasGrown() { return has_grown_; }

std::array<uint32_t, 3> Grid::GetBoxCoordinates(size_t box_idx) const {
  std::array<uint32_t, 3> box_coord;
  box_coord[2] = box_idx / num_boxes_xy_;
  auto remainder = box_idx % num_boxes_xy_;
  box_coord[1] = remainder / num_boxes_axis_[0];
  box_coord[0] = remainder % num_boxes_axis_[0];
  return box_coord;
}

bool Grid::IsInitialized() { return initialized_; }

// NeighborMutex ---------------------------------------------------------

Grid::NeighborMutexBuilder::MutexWrapper::MutexWrapper() {}

Grid::NeighborMutexBuilder::MutexWrapper::MutexWrapper(const MutexWrapper&) {}

Grid::NeighborMutexBuilder::NeighborMutex::NeighborMutex(
    uint64_t box_idx, const FixedSizeVector<uint64_t, 27>& mutex_indices,
    NeighborMutexBuilder* mutex_builder)
    : box_idx_(box_idx),
      mutex_indices_(mutex_indices),
      mutex_builder_(mutex_builder) {
  // Deadlocks occur if mutliple threads try to acquire the same locks,
  // but in different order.
  // -> sort to avoid deadlocks - see lock ordering
  std::sort(mutex_indices_.begin(), mutex_indices_.end());
}

void Grid::NeighborMutexBuilder::NeighborMutex::lock() {  // NOLINT
  for (auto idx : mutex_indices_) {
    auto& mutex = mutex_builder_->mutexes_[idx].mutex_;
    // acquire lock (and spin if another thread is holding it)
    while (mutex.test_and_set(std::memory_order_acquire)) {
    }
  }
}

void Grid::NeighborMutexBuilder::NeighborMutex::unlock() {  // NOLINT
  for (auto idx : mutex_indices_) {
    auto& mutex = mutex_builder_->mutexes_[idx].mutex_;
    mutex.clear(std::memory_order_release);
  }
}

void Grid::NeighborMutexBuilder::Update() {
  auto* grid = Simulation::GetActive()->GetGrid();
  mutexes_.resize(grid->GetNumBoxes());
}

Grid::NeighborMutexBuilder::NeighborMutex Grid::NeighborMutexBuilder::GetMutex(
    uint64_t box_idx) {
  auto* grid = Simulation::GetActive()->GetGrid();
  FixedSizeVector<uint64_t, 27> box_indices;
  grid->GetMooreBoxIndices(&box_indices, box_idx);
  return NeighborMutex(box_idx, box_indices, this);
}

void Grid::DisableNeighborMutexes() { nb_mutex_builder_ = nullptr; }

Grid::NeighborMutexBuilder* Grid::GetNeighborMutexBuilder() {
  return nb_mutex_builder_.get();
}

void Grid::CheckGridGrowth() {
  // Determine if the grid dimensions have changed (changed in the sense that
  // the grid has grown outwards)
  auto min_gd =
      *std::min_element(grid_dimensions_.begin(), grid_dimensions_.end());
  auto max_gd =
      *std::max_element(grid_dimensions_.begin(), grid_dimensions_.end());
  if (min_gd < threshold_dimensions_[0]) {
    threshold_dimensions_[0] = min_gd;
    has_grown_ = true;
  }
  if (max_gd > threshold_dimensions_[1]) {
    Log::Info("Grid",
              "Your simulation objects are getting near the edge of "
              "the simulation space. Be aware of boundary conditions that "
              "may come into play!");
    threshold_dimensions_[1] = max_gd;
    has_grown_ = true;
  }
}

/// Calculates what the grid dimensions need to be in order to contain all the
/// simulation objects
void Grid::CalculateGridDimensions(std::array<double, 6>* ret_grid_dimensions) {
  auto* rm = Simulation::GetActive()->GetResourceManager();

  const auto max_threads = omp_get_max_threads();
  // allocate version for each thread - avoid false sharing by padding them
  // assumes 64 byte cache lines (8 * sizeof(double))
  std::vector<std::array<double, 8>> xmin(max_threads, {{Math::kInfinity}});
  std::vector<std::array<double, 8>> ymin(max_threads, {{Math::kInfinity}});
  std::vector<std::array<double, 8>> zmin(max_threads, {{Math::kInfinity}});

  std::vector<std::array<double, 8>> xmax(max_threads, {{-Math::kInfinity}});
  std::vector<std::array<double, 8>> ymax(max_threads, {{-Math::kInfinity}});
  std::vector<std::array<double, 8>> zmax(max_threads, {{-Math::kInfinity}});

  std::vector<std::array<double, 8>> largest(max_threads, {{0}});

  rm->ApplyOnAllElementsParallelDynamic(1000, [&](SimObject* so, SoHandle) {
    auto tid = omp_get_thread_num();
    const auto& position = so->GetPosition();
    // x
    if (position[0] < xmin[tid][0]) {
      xmin[tid][0] = position[0];
    }
    if (position[0] > xmax[tid][0]) {
      xmax[tid][0] = position[0];
    }
    // y
    if (position[1] < ymin[tid][0]) {
      ymin[tid][0] = position[1];
    }
    if (position[1] > ymax[tid][0]) {
      ymax[tid][0] = position[1];
    }
    // z
    if (position[2] < zmin[tid][0]) {
      zmin[tid][0] = position[2];
    }
    if (position[2] > zmax[tid][0]) {
      zmax[tid][0] = position[2];
    }
    // larget object
    auto diameter = so->GetDiameter();
    if (diameter > largest[tid][0]) {
      largest[tid][0] = diameter;
    }
  });

  // reduce partial results into global one
  double& gxmin = (*ret_grid_dimensions)[0];
  double& gxmax = (*ret_grid_dimensions)[1];
  double& gymin = (*ret_grid_dimensions)[2];
  double& gymax = (*ret_grid_dimensions)[3];
  double& gzmin = (*ret_grid_dimensions)[4];
  double& gzmax = (*ret_grid_dimensions)[5];
  for (int tid = 0; tid < max_threads; tid++) {
    // x
    if (xmin[tid][0] < gxmin) {
      gxmin = xmin[tid][0];
    }
    if (xmax[tid][0] > gxmax) {
      gxmax = xmax[tid][0];
    }
    // y
    if (ymin[tid][0] < gymin) {
      gymin = ymin[tid][0];
    }
    if (ymax[tid][0] > gymax) {
      gymax = ymax[tid][0];
    }
    // z
    if (zmin[tid][0] < gzmin) {
      gzmin = zmin[tid][0];
    }
    if (zmax[tid][0] > gzmax) {
      gzmax = zmax[tid][0];
    }
    // larget object
    if (largest[tid][0] > largest_object_size_) {
      largest_object_size_ = largest[tid][0];
    }
  }
}

void Grid::RoundOffGridDimensions(
    const std::array<double, 6>& grid_dimensions) {
  assert(grid_dimensions_[0] > -9.999999999);
  assert(grid_dimensions_[2] > -9.999999999);
  assert(grid_dimensions_[4] > -9.999999999);
  assert(grid_dimensions_[1] < 80);
  assert(grid_dimensions_[3] < 80);
  assert(grid_dimensions_[5] < 80);
  grid_dimensions_[0] = floor(grid_dimensions[0]);
  grid_dimensions_[2] = floor(grid_dimensions[2]);
  grid_dimensions_[4] = floor(grid_dimensions[4]);
  grid_dimensions_[1] = ceil(grid_dimensions[1]);
  grid_dimensions_[3] = ceil(grid_dimensions[3]);
  grid_dimensions_[5] = ceil(grid_dimensions[5]);
}

void Grid::GetMooreBoxes(FixedSizeVector<const Box*, 27>* neighbor_boxes,
                         size_t box_idx) const {
  neighbor_boxes->push_back(GetBoxPointer(box_idx));

  // Adjacent 6 (top, down, left, right, front and back)
  if (adjacency_ >= kLow) {
    neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_));
    neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_));
    neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_axis_[0]));
    neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_axis_[0]));
    neighbor_boxes->push_back(GetBoxPointer(box_idx - 1));
    neighbor_boxes->push_back(GetBoxPointer(box_idx + 1));
  }

  // Adjacent 12
  if (adjacency_ >= kMedium) {
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[0]));
    neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_ - 1));
    neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_axis_[0] - 1));
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[0]));
    neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_ - 1));
    neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_axis_[0] - 1));
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[0]));
    neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_ + 1));
    neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_axis_[0] + 1));
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[0]));
    neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_ + 1));
    neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_axis_[0] + 1));
  }

  // Adjacent 8
  if (adjacency_ >= kHigh) {
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[0] - 1));
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[0] + 1));
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[0] - 1));
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[0] + 1));
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[0] - 1));
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[0] + 1));
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[0] - 1));
    neighbor_boxes->push_back(
        GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[0] + 1));
  }
}

void Grid::GetMooreBoxIndices(FixedSizeVector<uint64_t, 27>* box_indices,
                              size_t box_idx) const {
  box_indices->push_back(box_idx);

  // Adjacent 6 (top, down, left, right, front and back)
  if (adjacency_ >= kLow) {
    box_indices->push_back(box_idx - num_boxes_xy_);
    box_indices->push_back(box_idx + num_boxes_xy_);
    box_indices->push_back(box_idx - num_boxes_axis_[0]);
    box_indices->push_back(box_idx + num_boxes_axis_[0]);
    box_indices->push_back(box_idx - 1);
    box_indices->push_back(box_idx + 1);
  }

  // Adjacent 12
  if (adjacency_ >= kMedium) {
    box_indices->push_back(box_idx - num_boxes_xy_ - num_boxes_axis_[0]);
    box_indices->push_back(box_idx - num_boxes_xy_ - 1);
    box_indices->push_back(box_idx - num_boxes_axis_[0] - 1);
    box_indices->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0]);
    box_indices->push_back(box_idx + num_boxes_xy_ - 1);
    box_indices->push_back(box_idx + num_boxes_axis_[0] - 1);
    box_indices->push_back(box_idx - num_boxes_xy_ + num_boxes_axis_[0]);
    box_indices->push_back(box_idx - num_boxes_xy_ + 1);
    box_indices->push_back(box_idx - num_boxes_axis_[0] + 1);
    box_indices->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0]);
    box_indices->push_back(box_idx + num_boxes_xy_ + 1);
    box_indices->push_back(box_idx + num_boxes_axis_[0] + 1);
  }

  // Adjacent 8
  if (adjacency_ >= kHigh) {
    box_indices->push_back(box_idx - num_boxes_xy_ - num_boxes_axis_[0] - 1);
    box_indices->push_back(box_idx - num_boxes_xy_ - num_boxes_axis_[0] + 1);
    box_indices->push_back(box_idx - num_boxes_xy_ + num_boxes_axis_[0] - 1);
    box_indices->push_back(box_idx - num_boxes_xy_ + num_boxes_axis_[0] + 1);
    box_indices->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0] - 1);
    box_indices->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0] + 1);
    box_indices->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0] - 1);
    box_indices->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0] + 1);
  }
}

void Grid::GetHalfMooreBoxIndices(FixedSizeVector<size_t, 14>* neighbor_boxes,
                                  size_t box_idx) const {
  // C
  neighbor_boxes->push_back(box_idx);
  // BW
  neighbor_boxes->push_back(box_idx + num_boxes_axis_[0] - 1);
  // FNW
  neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0] - 1);
  // NW
  neighbor_boxes->push_back(box_idx + num_boxes_xy_ - 1);
  // BNW
  neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0] - 1);
  // B
  neighbor_boxes->push_back(box_idx + num_boxes_axis_[0]);
  // FN
  neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0]);
  // N
  neighbor_boxes->push_back(box_idx + num_boxes_xy_);
  // BN
  neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0]);
  // E
  neighbor_boxes->push_back(box_idx + 1);
  // BE
  neighbor_boxes->push_back(box_idx + num_boxes_axis_[0] + 1);
  // FNE
  neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0] + 1);
  // NE
  neighbor_boxes->push_back(box_idx + num_boxes_xy_ + 1);
  // BNE
  neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0] + 1);
}

const Grid::Box* Grid::GetBoxPointer(size_t index) const {
  return &(boxes_[index]);
}

Grid::Box* Grid::GetBoxPointer(size_t index) { return &(boxes_[index]); }

size_t Grid::GetBoxIndex(const std::array<uint32_t, 3>& box_coord) const {
  return box_coord[2] * num_boxes_xy_ + box_coord[1] * num_boxes_axis_[0] +
         box_coord[0];
}

}  // namespace bdm
