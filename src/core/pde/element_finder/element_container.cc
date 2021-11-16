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

namespace bdm {

ElementContainer::ElementContainer(mfem::Mesh* mesh) : mesh_(mesh) {}

size_t ElementContainer::size() const { return mesh_->GetNE(); }

const mfem::Vector ElementContainer::operator[](size_t idx) const {
  mfem::Vector point(3);
  mesh_->GetElementTransformation(idx)->Transform(
      mfem::Geometries.GetCenter(mesh_->GetElementBaseGeometry(idx)), point);
  return point;
}
}  // namespace bdm

#endif  // USE_MFEM
