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

#include "partitioner.h"

namespace bdm {

constexpr Surface SurfaceEnum::kNone;
constexpr Surface SurfaceEnum::kLeft;
constexpr Surface SurfaceEnum::kRight;
constexpr Surface SurfaceEnum::kTop;
constexpr Surface SurfaceEnum::kBottom;
constexpr Surface SurfaceEnum::kFront;
constexpr Surface SurfaceEnum::kBack;

void Partitioner::InitializeWithResourceManager(ResourceManager<> *rm) {
  double min_x = std::numeric_limits<double>::max();
  double max_x = std::numeric_limits<double>::min();
  double min_y = std::numeric_limits<double>::max();
  double max_y = std::numeric_limits<double>::min();
  double min_z = std::numeric_limits<double>::max();
  double max_z = std::numeric_limits<double>::min();
  auto f = [&](auto element, bdm::SoHandle) {
    std::array<double, 3> pos = element.GetPosition();
    min_x = std::min(min_x, pos[0]);
    max_x = std::max(max_x, pos[0]);
    min_y = std::min(min_y, pos[1]);
    max_y = std::max(max_y, pos[1]);
    min_z = std::min(min_z, pos[2]);
    max_z = std::max(max_z, pos[2]);
  };
  // Could parallel this min/max finding.
  rm->ApplyOnAllElements(f);
  max_x += 1e-9;
  max_y += 1e-9;
  max_z += 1e-9;

  InitializeWithBoundingBox({min_x, min_y, min_z},
                            {max_x, max_y, max_z});
}

Boxes CubePartitioner::Partition() const {
  Boxes ret;
  double x_range = right_back_top_[0] - left_front_bottom_[0];
  double y_range = right_back_top_[1] - left_front_bottom_[1];
  double z_range = right_back_top_[2] - left_front_bottom_[2];
  for (int z_factor = 0; z_factor < axial_factors_[1]; ++z_factor) {
    double z_start = left_front_bottom_[2] + z_range * z_factor / axial_factors_[2];
    double z_end = left_front_bottom_[2] + z_range * (z_factor + 1) / axial_factors_[2];
    for (int y_factor = 0; y_factor < axial_factors_[1]; ++y_factor) {
      double y_start = left_front_bottom_[1] + y_range * y_factor / axial_factors_[1];
      double y_end = left_front_bottom_[1] + y_range * (y_factor + 1) / axial_factors_[1];
      for (int x_factor = 0; x_factor < axial_factors_[0]; ++x_factor) {
        double x_start = left_front_bottom_[0] + x_range * x_factor / axial_factors_[0];
        double x_end = left_front_bottom_[0] + x_range * (x_factor + 1) / axial_factors_[0];
        ret.push_back({{x_start, y_start, z_start}, {x_end, y_end, z_end}});
      }
    }
  }
  return ret;
}

Box CubePartitioner::GetLocation(BoxId boxIndex) const {
  assert(boxIndex >= 0);
  assert(boxIndex < GetBoxCount());
  int z_index = boxIndex / (axial_factors_[0] * axial_factors_[1]);
  int yx = boxIndex % (axial_factors_[0] * axial_factors_[1]);
  int y_index = yx / axial_factors_[0];
  int x_index = yx % axial_factors_[0];
  double x_range = right_back_top_[0] - left_front_bottom_[0];
  double y_range = right_back_top_[1] - left_front_bottom_[1];
  double z_range = right_back_top_[2] - left_front_bottom_[2];
  return {{x_range * x_index / axial_factors_[0],
           y_range * y_index / axial_factors_[1],
           z_range * z_index / axial_factors_[2]},
          {x_range * (x_index + 1) / axial_factors_[0],
           y_range * (y_index + 1) / axial_factors_[1],
           z_range * (z_index + 1) / axial_factors_[2]}};
}

BoxId CubePartitioner::Locate(Point3D point) const {
  double x_step = (right_back_top_[0] - left_front_bottom_[0]) / axial_factors_[0];
  double y_step = (right_back_top_[1] - left_front_bottom_[1]) / axial_factors_[1];
  double z_step = (right_back_top_[2] - left_front_bottom_[2]) / axial_factors_[2];
  int x_index = std::floor((point[0] - left_front_bottom_[0]) / x_step);
  int y_index = std::floor((point[1] - left_front_bottom_[1]) / y_step);
  int z_index = std::floor((point[2] - left_front_bottom_[2]) / z_step);
  BoxId ret = z_index * (axial_factors_[0] * axial_factors_[1]) +
      y_index * axial_factors_[0] + x_index;
  assert(ret >= 0);
  assert(ret < GetBoxCount());
  return ret;
}

NeighborSurfaces CubePartitioner::GetNeighborSurfaces(BoxId boxIndex) const {
  assert(boxIndex >= 0);
  assert(boxIndex < GetBoxCount());
  int z_index = boxIndex / (axial_factors_[0] * axial_factors_[1]);
  int yx = boxIndex % (axial_factors_[0] * axial_factors_[1]);
  int y_index = yx / axial_factors_[0];
  int x_index = yx % axial_factors_[0];
  std::vector<std::pair<int, Surface>> x_neighbor_surfaces{{0, SurfaceEnum::kNone}};
  std::vector<std::pair<int, Surface>> y_neighbor_surfaces{{0, SurfaceEnum::kNone}};
  std::vector<std::pair<int, Surface>> z_neighbor_surfaces{{0, SurfaceEnum::kNone}};
  // We will see if there's any box adjacent to this box.
  // If there is, we take the adjacent surface of that box.
  // For e.g. if there's a box to the left, we take its right surface.
  if (x_index - 1 >= 0) {
    x_neighbor_surfaces.push_back({-1, SurfaceEnum::kRight});
  }
  if (x_index + 1 < axial_factors_[0]) {
    x_neighbor_surfaces.push_back({1, SurfaceEnum::kLeft});
  }
  if (y_index - 1 >= 0) {
    y_neighbor_surfaces.push_back({-1, SurfaceEnum::kBack});
  }
  if (y_index + 1 < axial_factors_[1]) {
    y_neighbor_surfaces.push_back({1, SurfaceEnum::kFront});
  }
  if (z_index - 1 >= 0) {
    z_neighbor_surfaces.push_back({-1, SurfaceEnum::kTop});
  }
  if (z_index + 1 < axial_factors_[2]) {
    z_neighbor_surfaces.push_back({1, SurfaceEnum::kBottom});
  }
  NeighborSurfaces ret;
  for (const std::pair<int, Surface>& xs : x_neighbor_surfaces) {
    for (const std::pair<int, Surface>& ys : y_neighbor_surfaces) {
      for (const std::pair<int, Surface>& zs : z_neighbor_surfaces) {
        Surface surface = xs.second | ys.second | zs.second;
        if (surface == SurfaceEnum::kNone) {
          continue;
        }
        BoxId box = (x_index + xs.first) +
            (y_index + ys.first) * axial_factors_[0] +
            (z_index + zs.first) * (axial_factors_[0] + axial_factors_[1]);
        ret.push_back({box, surface});
      }
    }
  }
  return ret;
}

}  // namespace bdm
