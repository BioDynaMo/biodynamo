#ifndef NEIGHBOR_NANOFLANN_OP_H_
#define NEIGHBOR_NANOFLANN_OP_H_

#include <fstream>
#include <utility>
#include <vector>
#include <chrono>
#include "inline_vector.h"
#include "nanoflann.h"

using std::ofstream;

namespace bdm {

using nanoflann::KDTreeSingleIndexAdaptorParams;
using nanoflann::L2_Simple_Adaptor;
using nanoflann::KDTreeSingleIndexAdaptor;

// https://github.com/jlblancoc/nanoflann/blob/master/examples/pointcloud_adaptor_example.cpp
template <typename Derived>
struct NanoFlannAdapter {
  using coord_t = double;

  const Derived& obj;  //!< A const ref to the data set origin

  /// The constructor that sets the data set source
  explicit NanoFlannAdapter(const Derived& obj_) : obj(obj_) {}

  /// CRTP helper method
  inline const Derived& derived() const { return obj; }

  /// Must return the number of data points
  inline size_t kdtree_get_point_count() const { return derived().size(); }

  /// Returns the distance between the vector "p1[0:size-1]" and the data point
  /// with index "idx_p2" stored in the class:
  inline coord_t kdtree_distance(const coord_t* p1, const size_t idx_p2,
                                 size_t /*size*/) const {
    const coord_t d0 = p1[0] - derived()[idx_p2].GetPosition()[0];
    const coord_t d1 = p1[1] - derived()[idx_p2].GetPosition()[1];
    const coord_t d2 = p1[2] - derived()[idx_p2].GetPosition()[2];
    return d0 * d0 + d1 * d1 + d2 * d2;
  }

  /// Returns the dim'th component of the idx'th point in the class:
  /// Since this is inlined and the "dim" argument is typically an immediate
  /// value, the "if/else's" are actually solved at compile time.
  inline coord_t kdtree_get_pt(const size_t idx, int dim) const {
    return derived()[idx].GetPosition()[dim];
  }

  /// Optional bounding-box computation: return false to default to a standard
  /// bbox computation loop.
  ///   Return true if the BBOX was already computed by the class and returned
  ///   in
  ///   "bb" so it can be avoided to redo it again.
  ///   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3
  ///   for point clouds)
  template <class BBOX>
  bool kdtree_get_bbox(BBOX&) const {
    return false;
  }
};

class NeighborNanoflannOp {
 public:
  NeighborNanoflannOp() {}
  explicit NeighborNanoflannOp(double distance) : distance_(distance) {}
  ~NeighborNanoflannOp() {}

  template <typename TContainer>
  void Compute(TContainer* cells) const {
    ofstream outfile;
    outfile.open("NeighborNanoflannOp.txt", std::ofstream::out | std::ofstream::app);

    typedef NanoFlannAdapter<TContainer> Adapter;
    const Adapter nf_cells(*cells);  // The adaptor

    // construct a 3D kd-tree index:
    typedef KDTreeSingleIndexAdaptor<L2_Simple_Adaptor<double, Adapter>,
                                     Adapter, 3>
        my_kd_tree_t;

    // three dimensions; max leafs: 10
    my_kd_tree_t index(3, nf_cells, KDTreeSingleIndexAdaptorParams(10));

    std::chrono::steady_clock::time_point begin_build = std::chrono::steady_clock::now();
    index.buildIndex();
    std::chrono::steady_clock::time_point end_build = std::chrono::steady_clock::now();

    // std::cout << "\n[NanoFlann] KD-Tree build time = " << std::chrono::duration_cast<std::chrono::microseconds>(end_build - begin_build).count() << "us\n";
    outfile << cells->size() << ",";
    outfile << std::chrono::duration_cast<std::chrono::microseconds>(end_build - begin_build).count() << ",";

// calc neighbors
std::chrono::microseconds totalTime{0};
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      // fixme make param
      // according to roman 50 - 100 micron
      double search_radius = distance_;

      std::vector<std::pair<size_t, double>> ret_matches;

      nanoflann::SearchParams params;
      params.sorted = false;

      const auto& position = (*cells)[i].GetPosition();

      // calculate neighbors
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      const size_t n_matches =
          index.radiusSearch(&position[0], search_radius, ret_matches, params);
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

      // transform result (change data structure - remove self from list)
      InlineVector<int, 8> neighbors;
      neighbors.reserve(n_matches - 1);
      for (size_t j = 0; j < n_matches; j++) {
        if (ret_matches[j].first != i) {
          neighbors.push_back(ret_matches[j].first);
        }
      }
      (*cells)[i].SetNeighbors(neighbors);
    }

    // std::cout << "\n[NanoFlann] Neighbor search time = " << totalTime.count() << "us\n";
    outfile << totalTime.count() << ",";
    outfile.close();
  }

 private:
  double distance_ = 3000;
};

}  // namespace bdm

#endif  // NEIGHBOR_NANOFLANN_OP_H_
