#ifndef NEIGHBOR_OP_H_
#define NEIGHBOR_OP_H_

#include <iostream>
#include "nanoflann.h"
#include "make_thread_safe.h"

namespace bdm {

using nanoflann::KDTreeSingleIndexAdaptorParams;
using nanoflann::L2_Simple_Adaptor;
using nanoflann::KDTreeSingleIndexAdaptor;

// https://github.com/jlblancoc/nanoflann/blob/master/examples/pointcloud_adaptor_example.cpp
// And this is the "dataset to kd-tree" adaptor class:
template <typename Derived>
struct NanoFlannDaosoaAdapter {
  using coord_t = VcVectorBackend::real_t;

  const Derived& obj;  //!< A const ref to the data set origin

  /// The constructor that sets the data set source
  NanoFlannDaosoaAdapter(const Derived& obj_) : obj(obj_) {}

  /// CRTP helper method
  inline const Derived& derived() const { return obj; }

  /// Must return the number of data points
  inline size_t kdtree_get_point_count() const { return derived().Elements(); }

  /// Returns the distance between the vector "p1[0:size-1]" and the data point
  /// with index "idx_p2" stored in the class:
  inline coord_t kdtree_distance(const coord_t* p1, const size_t idx_p2,
                                 size_t /*size*/) const {
    const auto vector_idx = idx_p2 / VcVectorBackend::kVecLen;
    const auto scalar_idx = idx_p2 % VcVectorBackend::kVecLen;
    const coord_t d0 =
        p1[0] - derived()[vector_idx].GetPosition()[0][scalar_idx];
    const coord_t d1 =
        p1[1] - derived()[vector_idx].GetPosition()[1][scalar_idx];
    const coord_t d2 =
        p1[2] - derived()[vector_idx].GetPosition()[2][scalar_idx];
    return d0 * d0 + d1 * d1 + d2 * d2;
  }

  /// Returns the dim'th component of the idx'th point in the class:
  /// Since this is inlined and the "dim" argument is typically an immediate
  /// value, the "if/else's" are actually solved at compile time.
  inline coord_t kdtree_get_pt(const size_t idx, int dim) const {
    const auto vector_idx = idx / VcVectorBackend::kVecLen;
    const auto scalar_idx = idx % VcVectorBackend::kVecLen;
    return derived()[vector_idx].GetPosition()[dim][scalar_idx];
  }

  /// Optional bounding-box computation: return false to default to a standard
  /// bbox computation loop.
  ///   Return true if the BBOX was already computed by the class and returned
  ///   in
  ///   "bb" so it can be avoided to redo it again.
  ///   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3
  ///   for point clouds)
  template <class BBOX>
  bool kdtree_get_bbox(BBOX& /*bb*/) const {
    return false;
  }
};

class NeighborOp {
 public:
  NeighborOp() {}
  NeighborOp(double distance) : distance_(distance) {}
  ~NeighborOp() {}
  NeighborOp(const NeighborOp&) = delete;
  NeighborOp& operator=(const NeighborOp&) = delete;

  template <typename TContainer>
  Vc_ALWAYS_INLINE void Compute(TContainer* cells) const {
    typedef NanoFlannDaosoaAdapter<TContainer> NanoFlann2Daosoa;
    const NanoFlann2Daosoa nf_cells(*cells);  // The adaptor

    // construct a kd-tree index:
    typedef KDTreeSingleIndexAdaptor<
        L2_Simple_Adaptor<VcVectorBackend::real_t, NanoFlann2Daosoa>,
        NanoFlann2Daosoa, 3 /* dim */
        >
        my_kd_tree_t;

    // three dimensions; max leafs: 10
    my_kd_tree_t index(3, nf_cells, KDTreeSingleIndexAdaptorParams(10));
    index.buildIndex();

    std::vector<std::array<InlineVector<int, 8>, VcVectorBackend::kVecLen> >
        neighbors(cells->Vectors());

// calc neighbors
#pragma omp parallel
    {
      auto thread_safe_cells = make_thread_safe(cells);
#pragma omp for
      for (size_t i = 0; i < thread_safe_cells->Elements(); i++) {
        const auto vector_idx = i / VcVectorBackend::kVecLen;
        const auto scalar_idx = i % VcVectorBackend::kVecLen;

        // fixme make param
        // according to roman 50 - 100 micron
        const VcVectorBackend::real_t search_radius =
            static_cast<VcVectorBackend::real_t>(distance_);

        std::vector<std::pair<size_t, VcVectorBackend::real_t> > ret_matches;

        nanoflann::SearchParams params;
        params.sorted = false;

        const auto& position = (*thread_safe_cells)[vector_idx].GetPosition();
        const VcVectorBackend::real_t query_pt[3] = {position[0][scalar_idx],
                                               position[1][scalar_idx],
                                               position[2][scalar_idx]};

        // calculate neighbors
        const size_t n_matches = index.radiusSearch(&query_pt[0], search_radius,
                                                    ret_matches, params);

        // transform result (change data structure - remove self from list)
        InlineVector<int, 8> i_neighbors;
        i_neighbors.reserve(n_matches - 1);
        for (size_t j = 0; j < n_matches; j++) {
          if (ret_matches[j].first != i) {
            i_neighbors.push_back(ret_matches[j].first);
          }
        }
        neighbors[vector_idx][scalar_idx] = std::move(i_neighbors);
      }

// update neighbors
#pragma omp for
      for (size_t i = 0; i < thread_safe_cells->Vectors(); i++) {
        (*thread_safe_cells)[i].SetNeighbors(neighbors[i]);
      }
    }
  }

 private:
  double distance_ = 3000;
};

}  // namespace bdm

#endif  // NEIGHBOR_OP_H_
