#ifndef DIFFUSION_GRID_H_
#define DIFFUSION_GRID_H_

#include <assert.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <Rtypes.h>

#include "math_util.h"

namespace bdm {

using std::array;

/// A class that computes the diffusion of extracellular substances
/// It maintains the concentration and gradient of a single substance
class DiffusionGrid {
 public:
  DiffusionGrid() {}
  DiffusionGrid(int substance_id, std::string substance_name, double dc,
                double mu, int resolution)
      : substance_(substance_id),
        substance_name_(substance_name),
        dc_({{1 - dc, dc / 6, dc / 6, dc / 6, dc / 6, dc / 6, dc / 6}}),
        mu_(mu),
        resolution_(resolution) {}

  virtual ~DiffusionGrid() {}

  /// @brief      Initializes the grid by calculating the grid dimensions
  ///             and number of boxes along the axis from the input arguments
  ///
  /// @param[in]  grid_dimensions  The grid dimensions
  /// @param[in]  box_length       The box length
  ///
  void Initialize(const std::array<int32_t, 6>& grid_dimensions,
                  uint32_t box_length) {
    // Get grid properties from neighbor grid
    grid_dimensions_ = grid_dimensions;
    assert(resolution_ > 0 && "The resolution cannot be zero!");
    box_length_ = box_length / resolution_;

    assert(box_length_ > 0 &&
           "Box length of diffusion grid must be greater than zero!");

    // If the grid is not perfectly divisible along each dimension by the
    // resolution, extend the grid so that it is
    for (int i = 0; i < 3; i++) {
      int dimension_length =
          grid_dimensions_[2 * i + 1] - grid_dimensions_[2 * i];
      int r = dimension_length % box_length_;
      if (r != 0) {
        // std::abs for the case that box_length_ > dimension_length
        grid_dimensions_[2 * i + 1] += (box_length_ - r);
      }
    }

    // Calculate how many boxes fit along each dimension
    for (int i = 0; i < 3; i++) {
      int dimension_length =
          grid_dimensions_[2 * i + 1] - grid_dimensions_[2 * i];
      assert((dimension_length % box_length_ == 0) &&
             "The grid dimensions are not a multiple of its box length");
      num_boxes_axis_[i] = dimension_length / box_length_;
    }

    // Set the parity of the number of boxes along the dimensions (since all
    // dimensions are the same, we just take the x-axis here)
    parity_ = num_boxes_axis_[0] % 2;

    total_num_boxes_ =
        num_boxes_axis_[0] * num_boxes_axis_[1] * num_boxes_axis_[2];

    // Allocate memory for the concentration and gradient arrays
    c1_.resize(total_num_boxes_);
    c2_.resize(total_num_boxes_);
    gradients_.resize(3 * total_num_boxes_);

    initialized_ = true;
  }

  void RunInitializers() {
    assert(num_boxes_axis_[0] > 0 &&
           "The number of boxes along an axis was found to be zero!");
    if (initializers_.empty()) {
      return;
    }

    // Apply all functions that initialize this diffusion grid
    for (size_t f = 0; f < initializers_.size(); f++) {
      for (size_t x = 0; x < num_boxes_axis_[0]; x++) {
        double real_x = grid_dimensions_[0] + x * box_length_;
        for (size_t y = 0; y < num_boxes_axis_[1]; y++) {
          double real_y = grid_dimensions_[2] + y * box_length_;
          for (size_t z = 0; z < num_boxes_axis_[2]; z++) {
            double real_z = grid_dimensions_[4] + z * box_length_;
            std::array<uint32_t, 3> box_coord;
            box_coord[0] = x;
            box_coord[1] = y;
            box_coord[2] = z;
            size_t idx = GetBoxIndex(box_coord);
            IncreaseConcentrationBy(idx,
                                    initializers_[f](real_x, real_y, real_z));
          }
        }
      }
    }

    // Clear the initializer to free up space
    initializers_.clear();
    initializers_.shrink_to_fit();
  }

