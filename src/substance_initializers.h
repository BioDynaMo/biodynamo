#ifndef SUBSTANCE_INITIALIZERS_H_
#define SUBSTANCE_INITIALIZERS_H_

#include <stdexcept>
#include <vector>

#include "Math/DistFunc.h"

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
  double operator()(size_t length, double min, uint32_t bl, size_t x, size_t y, size_t z) {
    if(!initialized_) {
      data_.resize(length);
      for (size_t i = 0; i < length; i++) {
        data_[i] = ROOT::Math::normal_pdf(min + i * bl, sigma_, mean_);
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
  PoissonBand(double lambda) {
    lambda_ = lambda;
  }

  // Returns the value of gaussian data at specified index
  double operator()(int length, double min, uint32_t bl, size_t idx, size_t x, size_t y, size_t z) {
    if(!initialized_) {
      data_.resize(length);
      for (int i = 0; i < length; i++) {
        data_[i] = ROOT::Math::poisson_pdf(min + i * bl, lambda_);
      }
      initialized_ = true;
    }
    return data_[idx];
  }
};

}  // namespace bdm

#endif  // SUBSTANCE_INITIALIZERS_H_
