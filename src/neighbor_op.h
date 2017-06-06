#ifndef NEIGHBOR_OP_H_
#define NEIGHBOR_OP_H_

#include <fstream>
#include <cmath>
#include <chrono>
#include <iostream>
#include "inline_vector.h"
#include "spatial_organization/octree_node.h"

namespace bdm {

using std::ofstream;
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
  void Compute(TContainer* cells, const char* filename = nullptr) const {

    ofstream outfile;
    if (filename != nullptr) {
      outfile.open(filename, std::ofstream::out | std::ofstream::app);
    }
    // Tree search
    // Creating a spatial tree (Bound, max_depth_, capacity_of_leaf_node)
    // IMPORTANT! Ensure that your bound is big enough and enclose all parts of
    // simulation
    SpatialTreeNode<size_t>* tree = new OctreeNode<size_t>(
        Bound(-10000.0, -10000.0, -10000.0, 10000.0, 10000.0, 10000.0), 100,
        10);

    // Initializing tree with a objects
    std::chrono::steady_clock::time_point begin_build = std::chrono::steady_clock::now();
    for (size_t i = 0; i < cells->size(); i++) {
      auto&& cell = (*cells)[i];
      const auto& position = cell.GetPosition();
      Point pos(position[0], position[1], position[2]);
      tree->Put(pos, i);
    }
    std::chrono::steady_clock::time_point end_build = std::chrono::steady_clock::now();

    if (print_terminal == 1) {
      std::cout << "=================[Russian]===================" << std::endl;
      std::cout << "Octree build time    = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_build - begin_build).count() << "ms\n";
    }
    if (filename != nullptr) {
      outfile << cells->size() << ",";
      outfile << std::chrono::duration_cast<std::chrono::microseconds>(end_build - begin_build).count() << ",";
    }

    double search_radius = sqrt(distance_);

    // Container for neighbors
    InlineVector<int, 8>* i_neighbors = new InlineVector<int, 8>[ cells->size() ];

    // Getting all pairs within the 'search_radius distance'
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto tree_neighbors = tree->GetNeighbors(search_radius);
    std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
    if (print_terminal == 1) {
      std::cout << "Neighbor search time = " 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
                << "ms\n";
    }
    if (filename != nullptr) {
      outfile << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << ",";
      outfile.close();
    }

    int amount_of_pairs = tree_neighbors.size();

    // Filling container for neighbors for every object
    // pragma omp parallel for
    for (int i = 0; i < amount_of_pairs; i++) {
      size_t neighbor_a = tree_neighbors[i].first;
      size_t neighbor_b = tree_neighbors[i].second;
      i_neighbors[neighbor_a].push_back(neighbor_b);
      i_neighbors[neighbor_b].push_back(neighbor_a);
    }

// update neighbors
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      (*cells)[i].SetNeighbors(i_neighbors[i]);
    }
    delete tree;
    delete[] i_neighbors;
  }

 private:
  double distance_ = 3000;
};

}  // namespace bdm

#endif  // NEIGHBOR_OP_H_
