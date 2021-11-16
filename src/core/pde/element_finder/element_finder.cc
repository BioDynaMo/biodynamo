// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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
#include "core/pde/element_finder/element_finder.h"

namespace bdm {

ElementFinder::ElementFinder(mfem::Mesh* mesh) : element_container_(mesh) {
  if (mesh->GetNE() > std::numeric_limits<int32_t>::max()) {
    Log::Fatal(
        "ElementFinder::ElementFinder",
        "The mesh you provided has to many Elements and is currently not ",
        "supported by the tree search. Largest number or supported elements "
        "is std::numeric_limits<int32_t>::max() = ",
        std::numeric_limits<int32_t>::max(), " but your mesh contains ",
        mesh->GetNE(), " elements.");
  }
  octree_ = new unibn::Octree<mfem::Vector, ElementContainer>();
  unibn::OctreeParams params;
  octree_->initialize(element_container_, params);
}

ElementFinder::~ElementFinder() { delete octree_; }

int ElementFinder::FindClosestElement(mfem::Vector& x) {
  return static_cast<int>(
      octree_->findNeighbor<unibn::L2Distance<mfem::Vector>>(x));
}

mfem::Vector ElementFinder::GetElementCenter(int element_id) {
  mfem::Vector center = element_container_[element_id];
  return center;
}
}  // namespace bdm

#endif  // USE_MFEM
