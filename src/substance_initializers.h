#ifndef SUBSTANCE_INITIALIZERS_H_
#define SUBSTANCE_INITIALIZERS_H_

#include <stdexcept>
#include <vector>

#include "Math/DistFunc.h"

#include "diffusion_grid.h"

namespace bdm {

enum Axis {kXAxis, kYAxis, kZAxis};

struct GaussianBand {
  double mean_;
  double sigma_;
  uint8_t axis_;
  std::vector<double> data_;
  bool initialized_ = false;

  // Store the model-related values
  GaussianBand(double mean, double sigma, uint8_t axis) {
    mean_ = mean;
    sigma_ = sigma;
    axis_ = axis;
  }

  // Returns the value of gaussian data at specified index
  double operator()(DiffusionGrid* dgrid, size_t x, size_t y, size_t z) {
    if(!initialized_) {
      auto length = dgrid->GetNumBoxesArray()[0];
      auto box_length = dgrid->GetBoxLength();
      auto min = dgrid->GetDimensions()[0];

      data_.resize(length);
      for (size_t i = 0; i < length; i++) {
        data_[i] = ROOT::Math::normal_pdf(min + i * box_length, sigma_, mean_);
      }
      initialized_ = true;
    }

    switch(axis_) {
      case Axis::kXAxis: return data_[x];
      case Axis::kYAxis: return data_[y];
      case Axis::kZAxis: return data_[z];
      default: throw std::logic_error("You have chosen an non-existing axis!"); 
    }
  }
};

struct PoissonBand {
  double lambda_;
  uint8_t axis_;
  std::vector<double> data_;
  bool initialized_ = false;

  // Store the model-related values
  PoissonBand(double lambda, uint8_t axis) {
    lambda_ = lambda;
    axis_ = axis;
  }

  // Returns the value of gaussian data at specified index
  double operator()(DiffusionGrid* dgrid, size_t x, size_t y, size_t z) {
    if(!initialized_) {
      auto length = dgrid->GetNumBoxesArray()[0];
      auto box_length = dgrid->GetBoxLength();
      auto min = dgrid->GetDimensions()[0];

      data_.resize(length);
      for (size_t i = 0; i < length; i++) {
        data_[i] = ROOT::Math::poisson_pdf(min + i * box_length, lambda_);
      }
      initialized_ = true;
    }
    switch(axis_) {
      case Axis::kXAxis: return data_[x];
      case Axis::kYAxis: return data_[y];
      case Axis::kZAxis: return data_[z];
      default: throw std::logic_error("You have chosen an non-existing axis!"); 
    }
  }
};

}  // namespace bdm

#endif  // SUBSTANCE_INITIALIZERS_H_
