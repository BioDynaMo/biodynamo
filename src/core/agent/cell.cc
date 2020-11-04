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

const Double3 Cell::kXAxis = {1.0, 0.0, 0.0};
const Double3 Cell::kYAxis = {0.0, 1.0, 0.0};
const Double3 Cell::kZAxis = {0.0, 0.0, 1.0};

void Cell::ApplyDisplacement(const Double3& displacement) {
  if (displacement[0] == 0 && displacement[1] == 0 && displacement[2] == 0) {
    return;
  }

  UpdatePosition(displacement);
  // Reset biological movement to 0.
  SetTractorForce({0, 0, 0});
}

Double3 Cell::TransformCoordinatesGlobalToPolar(const Double3& pos) const {
  auto vector_to_point = pos - position_;
  Double3 local_cartesian{kXAxis * vector_to_point, kYAxis * vector_to_point,
                          kZAxis * vector_to_point};
  double radius = std::sqrt(local_cartesian[0] * local_cartesian[0] +
                            local_cartesian[1] * local_cartesian[1] +
                            local_cartesian[2] * local_cartesian[2]);
  return {radius, std::acos(local_cartesian[2] / radius),
          std::atan2(local_cartesian[1], local_cartesian[0])};
}

}  // namespace bdm
