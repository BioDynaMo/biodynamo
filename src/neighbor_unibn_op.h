#ifndef NEIGHBOR_UNIBN_OP_H_
#define NEIGHBOR_UNIBN_OP_H_

#include <fstream>
#include <cmath>
#include <utility>
#include <vector>
#include <chrono>

#include "../third_party/unibn_octree.h"
#include "inline_vector.h"

using std::ofstream;

namespace bdm {

class NeighborUnibnOp {
 public:
  NeighborUnibnOp() {}
  explicit NeighborUnibnOp(double distance) : distance_(distance) {}
  ~NeighborUnibnOp() {}

  template <typename TContainer>
  void Compute(TContainer* cells) const {
    ofstream outfile;
    outfile.open("NeighborUnibnOp.txt", std::ofstream::out | std::ofstream::app);

    unibn::OctreeParams params;
    params.bucketSize = 16;

    unibn::Octree<std::array<double, 3> > octree;

    std::chrono::steady_clock::time_point begin_build = std::chrono::steady_clock::now();
    octree.initialize(cells->GetAllPositions(), params);
    std::chrono::steady_clock::time_point end_build = std::chrono::steady_clock::now();

    // std::cout << "\n[UNIBN] Octree build time = " << std::chrono::duration_cast<std::chrono::microseconds>(end_build - begin_build).count() << "us\n";
    outfile << cells->size() << ",";
    outfile << std::chrono::duration_cast<std::chrono::microseconds>(end_build - begin_build).count() << ",";

// calc neighbors
std::chrono::microseconds totalTime{0};
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      // fixme make param
      // according to roman 50 - 100 micron
      double search_radius = sqrt(distance_);

      // holds the indices of found neighbors
      std::vector<uint32_t> found_neighbors;

      const auto& query = (*cells)[i].GetPosition();

      // calculate neighbors
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      octree.radiusNeighbors<unibn::L2Distance<std::array<double, 3> > >(query, search_radius, found_neighbors);
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

      // transform result
      InlineVector<int, 8> neighbors;
      neighbors.reserve(found_neighbors.size() - 1);
      for (int j = 0; j < found_neighbors.size(); j++) {
        if (found_neighbors[j] != i) {
          neighbors.push_back(found_neighbors[j]);
        }
      }
      (*cells)[i].SetNeighbors(neighbors);
    }

    // std::cout << "\n[UNIBN] Neighbor search time = " << totalTime.count() << "us\n";
    outfile << totalTime.count() << ",";
    outfile.close();
  }

 private:
  double distance_ = 3000;
};

}  // namespace bdm

#endif  // NEIGHBOR_UNIBN_OP_H_
