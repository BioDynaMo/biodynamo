#ifndef NEIGHBOR_OP_H_
#define NEIGHBOR_OP_H_

#include <cmath>
#include <iostream>
#include <vector>
// #include <pair>
#include "inline_vector.h"
#include "spatial_organization/octree_node.h"
#include "timing.h"

namespace bdm {

using spatial_organization::SpatialTreeNode;
using spatial_organization::OctreeNode;
using spatial_organization::Bound;
using spatial_organization::Point;

class NeighborOp {
 public:
  NeighborOp() {}
  explicit NeighborOp(double distance) : distance_(distance) {}
  ~NeighborOp() {}
  NeighborOp(const NeighborOp&) = delete;
  NeighborOp& operator=(const NeighborOp&) = delete;

  template <typename TContainer>
  void Compute(TContainer* cells) const {
    // Tree search
    // Creating a spatial tree (Bound, max_depth_, capacity_of_leaf_node)
    // IMPORTANT! Ensure that your bound is big enough and enclose all parts of
    // simulation
    auto build_timer = new Timing("build time ");
    OctreeNode<size_t>* tree = new OctreeNode<size_t>(
        Bound(-10000.0, -10000.0, -10000.0, 10000.0, 10000.0, 10000.0), 100,
        20);

    // Initializing tree with a objects
    for (size_t i = 0; i < cells->size(); i++) {
      auto&& cell = (*cells)[i];
      const auto& position = cell.GetPosition();
      Point pos(position[0], position[1], position[2]);
      tree->Put(pos, i);
    }
    delete build_timer;

    size_t num_neighbors = 0;
    size_t calls;

    tree->Traverse([&](const std::vector<std::pair<Point, size_t> >& elements) {
      const size_t size = elements.size();
      num_neighbors += size;
      calls++;
      for (size_t i = 0; i < size; i++) {
        InlineVector<int, 8> neighbors;
        neighbors.reserve(size-1);
        for (size_t j = 0; j < size; j++) {
          if (i != j) {
            neighbors.push_back(elements[j].second);
          }
        }
        (*cells)[elements[i].second].SetNeighbors(neighbors);
      }
    });

    std::cout << (num_neighbors + 1.0) / calls << std::endl;
    std::cout << calls << std::endl;

//     double search_radius = sqrt(distance_);
//
//     // Container for neighbors
//     InlineVector<int, 8>* i_neighbors =
//         new InlineVector<int, 8>[ cells->size() ];
//
//     // Getting all pairs within the 'search_radius distance'
//     auto tree_neighbors = tree->GetNeighbors(search_radius);
//     int amount_of_pairs = tree_neighbors.size();
//
//     // Filling container for neighbors for every object
//     // ragma omp parallel for
//     for (int i = 0; i < amount_of_pairs; i++) {
//       size_t neighbor_a = tree_neighbors[i].first;
//       size_t neighbor_b = tree_neighbors[i].second;
//       i_neighbors[neighbor_a].push_back(neighbor_b);
//       i_neighbors[neighbor_b].push_back(neighbor_a);
//     }
//
// // update neighbors
// #pragma omp parallel for
//     for (size_t i = 0; i < cells->size(); i++) {
//       (*cells)[i].SetNeighbors(i_neighbors[i]);
//     }
    delete tree;
    // delete[] i_neighbors;
  }

 private:
  double distance_ = 3000;
};

}  // namespace bdm

#endif  // NEIGHBOR_OP_H_
