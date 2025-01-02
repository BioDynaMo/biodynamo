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

int main(int argc, const char** argv) { return bdm::astrophysics::Simulate(argc, argv); }

namespace bdm {
  namespace astrophysics {

    Real3 RotateVector(const Real3& Vec, real_t th, char Axis) {
        th *= M_PI / 180;
        real_t costh = std::cos(th), sinth = std::sin(th);

        Real3 RotVec;
        switch (Axis) {
            case 'x':
                RotVec[0] = Vec[0];
                RotVec[1] = Vec[1] * costh - Vec[2] * sinth;
                RotVec[2] = Vec[1] * sinth + Vec[2] * costh;
                break;
            case 'y':
                RotVec[0] = Vec[0] * costh + Vec[2] * sinth;
                RotVec[1] = Vec[1];
                RotVec[2] = -Vec[0] * sinth + Vec[2] * costh;
                break;
            case 'z':
                RotVec[0] = Vec[0] * costh - Vec[1] * sinth;
                RotVec[1] = Vec[0] * sinth + Vec[1] * costh;
                RotVec[2] = Vec[2];
                break;
            default:
                throw std::invalid_argument("Invalid rotation axis. Must be 'x', 'y', or 'z'.");
        }
        return RotVec;
    }
  } // namespace astrophysics
} // namespace bdm
