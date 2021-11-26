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
#ifndef ELEMENT_CONTAINER_H_
#define ELEMENT_CONTAINER_H_
#include <vector>
#include "core/container/math_array.h"
#include "mfem.hpp"

namespace bdm {

/// This class wraps the access to a mfem::Mesh for a unibn::Octree to enable
/// faster searching. This object may get large since it copies the center
/// elements to a vector. This vector will require
/// 3 x NumberOfElements x 8 bytes.
class ElementContainer {
 private:
  std::vector<Double3> element_center_points_;

  /// Copies the element center points of the mesh to element_center_points via
  /// std::vector<>.push_back(). Copies in serial.
  void Initialize(mfem::Mesh* mesh);

 public:
  ElementContainer(mfem::Mesh* mesh);

  /// Returns the number of elements of mesh_.
  size_t size() const;

  /// Retuns the center coordinate of a element in mesh_ labeled by idx.
  const Double3& operator[](size_t idx) const;
};

}  // namespace bdm

#endif  // ELEMENT_CONTAINER_H_
#endif  // USE_MFEM
