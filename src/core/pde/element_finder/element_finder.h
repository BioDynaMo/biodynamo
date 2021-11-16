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
#ifndef ELEMENT_FINDER_H_
#define ELEMENT_FINDER_H_

#include <limits>
#include "core/container/math_array.h"
#include "core/pde/element_finder/element_container.h"

namespace bdm {

/// The ElementFinder is a Octree based search engine to identify the closest
/// center of an element relative to a given vector.
class ElementFinder {
 private:
  /// Struct to wrap the octree implementation (see best practices, e.g. in
  /// OctreeEnvironment).
  struct OctreeImplementation;
  /// Octree for spatial searches
  std::unique_ptr<OctreeImplementation> octree_;
  /// mfem::Mesh
  mfem::Mesh* mesh_;
  /// Wraps the access to a mfem::Mesh for octree search
  ElementContainer element_container_;
  /// Vertex to Element Table from mfem mesh.
  mfem::Table* vertex_to_element_;

  /// Converts a Double3 to a corresponding MFEM vector.
  mfem::Vector ConvertToMFEMVector(const Double3& point);

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

  /// Calls back to FindClosestElement(mfem::Vector&).
  int FindClosestElement(const Double3& x);

  /// Get the center coordinates of a given element labeled by element_id.
  mfem::Vector GetElementCenter(int element_id);

  /// Function to retrieve the element containing a point as well as the closest
  /// integration point. First, this function uses a octree to find the closest
  /// center among all elements. Afterwards, it checks if the position is either
  /// in the respective element or in it's neighbouring elements. If an
  /// expected_element_id is provide, we first check if the point is in this
  /// element before initiating a search.
  std::pair<int, mfem::IntegrationPoint> FindPointWithOctree(
      const Double3& point,
      int expected_element_id = std::numeric_limits<int>::max());

  /// Wrapper to mfem::Mesh::FindPoints(). First iterates over all elements
  /// to find the closest center. Afterwards, it checks if the position is
  /// either in the respective element or in it's neighbouring elements. This
  /// member function is expensive to call and should therefore be used with
  /// caution and avoided.
  std::pair<int, mfem::IntegrationPoint> FindPointWithMFEM(
      const Double3& point);

  /// Verifies if the provided point lies in the element labeled with
  /// element_id.
  bool IsInElement(const Double3& point, int element_id,
                   mfem::IntegrationPoint* ip = nullptr);

  /// Verifies if the provided point lies in the neighbouring elements of the
  /// element labeled with element_id. While crucial, this function is not very
  /// fast, bear in mind when using.
  bool IsInNeighborElements(const Double3& point, int& element_id,
                            mfem::IntegrationPoint* ip = nullptr);
};

}  // namespace bdm

#endif  // ELEMENT_FINDER_H_
#endif  // USE_MFEM