  /// @brief      Updates the grid dimensions, based on the given threshold
  ///             values. The diffusion grid dimensions need always be larger
  ///             than the neighbor grid dimensions, so that each simulation
  ///             object can obtain its local concentration / gradient
  ///
  /// @param[in]  threshold_dimensions  The threshold values
  ///
  void Update(const std::array<int32_t, 2>& threshold_dimensions) {
    // Extend the grid dimensions such that each dimension ranges from
    // {treshold_dimensions[0] - treshold_dimensions[1]}
    auto min_gd = threshold_dimensions[0];
    auto max_gd = threshold_dimensions[1];
    grid_dimensions_ = {min_gd, max_gd, min_gd, max_gd, min_gd, max_gd};

    // If the grid is not perfectly divisible along each dimension by the
    // resolution, extend the grid so that it is
    for (int i = 0; i < 3; i++) {
      int dimension_length =
          grid_dimensions_[2 * i + 1] - grid_dimensions_[2 * i];
      int r = dimension_length % box_length_;
      if (r != 0) {
        // std::abs for the case that box_length_ > dimension_length
        grid_dimensions_[2 * i + 1] += (box_length_ - r);
      }
    }

    // Store the old number of boxes along each axis for comparison
    array<size_t, 3> tmp_num_boxes_axis = num_boxes_axis_;

    // Calculate how many boxes fit along each dimension
    for (int i = 0; i < 3; i++) {
      int dimension_length =
          grid_dimensions_[2 * i + 1] - grid_dimensions_[2 * i];
      assert((dimension_length % box_length_ == 0) &&
             "The grid dimensions are not a multiple of its box length");
      num_boxes_axis_[i] = dimension_length / box_length_;
    }

    // We need to maintain the parity of the number of boxes along each
    // dimension,
    // otherwise copying of the substances to the increases grid will not be
    // symmetrically done; resulting in shifting of boxes
    // We add a box in the negative direction, because the only way the parity
    // could have changed is because of adding a box in the positive direction
    // (due to the grid not being perfectly divisible; see above)
    if (num_boxes_axis_[0] % 2 != parity_) {
      for (int i = 0; i < 3; i++) {
        grid_dimensions_[2 * i] -= box_length_;
        num_boxes_axis_[i]++;
      }
    }

    // Temporarily save previous grid data
    std::vector<double> tmp_c1 = c1_;
    std::vector<double> tmp_gradients = gradients_;

    c1_.clear();
    c2_.clear();
    gradients_.clear();

    total_num_boxes_ =
        num_boxes_axis_[0] * num_boxes_axis_[1] * num_boxes_axis_[2];

    CopyOldData(tmp_c1, tmp_gradients, tmp_num_boxes_axis);

    assert(total_num_boxes_ >= tmp_num_boxes_axis[0] * tmp_num_boxes_axis[1] *
                                   tmp_num_boxes_axis[2] &&
           "The diffusion grid tried to shrink! It can only become larger");
  }

  /// Copies the concentration and gradients values to the new
  /// (larger) grid. In the 2D case it looks like the following:
  ///
  ///                             [0 0  0  0]
  ///               [v1 v2]  -->  [0 v1 v2 0]
  ///               [v3 v4]  -->  [0 v3 v4 0]
  ///                             [0 0  0  0]
  ///
  /// The dimensions are doubled in this case from 2x2 to 4x4
  /// If the dimensions would be increased from 2x2 to 3x3, it will still
  /// be increased to 4x4 in order for GetBoxIndex to function correctly
  ///
  void CopyOldData(const std::vector<double>& old_c1,
                   const std::vector<double>& old_gradients,
                   const array<size_t, 3>& old_num_boxes_axis) {
    // Allocate more memory for the grid data arrays
    c1_.resize(total_num_boxes_);
    c2_.resize(total_num_boxes_);
    gradients_.resize(3 * total_num_boxes_);

    auto incr_dim_x = num_boxes_axis_[0] - old_num_boxes_axis[0];
    auto incr_dim_y = num_boxes_axis_[1] - old_num_boxes_axis[1];
    auto incr_dim_z = num_boxes_axis_[2] - old_num_boxes_axis[2];

    int off_x = incr_dim_x / 2;
    int off_y = incr_dim_y / 2;
    int off_z = incr_dim_z / 2;

    int num_box_xy = num_boxes_axis_[0] * num_boxes_axis_[1];
    int old_box_xy = old_num_boxes_axis[0] * old_num_boxes_axis[1];
    int new_origin = off_z * (num_boxes_axis_[0] * num_boxes_axis_[1]) +
                     off_y * num_boxes_axis_[0] + off_x;
    for (size_t k = 0; k < old_num_boxes_axis[2]; k++) {
      int offset = new_origin + k * num_box_xy;
      for (size_t j = 0; j < old_num_boxes_axis[1]; j++) {
        if (j != 0) {
          offset += num_boxes_axis_[0];
        }
        for (size_t i = 0; i < old_num_boxes_axis[0]; i++) {
          auto idx = k * old_box_xy + j * old_num_boxes_axis[0] + i;
          c1_[offset + i] = old_c1[idx];
          gradients_[3 * (offset + i)] = old_gradients[3 * idx];
          gradients_[3 * (offset + i) + 1] = old_gradients[3 * idx + 1];
          gradients_[3 * (offset + i) + 2] = old_gradients[3 * idx + 2];
        }
      }
    }
  }

