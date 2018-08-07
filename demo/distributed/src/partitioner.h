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

#ifndef DEMO_DISTRIBUTED_PARTITIONER_H_
#define DEMO_DISTRIBUTED_PARTITIONER_H_

#include "backend.h"

namespace bdm {

class Surface {
 public:
  Surface() : Surface(0) {}

  constexpr Surface intersect(const Surface &s) const {
    return Surface(value_ | s.value_);
  }

  constexpr bool operator==(const Surface &other) const {
    return value_ == other.value_;
  }

  constexpr Surface operator|(const Surface &other) const {
    return intersect(other);
  }

  constexpr bool conflict(const Surface &other) const {
    return (value_ == 1 && other.value_ == 8) ||
        (value_ == 2 && other.value_ == 16) ||
        (value_ == 4 && other.value_ == 32) ||
        (value_ == 8 && other.value_ == 1) ||
        (value_ == 16 && other.value_ == 4) ||
        (value_ == 32 && other.value_ == 8);
  }

  constexpr operator int() const {
    return value_;
  }

 private:
  constexpr explicit Surface(int v) : value_(v) {}
  int value_;
  friend class SurfaceEnum;
};

class SurfaceEnum {
 public:
  static constexpr Surface kNone{0};
  static constexpr Surface kLeft{1};
  static constexpr Surface kFront{2};
  static constexpr Surface kBottom{4};
  static constexpr Surface kRight{8};
  static constexpr Surface kBack{16};
  static constexpr Surface kTop{32};
};

using Point3D = std::array<double, 3>;
using Box = std::pair<Point3D, Point3D>;
using BoxId = int;
using Boxes = std::vector<Box>;

/// Returns true if pos is in a bounded box.
///
/// /param pos the location to check
/// /param left_front_bottom (inclusive)
/// /param right_back_top (exclusive)
static inline bool is_in(const Point3D& pos,
                         const Point3D& left_front_bottom,
                         const Point3D& right_back_top) {
  double x = pos[0];
  double y = pos[1];
  double z = pos[2];
  return x >= left_front_bottom[0] && x < right_back_top[0] &&
      y >= left_front_bottom[1] && y < right_back_top[1] &&
      z >= left_front_bottom[2] && z < right_back_top[2];
}

static inline bool is_in(const Point3D& pos,
                         const Box& box) {
  return is_in(pos, box.first, box.second);
}

using NeighborSurface = std::pair<BoxId, Surface>;
using NeighborSurfaces = std::vector<NeighborSurface>;

class Partitioner {
 public:
  /// Initializes a partitioner with the bounding box.
  virtual void InitializeWithBoundingBox(
      const Point3D &left_front_bottom,
      const Point3D &right_back_top) {
    left_front_bottom_ = left_front_bottom;
    right_back_top_ = right_back_top;
  }

  /// Initializes a partitioner with simulation objects contained in rm.
  virtual void InitializeWithResourceManager(ResourceManager<>* rm);

  virtual ~Partitioner() {}

  /// Returns the number of boxes that would be partitioned into.
  virtual int GetBoxCount() const = 0;

  /// Partitions the given box into smaller ones.
  virtual Boxes Partition() const = 0;

  /// Returns the bounding box by left_front_bottom and right_back_top.
  Box GetBoundingBox() const {
    return {left_front_bottom_, right_back_top_};
  }

  /// Retrieves the coordinates of the box indexed by boxIndex.
  ///
  /// This is basically the same as Partition()[boxIndex] but may be more
  /// optimized in concrete partitioner implementations.
  virtual Box GetLocation(BoxId boxIndex) const {
    assert(boxIndex >= 0);
    Boxes boxes = Partition();
    assert(static_cast<size_t>(boxIndex) < boxes.size());
    return boxes[boxIndex];
  }

  /// Returns the box index containing point.
  virtual BoxId Locate(Point3D point) const = 0;

  /// Returns all surfaces surrounding the box indexed by boxIndex.
  ///
  /// There could be 6 full surfaces, 12 edges, and 8 corners neighboring a box.
  virtual NeighborSurfaces GetNeighborSurfaces(BoxId boxIndex) const = 0;

 protected:
  Point3D left_front_bottom_;
  Point3D right_back_top_;
};

class CubePartitioner : public Partitioner {
 public:
  CubePartitioner(const std::array<int, 3>& axial_factors) :
      axial_factors_(axial_factors) {
    assert(axial_factors[0] >= 1);
    assert(axial_factors[1] >= 1);
    assert(axial_factors[2] >= 1);
    assert(axial_factors[0] * axial_factors[1] >= 1);
    assert(axial_factors[0] * axial_factors[1] * axial_factors[2] >= 1);
  }

  virtual int GetBoxCount() const override {
    return axial_factors_[0] * axial_factors_[1] * axial_factors_[2];
  }

  virtual Boxes Partition() const override;

  virtual Box GetLocation(BoxId boxIndex) const override;

  virtual BoxId Locate(Point3D point) const override;

  virtual NeighborSurfaces GetNeighborSurfaces(BoxId boxIndex) const override;
 private:
  std::array<int, 3> axial_factors_;
};

}  // namespace bdm
#endif  // DEMO_DISTRIBUTED_PARTITIONER_H_
