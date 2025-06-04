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
#include "mars.h"

int main(int argc, const char** argv) {
  return bdm::astrophysics::Simulate(argc, argv);
}

namespace bdm {
namespace astrophysics {

Real3 RotateVector(const Real3& vec, real_t th, char axis) {
  th *= M_PI / 180;
  real_t costh = std::cos(th), sinth = std::sin(th);

  Real3 rot_vec;
  switch (axis) {
    case 'x':
      rot_vec[0] = vec[0];
      rot_vec[1] = vec[1] * costh - vec[2] * sinth;
      rot_vec[2] = vec[1] * sinth + vec[2] * costh;
      break;
    case 'y':
      rot_vec[0] = vec[0] * costh + vec[2] * sinth;
      rot_vec[1] = vec[1];
      rot_vec[2] = -vec[0] * sinth + vec[2] * costh;
      break;
    case 'z':
      rot_vec[0] = vec[0] * costh - vec[1] * sinth;
      rot_vec[1] = vec[0] * sinth + vec[1] * costh;
      rot_vec[2] = vec[2];
      break;
    default:
      throw std::invalid_argument(
          "Invalid rotation axis. Must be 'x', 'y', or 'z'.");
  }
  return rot_vec;
}
}  // namespace astrophysics
}  // namespace bdm
