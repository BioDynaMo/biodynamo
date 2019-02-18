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

#include "cell.h"

namespace bdm {

constexpr std::array<double, 3> Cell::kXAxis;
constexpr std::array<double, 3> Cell::kYAxis;
constexpr std::array<double, 3> Cell::kZAxis;

void Cell::ApplyDisplacement(const std::array<double, 3>& displacement) {
  UpdatePosition(displacement);
  // Reset biological movement to 0.
  SetTractorForce({0, 0, 0});
}

std::array<double, 3> Cell::TransformCoordinatesGlobalToPolar(
    const std::array<double, 3>& pos) const {
  auto vector_to_point = Math::Subtract(pos, position_);
  std::array<double, 3> local_cartesian{Math::Dot(kXAxis, vector_to_point),
                                        Math::Dot(kYAxis, vector_to_point),
                                        Math::Dot(kZAxis, vector_to_point)};
  double radius = std::sqrt(local_cartesian[0] * local_cartesian[0] +
                            local_cartesian[1] * local_cartesian[1] +
                            local_cartesian[2] * local_cartesian[2]);
  return {radius, std::acos(local_cartesian[2] / radius),
          std::atan2(local_cartesian[1], local_cartesian[0])};
}

}  // namespace bdm
