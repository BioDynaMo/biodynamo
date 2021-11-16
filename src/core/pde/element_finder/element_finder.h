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
#ifndef ELEMENT_FINDER_H_
#define ELEMENT_FINDER_H_

#include <limits>
#include "core/pde/element_finder/element_container.h"
#include "core/util/log.h"
#include "unibn_octree.h"

namespace bdm {

/// The ElementFinder is a Octree based search engine to identify the closest
/// center of an element relative to a given vector.
class ElementFinder {
 private:
  /// Wraps the access to a mfem::Mesh
  ElementContainer element_container_;
  /// Octree implementation used for searches
  unibn::Octree<mfem::Vector, ElementContainer>* octree_;

 public:
  /// Constructing a ElementFinder requires a mfem::Mesh. During this call, the
  /// Octree is build for fast searches.
  ElementFinder(mfem::Mesh* mesh);

  /// Frees the heap memory allocated by the constructed octree.
  ~ElementFinder();

  /// Given a mfem::Vector, the function returns the element_id of the mesh
  /// whose center is the closest to the vector x (measured as L2 distance).
  /// Note: this does not imply that the element labeled by element_id contains
  /// the vector x.
  int FindClosestElement(mfem::Vector& x);

  /// Get the center coordinates of a given element labeled by element_id.
  mfem::Vector GetElementCenter(int element_id);
};

}  // namespace bdm

#endif  // ELEMENT_FINDER_H_
#endif  // USE_MFEM
