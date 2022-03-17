// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#include "core/agent/cell.h"

namespace bdm {

const Real3 Cell::kXAxis = {1.0, 0.0, 0.0};
const Real3 Cell::kYAxis = {0.0, 1.0, 0.0};
const Real3 Cell::kZAxis = {0.0, 0.0, 1.0};

void Cell::ApplyDisplacement(const Real3& displacement) {
  if (displacement[0] == 0 && displacement[1] == 0 && displacement[2] == 0) {
    return;
  }

  UpdatePosition(displacement);
  // Reset biological movement to 0.
  SetTractorForce({0, 0, 0});
}

Real3 Cell::TransformCoordinatesGlobalToPolar(const Real3& pos) const {
  auto vector_to_point = pos - position_;
  Real3 local_cartesian{kXAxis * vector_to_point, kYAxis * vector_to_point,
                          kZAxis * vector_to_point};
  real_t radius = std::sqrt(local_cartesian[0] * local_cartesian[0] +
                            local_cartesian[1] * local_cartesian[1] +
                            local_cartesian[2] * local_cartesian[2]);
  return {radius, std::acos(local_cartesian[2] / radius),
          std::atan2(local_cartesian[1], local_cartesian[0])};
}

}  // namespace bdm
