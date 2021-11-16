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
#include "core/util/log.h"
#include "unibn_octree.h"

namespace bdm {

struct ElementFinder::OctreeImplementation {
  ~OctreeImplementation() { delete tree; };
  unibn::Octree<mfem::Vector, ElementContainer>* tree = nullptr;
};

ElementFinder::ElementFinder(mfem::Mesh* mesh)
    : mesh_(mesh), element_container_(mesh) {
  if (mesh->GetNE() > std::numeric_limits<int32_t>::max()) {
    Log::Fatal(
        "ElementFinder::ElementFinder",
        "The mesh you provided has to many Elements and is currently not ",
        "supported by the tree search. Largest number or supported elements "
        "is std::numeric_limits<int32_t>::max() = ",
        std::numeric_limits<int32_t>::max(), " but your mesh contains ",
        mesh->GetNE(), " elements.");
  }
  // Load vertex to element table from mesh
  vertex_to_element_ = mesh->GetVertexToElementTable();
  // Initialize wrapper for unibn::Octree
  octree_ = std::unique_ptr<ElementFinder::OctreeImplementation>(
      new ElementFinder::OctreeImplementation());
  octree_->tree = new unibn::Octree<mfem::Vector, ElementContainer>();
  unibn::OctreeParams params;
  // Build the Octree
  octree_->tree->initialize(element_container_, params);
}

ElementFinder::~ElementFinder() { delete vertex_to_element_; }

int ElementFinder::FindClosestElement(mfem::Vector& x) {
  return static_cast<int>(
      octree_->tree->findNeighbor<unibn::L2Distance<mfem::Vector>>(x));
}

int ElementFinder::FindClosestElement(const Double3& x) {
  mfem::Vector mfem_x = ConvertToMFEMVector(x);
  return FindClosestElement(mfem_x);
}

mfem::Vector ElementFinder::GetElementCenter(int element_id) {
  mfem::Vector center = element_container_[element_id];
  return center;
}

mfem::Vector ElementFinder::ConvertToMFEMVector(const Double3& point) {
  mfem::Vector vec({point[0], point[1], point[2]});
  return vec;
}

bool ElementFinder::IsInElement(const Double3& point, int element_id,
                                mfem::IntegrationPoint* ip) {
  // Catch error such that we cannot derive a elment that is not in the mesh.
  if (element_id >= mesh_->GetNE()) {
    return false;
  }
  bool ip_allocated = false;
  if (ip == nullptr) {
    ip = new mfem::IntegrationPoint();
    ip_allocated = true;
  }
  mfem::Vector vec = ConvertToMFEMVector(point);
  mfem::InverseElementTransformation inv_tr;
  auto trans = mesh_->GetElementTransformation(element_id);
  inv_tr.SetTransformation(*trans);
  bool is_inside = (inv_tr.Transform(vec, *ip) ==
                    mfem::InverseElementTransformation::Inside);
  if (ip_allocated) {
    delete ip;
  }
  return is_inside;
}

bool ElementFinder::IsInNeighborElements(const Double3& point, int& element_id,
                                         mfem::IntegrationPoint* ip) {
  // Catch error such that we cannot derive a elment that is not in the mesh.
  if (element_id >= mesh_->GetNE()) {
    return false;
  }
  bool ip_allocated = false;
  if (ip == nullptr) {
    ip = new mfem::IntegrationPoint();
    ip_allocated = true;
  }

  // If the search in the neighboring elements is not successful, we return
  // INT_MAX.
  int containing_neighbor_element = std::numeric_limits<int>::max();

  // Integer array in which we store the vertices of the element
  // finite_element_id. We iterate over all vertices.
  mfem::Array<int> vertices;
  mesh_->GetElementVertices(element_id, vertices);
  for (int v = 0; v < vertices.Size(); v++) {
    // For each vertex, we get all elements that touch it / are connected
    // through it.
    int vertex = vertices[v];
    int number_of_elements = vertex_to_element_->RowSize(vertex);
    const int* element_ids = vertex_to_element_->GetRow(vertex);
    for (int e = 0; e < number_of_elements; e++) {
      // We don't want to check the original element as we want to scan the
      // neighbors.
      if (element_ids[e] == element_id) {
        continue;
      }
      // For each element, we test if if it contains the point.
      bool is_inside = IsInElement(point, element_ids[e], ip);
      if (is_inside) {
        containing_neighbor_element = element_ids[e];
        break;
      }
    }
  }
  // Warning for non-conforming meshes
  if (mesh_->ncmesh &&
      containing_neighbor_element == std::numeric_limits<int>::max()) {
    Log::Warning(
        "ElementFinder::IsInNeighborElements",
        "You seem to use a non-conforming mesh. Iterating over neighbors in ",
        "non-conforming meshes is currently not supported since it requires "
        "access",
        " to private / protected members of mfem::Mesh.");
  }
  if (ip_allocated) {
    delete ip;
  }
  element_id = containing_neighbor_element;
  return (containing_neighbor_element != std::numeric_limits<int>::max());
}

std::pair<int, mfem::IntegrationPoint> ElementFinder::FindPointWithOctree(
    const Double3& point, int expected_element_id) {
  int true_element_id = std::numeric_limits<int>::max();
  mfem::IntegrationPoint ip;
  if (IsInElement(point, expected_element_id, &ip) &&
      expected_element_id != std::numeric_limits<int>::max()) {
    true_element_id = expected_element_id;
    // // Empirical experiments showed, that it is usually faster to recompute
    // // the closest center and test the corresponding element. However, its
    // // not clear if this is always the case and further experiments are
    // // needed to prove this hypothesis. Until then, this code segment remains
    // // here as a comment.
    // } else if (IsInNeighborElements(point, expected_element_id, &ip) &&
    //            expected_element_id != std::numeric_limits<int>::max()) {
    //   true_element_id = expected_element_id;
  } else {
    expected_element_id = FindClosestElement(point);
    if (IsInElement(point, expected_element_id, &ip)) {
      true_element_id = expected_element_id;
    } else if (IsInNeighborElements(point, expected_element_id, &ip)) {
      true_element_id = expected_element_id;
    } else {
      Log::Fatal(
          "ElementFinder::FindPointWithOctree",
          "Point could not be located in Mesh. Point: (", point,
          ")\nIf you see this error message, this can be for either of "
          "some reasons:\n",
          "1) You manually called this function for a point that is not "
          "in the FE mesh\n"
          "2) Your agent simulation is not fully contained in the FE mesh "
          "- see VerifyBDMCompatibility()\n",
          "3) An Agent tried to request a value from the continuum in the",
          " syncronization phase but was not located inside the FE Mesh\n",
          "4) MFEM's internal methods failed to find the point for any ",
          "other reason.\n(This list may not be exhaustive.)");
    }
  }
  if (true_element_id == std::numeric_limits<int>::max()) {
    Log::Fatal("ElementFinder::FindPointWithOctree",
               "Unexpectedly, a point could not be found.");
  }
  return std::make_pair(true_element_id, ip);
}

std::pair<int, mfem::IntegrationPoint> ElementFinder::FindPointWithMFEM(
    const Double3& point) {
  Log::Warning("ElementFinder::FindPointWithMFEM",
               "This function is slow, consider using "
               "ElementFinder::FindPointWithOctree");
  // This is not the most efficient way to transfer the information but
  // currently necessary.
  mfem::DenseMatrix mfem_position(mesh_->Dimension(), 1);
  for (int i = 0; i < mesh_->Dimension(); i++) {
    mfem_position(i, 0) = point[i];
  }
  mfem::Array<int> element_id;
  mfem::Array<mfem::IntegrationPoint> integration_points;
  // This function is particularly slow since it searches the space rather
  // inefficiently. It would be good to have something more efficient.
  auto found = mesh_->FindPoints(mfem_position, element_id, integration_points);
  if (found == 0) {
    Log::Fatal("ElementFinder::FindPointWithMFEM",
               "Point could not be located in Mesh. Point: (", point,
               ")\nIf you see this error message, this can be for either of "
               "some reasons:\n",
               "1) You manually called this function for a point that is not "
               "in the FE mesh\n"
               "2) Your agent simulation is not fully contained in the FE mesh "
               "- see VerifyBDMCompatibility()\n",
               "3) An Agent tried to request a value from the continuum in the",
               " syncronization phase but was not located inside the FE Mesh\n",
               "4) MFEM's internal methods failed to find the point for any ",
               "other reason.\n(This list may not be exhaustive.)");
  }
  return std::make_pair(element_id[0], integration_points[0]);
}

}  // namespace bdm

#endif  // USE_MFEM
