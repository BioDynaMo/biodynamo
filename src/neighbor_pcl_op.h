#ifndef NEIGHBOR_PCL_OP_H_
#define NEIGHBOR_PCL_OP_H_

#include <pcl/octree/octree_search.h>

#include <fstream>
#include <cmath>
#include <utility>
#include <vector>
#include <chrono>

#include "inline_vector.h"

using std::ofstream;

namespace bdm {

using pcl::PointCloud;
using pcl::PointXYZ;
using pcl::octree::OctreePointCloudSearch;

class NeighborPclOp {
 public:
  NeighborPclOp() {}
  explicit NeighborPclOp(double distance) : distance_(distance) {}
  ~NeighborPclOp() {}

  template <typename TContainer>
  void Compute(TContainer* cells, const char* filename = nullptr) const {
    ofstream outfile;
    if (filename != nullptr) {
      outfile.open(filename, std::ofstream::out | std::ofstream::app);
      outfile << cells->size() << ",";
    }

    PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);

    // set point cloud dimensions
    cloud->width = cells->size();
    cloud->height = 1;
    cloud->points.resize (cloud->width * cloud->height);

    // fill point cloud data
    // todo assign whole array at once
    for (size_t i = 0; i < cloud->points.size (); ++i)
    {
      const auto& position = (*cells)[i].GetPosition();
      cloud->points[i].x = position[0];
      cloud->points[i].y = position[1];
      cloud->points[i].z = position[2];
    }

    // todo find out what this really means
    float resolution = 128.0f;

    OctreePointCloudSearch<PointXYZ> octree(resolution);
    octree.setInputCloud(cloud);

    std::chrono::steady_clock::time_point begin_build = std::chrono::steady_clock::now();
    octree.addPointsFromInputCloud();
    std::chrono::steady_clock::time_point end_build = std::chrono::steady_clock::now();

    if (print_terminal == 1) {
      std::cout << "===================[PCL]=====================" << std::endl;
      std::cout << "Octree build time    = " << std::chrono::duration_cast<std::chrono::milliseconds>(end_build - begin_build).count() << "ms\n";
    }
    if (filename != nullptr) {
      outfile << std::chrono::duration_cast<std::chrono::microseconds>(end_build - begin_build).count() << ",";
    }

// calc neighbors
std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
int avg_num_neighbors = 0;
#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(cells->size()); i++) {
      // fixme make param
      // according to roman 50 - 100 micron
      double search_radius = sqrt(distance_);

      // holds the indices of found neighbors
      std::vector<int> pointIdxRadiusSearch;
      // holds the distances of found neighbors from reference cell
      std::vector<float> pointRadiusSquaredDistance;

      // calculate neighbors
      const int n_matches =
          octree.radiusSearch(cloud->points[i], search_radius, pointIdxRadiusSearch, pointRadiusSquaredDistance);
      avg_num_neighbors += n_matches;

      // transform result
      InlineVector<int, 8> neighbors;
      neighbors.reserve(n_matches - 1);
      for (int j = 0; j < n_matches; j++) {
        if (pointIdxRadiusSearch[j] != i) {
          neighbors.push_back(pointIdxRadiusSearch[j]);
        }
      }
      (*cells)[i].SetNeighbors(neighbors);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    if (print_terminal == 1) {
      std::cout << "Neighbor search time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms\n";
      // std::cout << "# of neighbors found = " << (avg_num_neighbors/(cells->size())) << std::endl;
    }
    if (filename != nullptr) {
      outfile << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << ",";
      outfile.close();
    }
  }

 private:
  double distance_ = 3000;
};

}  // namespace bdm

#endif  // NEIGHBOR_NANOFLANN_OP_H_