  /// Solves a 5-point stencil diffusion equation, with leaking-edge
  /// boundary conditions. Substances are allowed to leave the simulation
  /// space. This prevents building up concentration at the edges
  ///
  void DiffuseWithLeakingEdge() {
    int nx = num_boxes_axis_[0];
    int ny = num_boxes_axis_[1];
    int nz = num_boxes_axis_[2];

#define YBF 16
#pragma omp parallel for collapse(2)
    for (int yy = 0; yy < ny; yy += YBF) {
      for (int z = 0; z < nz; z++) {
        // To let the edges bleed we set some diffusion coefficients
        // to zero. This prevents substance building up at the edges
        auto dc_2_ = dc_;
        int ymax = yy + YBF;
        if (ymax >= ny) {
          ymax = ny;
        }
        for (int y = yy; y < ymax; y++) {
          dc_2_ = dc_;
          int x;
          int c, n, s, b, t;
          x = 0;
          c = x + y * nx + z * nx * ny;
          if (y == 0) {
            n = c;
            dc_2_[4] = 0;
          } else {
            n = c - nx;
          }
          if (y == (ny - 1)) {
            s = c;
            dc_2_[3] = 0;
          } else {
            s = c + nx;
          }
          if (z == 0) {
            b = c;
            dc_2_[5] = 0;
          } else {
            b = c - nx * ny;
          }
          if (z == (nz - 1)) {
            t = c;
            dc_2_[6] = 0;
          } else {
            t = c + nx * ny;
          }
          // x = 0; we leak out substances past this edge (so multiply by 0)
          c2_[c] = (dc_2_[0] * c1_[c] + 0 * c1_[c] + dc_2_[2] * c1_[c + 1] +
                    dc_2_[3] * c1_[s] + dc_2_[4] * c1_[n] + dc_2_[5] * c1_[b] +
                    dc_2_[6] * c1_[t]) *
                   (1 - mu_);
#pragma omp simd
          for (x = 1; x < nx - 1; x++) {
            ++c;
            ++n;
            ++s;
            ++b;
            ++t;
            c2_[c] =
                (dc_2_[0] * c1_[c] + dc_2_[1] * c1_[c - 1] +
                 dc_2_[2] * c1_[c + 1] + dc_2_[3] * c1_[s] + dc_2_[4] * c1_[n] +
                 dc_2_[5] * c1_[b] + dc_2_[6] * c1_[t]) *
                (1 - mu_);
          }
          ++c;
          ++n;
          ++s;
          ++b;
          ++t;
          // x = nx-1; we leak out substances past this edge (so multiply by 0)
          c2_[c] = (dc_2_[0] * c1_[c] + dc_2_[1] * c1_[c - 1] + 0 * c1_[c] +
                    dc_2_[3] * c1_[s] + dc_2_[4] * c1_[n] + dc_2_[5] * c1_[b] +
                    dc_2_[6] * c1_[t]) *
                   (1 - mu_);
        }  // tile ny
      }    // tile nz
    }      // block ny
    c1_.swap(c2_);
  }

