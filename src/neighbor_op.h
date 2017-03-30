#ifndef NEIGHBOR_OP_H_
#define NEIGHBOR_OP_H_

#include <cmath>
#include <iostream>
#include "spatial/octree_node.h"

namespace bdm {

class NeighborOp {
 public:
  NeighborOp() {}
  NeighborOp(double distance) : distance_(distance) {}
  ~NeighborOp() {}
  NeighborOp(const NeighborOp&) = delete;
  NeighborOp& operator=(const NeighborOp&) = delete;

  template <typename daosoa>
  Vc_ALWAYS_INLINE void Compute(daosoa* cells) const {
    std::vector<std::array<InlineVector<int, 8>, VcBackend::kVecLen> >
        neighbors(cells->vectors());

    // calc neighbors

    // Tree search
    // Creating a spatial tree (Bound, max_depth_, capacity_of_leaf_node)
    // IMPORTANT! Ensure that your bound is big enough and enclose all parts of
    // simulation
    SpatialTreeNode<size_t>* tree = new OctreeNode<size_t>(
        Bound(-10000.0, -10000.0, -10000.0, 10000.0, 10000.0, 10000.0), 100,
        10);

    // Initializing tree with a objects
    for (size_t i = 0; i < cells->elements(); i++) {
      auto cell = cells->GetScalar(i);
      const auto& position = cell.GetPosition();
      Point pos(position[0][0], position[1][0], position[2][0]);
      tree->Put(pos, i);
    }
    const VcBackend::real_t search_radius =
        sqrt(static_cast<VcBackend::real_t>(distance_));

    // Container for neighbors
    InlineVector<int, 8>* i_neighbors =
        new InlineVector<int, 8>[ cells->elements() ];

    // std::cout << "Neighbor search. Distance " << search_radius << std::endl;
    // Getting all pairs within the 'search_radius distance'
    auto tree_neighbors = tree->GetNeighbors(search_radius);
    int amount_of_pairs = tree_neighbors.size();

    // Filling container for neighbors for every object
    for (int i = 0; i < amount_of_pairs; i++) {
      size_t neighbor_a = tree_neighbors[i].first;
      size_t neighbor_b = tree_neighbors[i].second;
      i_neighbors[neighbor_a].push_back(neighbor_b);
      i_neighbors[neighbor_b].push_back(neighbor_a);
    }
    for (size_t i = 0; i < cells->elements(); i++) {
      const auto vector_idx = i / VcBackend::kVecLen;
      const auto scalar_idx = i % VcBackend::kVecLen;
      neighbors[vector_idx][scalar_idx] = std::move(i_neighbors[i]);
    }
    delete tree;
    delete[] i_neighbors;

// std::cout << "End of search." << std::endl;
// End of tree search

// update neighbors
#pragma omp parallel for
    for (size_t i = 0; i < cells->vectors(); i++) {
      (*cells)[i].SetNeighbors(neighbors[i]);
    }
  }

 private:
  double distance_ = 3000;
};

}  // namespace bdm

#endif  // NEIGHBOR_OP_H_
