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

#ifdef USE_MFEM
#include "core/pde/element_finder/element_container.h"
#include <cassert>

namespace bdm {

ElementContainer::ElementContainer(mfem::Mesh* mesh) { Initialize(mesh); }

// In the initialization we **copy** each center position of the elements to
// a vector. This is done for three reasons:
// 1) To obtain the center, MFEM call stack goes through 4-5 functions whereas
// we only need to access a certain vector elment here.
// 2) The default access to MFEMs center elements
// ```
// mesh_->GetElementTransformation(idx)->Transform(
//      mfem::Geometries.GetCenter(mesh_->GetElementBaseGeometry(idx)), point);
// ```
// is not thread safe. This has to do with the GetElmentTransformation(int) call
// but could most likely be resolved by providing a local ElementTransformation.
// 3) By storing the Double3 center positions we can compare them directly to
// the Double3 agent positions rather than convering them to mfem::Vector
// objects first.
void ElementContainer::Initialize(mfem::Mesh* mesh) {
  assert(mesh != nullptr && "Mesh is a nullpointer.");
  element_center_points_.reserve(mesh->GetNE());
  for (int i = 0; i < mesh->GetNE(); i++) {
    mfem::Vector point(3);
    mesh->GetElementTransformation(i)->Transform(
        mfem::Geometries.GetCenter(mesh->GetElementBaseGeometry(i)), point);
    Double3 bdm_point{point[0], point[1], point[2]};
    element_center_points_.push_back(bdm_point);
  }
}

size_t ElementContainer::size() const { return element_center_points_.size(); }

const Double3& ElementContainer::operator[](size_t idx) const {
  assert(idx < element_center_points_.size() && "Out of bounds access.");
  return element_center_points_[idx];
}
}  // namespace bdm

#endif  // USE_MFEM