  /// Solves a 5-point stencil diffusion equation, with closed-edge
  /// boundary conditions. Substances are not allowed to leave the simulation
  /// space. Keep in mind that the concentration can build up at the edges
  ///
  void DiffuseWithClosedEdge() {
    auto nx = num_boxes_axis_[0];
    auto ny = num_boxes_axis_[1];
    auto nz = num_boxes_axis_[2];

#define YBF 16
#pragma omp parallel for collapse(2)
    for (size_t yy = 0; yy < ny; yy += YBF) {
      for (size_t z = 0; z < nz; z++) {
        size_t ymax = yy + YBF;
        if (ymax >= ny) {
          ymax = ny;
        }
        for (size_t y = yy; y < ymax; y++) {
          size_t x;
          int c, n, s, b, t;
          x = 0;
          c = x + y * nx + z * nx * ny;
          n = (y == 0) ? c : c - nx;
          s = (y == ny - 1) ? c : c + nx;
          b = (z == 0) ? c : c - nx * ny;
          t = (z == nz - 1) ? c : c + nx * ny;
          c2_[c] = (dc_[0] * c1_[c] + dc_[1] * c1_[c] + dc_[2] * c1_[c + 1] +
                    dc_[3] * c1_[s] + dc_[4] * c1_[n] + dc_[5] * c1_[b] +
                    dc_[6] * c1_[t]) *
                   (1 - mu_);
#pragma omp simd
          for (x = 1; x < nx - 1; x++) {
            ++c;
            ++n;
            ++s;
            ++b;
            ++t;
            c2_[c] = (dc_[0] * c1_[c] + dc_[1] * c1_[c - 1] +
                      dc_[2] * c1_[c + 1] + dc_[3] * c1_[s] + dc_[4] * c1_[n] +
                      dc_[5] * c1_[b] + dc_[6] * c1_[t]) *
                     (1 - mu_);
          }
          ++c;
          ++n;
          ++s;
          ++b;
          ++t;
          c2_[c] = (dc_[0] * c1_[c] + dc_[1] * c1_[c - 1] + dc_[2] * c1_[c] +
                    dc_[3] * c1_[s] + dc_[4] * c1_[n] + dc_[5] * c1_[b] +
                    dc_[6] * c1_[t]) *
                   (1 - mu_);
        }  // tile ny
      }    // tile nz
    }      // block ny
    c1_.swap(c2_);
  }

  /// Calculates the gradient for each box in the diffusion grid.
  /// The gradient is calculated in each direction (x, y, z) as following:
  ///
  /// c(x + box_length_) - c(x - box_length) / (2 * box_length_),
  ///
  /// where c(x) implies the concentration at position x
  ///
  /// At the edges the gradient is the same as the box next to it
  void CalculateGradient() {
    double gd = 1 / (static_cast<double>(box_length_) * 2);

    auto nx = num_boxes_axis_[0];
    auto ny = num_boxes_axis_[1];
    auto nz = num_boxes_axis_[2];

#pragma omp parallel for collapse(2)
    for (size_t z = 0; z < nz; z++) {
      for (size_t y = 0; y < ny; y++) {
        for (size_t x = 0; x < nx; x++) {
          int c, e, w, n, s, b, t;
          c = x + y * nx + z * nx * ny;

          if (x == 0) {
            e = c;
            w = c + 2;
          } else if (x == nx - 1) {
            e = c - 2;
            w = c;
          } else {
            e = c - 1;
            w = c + 1;
          }

          if (y == 0) {
            n = c + 2 * nx;
            s = c;
          } else if (y == ny - 1) {
            n = c;
            s = c - 2 * nx;
          } else {
            n = c + nx;
            s = c - nx;
          }

          if (z == 0) {
            t = c + 2 * nx * ny;
            b = c;
          } else if (z == nz - 1) {
            t = c;
            b = c - 2 * nx * ny;
          } else {
            t = c + nx * ny;
            b = c - nx * ny;
          }

          // Let the gradient point from low to high concentration
          gradients_[3 * c] = (c1_[w] - c1_[e]) * gd;
          gradients_[3 * c + 1] = (c1_[n] - c1_[s]) * gd;
          gradients_[3 * c + 2] = (c1_[t] - c1_[b]) * gd;
        }
      }
    }
  }

  /// Increase the concentration at specified position with specified amount
  void IncreaseConcentrationBy(const array<double, 3>& position,
                               double amount) {
    auto idx = GetBoxIndex(position);
    IncreaseConcentrationBy(idx, amount);
  }

  /// Increase the concentration at specified box with specified amount
  void IncreaseConcentrationBy(size_t idx, double amount) {
    assert(idx < total_num_boxes_ &&
           "Cell position is out of diffusion grid bounds");
    c1_[idx] += amount;
    if (c1_[idx] > concentration_threshold_) {
      c1_[idx] = concentration_threshold_;
    }
  }

  /// Get the concentration at specified position
  double GetConcentration(const array<double, 3>& position) {
    return c1_[GetBoxIndex(position)];
  }

