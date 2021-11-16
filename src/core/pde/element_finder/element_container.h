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
#ifndef ELEMENT_CONTAINER_H_
#define ELEMENT_CONTAINER_H_
#include "mfem.hpp"

namespace bdm {

/// This class wraps the access to a mfem::Mesh for a unibn::Octree to enable
/// faster searching.
class ElementContainer {
 private:
  mfem::Mesh* mesh_;

 public:
  ElementContainer(mfem::Mesh* mesh);

  /// Returns the number of elements of mesh_.
  size_t size() const;

  /// Retuns the center coordinate of a element in mesh_ labeled by idx.
  const mfem::Vector operator[](size_t idx) const;
};

}  // namespace bdm

#endif  // ELEMENT_CONTAINER_H_
#endif  // USE_MFEM