  /// Get the (normalized) gradient at specified position
  void GetGradient(const array<double, 3>& position,
                   array<double, 3>* gradient) {
    auto idx = GetBoxIndex(position);
    assert(idx < total_num_boxes_ &&
           "Cell position is out of diffusion grid bounds");
    (*gradient)[0] = gradients_[3 * idx];
    (*gradient)[1] = gradients_[3 * idx + 1];
    (*gradient)[2] = gradients_[3 * idx + 2];
    auto norm = std::sqrt((*gradient)[0] * (*gradient)[0] +
                          (*gradient)[1] * (*gradient)[1] +
                          (*gradient)[2] * (*gradient)[2]);
    if (norm > 1e-10) {
      (*gradient)[0] /= norm;
      (*gradient)[1] /= norm;
      (*gradient)[2] /= norm;
    }
  }

  size_t GetBoxIndex(const array<uint32_t, 3>& box_coord) const {
    size_t ret = box_coord[2] * num_boxes_axis_[0] * num_boxes_axis_[1] +
                 box_coord[1] * num_boxes_axis_[0] + box_coord[0];
    return ret;
  }

  /// Calculates the box index of the substance at specified position
  size_t GetBoxIndex(const array<double, 3>& position) const {
    array<uint32_t, 3> box_coord;
    box_coord[0] = (floor(position[0]) - grid_dimensions_[0]) / box_length_;
    box_coord[1] = (floor(position[1]) - grid_dimensions_[2]) / box_length_;
    box_coord[2] = (floor(position[2]) - grid_dimensions_[4]) / box_length_;

    return GetBoxIndex(box_coord);
  }

  void SetDecayConstant(double mu) { mu_ = mu; }

  void SetConcentrationThreshold(double t) { concentration_threshold_ = t; }

  double GetConcentrationThreshold() { return concentration_threshold_; }

  double* GetAllConcentrations() { return c1_.data(); }

  double* GetAllGradients() { return gradients_.data(); }

  const array<size_t, 3>& GetNumBoxesArray() { return num_boxes_axis_; }

  size_t GetNumBoxes() { return total_num_boxes_; }

  int GetBoxLength() { return box_length_; }

  int GetSubstanceId() { return substance_; }

  std::string GetSubstanceName() { return substance_name_; }

  double GetDecayConstant() { return mu_; }

  int32_t* GetDimensionsPtr() { return grid_dimensions_.data(); }

  array<int32_t, 6>& GetDimensions() { return grid_dimensions_; }

  array<double, 7>& GetDiffusionCoefficients() { return dc_; }

  bool IsInitialized() { return initialized_; }

  int GetResolution() { return resolution_; }

  // void AddGaussianLayer(double mean, double sigma, int selected) {
  //   mean_.push_back(mean);
  //   sigma_.push_back(sigma);
  //   selected_.push_back(selected);
  // }

  template <typename F>
  void AddInitializer(F function) {
    initializers_.push_back(function);
  }

 private:
  /// The id of the substance of this grid
  int substance_;
  /// The name of the substance of this grid
  std::string substance_name_;
  /// The length of a box
  uint32_t box_length_;
  /// The array of concentration values
  std::vector<double> c1_;
  /// An extra concentration data buffer for faster value updating
  std::vector<double> c2_;
  /// The array of gradients (x, y, z)
  std::vector<double> gradients_;
  /// The maximum concentration value that a box can have
  double concentration_threshold_ = 1;
  /// The diffusion coefficients [cc, cw, ce, cs, cn, cb, ct]
  array<double, 7> dc_;
  /// The decay constant
  double mu_ = 0;
  /// The grid dimensions of the diffusion grid
  array<int32_t, 6> grid_dimensions_;
  /// The number of boxes at each axis [x, y, z]
  array<size_t, 3> num_boxes_axis_ = {{0}};
  /// The total number of boxes in the diffusion grid
  size_t total_num_boxes_ = 0;
  /// Flag to determine if this grid has been initialized
  bool initialized_ = false;
  /// The resolution of the diffusion grid
  int resolution_ = 1;
  /// If false, grid dimensions are even; if true, they are odd
  bool parity_ = false;
  /// A list of functions that initialize this diffusion grid
  std::vector<std::function<double(double, double, double)>> initializers_;

  ClassDefNV(DiffusionGrid, 1);
};

}  // namespace bdm

#endif  // DIFFUSION_GRID_H_
