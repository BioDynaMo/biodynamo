#ifndef SUBSTANCE_INITIALIZERS_H_
#define SUBSTANCE_INITIALIZERS_H_

#include <vector>

#include "Math/DistFunc.h"

namespace bdm {

struct GaussianBand {
  double mean_;
  double sigma_;
  std::vector<double> data_;
  bool initialized_ = false;

  // Store the model-related values
  GaussianBand(double mean, double sigma) {
    mean_ = mean;
    sigma_ = sigma;
  }

  // Returns the value of gaussian data at specified index
  double operator()(int length, double min, uint32_t bl, size_t idx) {
    if(!initialized_) {
      data_.resize(length);
      for (int i = 0; i < length; i++) {
        data_[i] = ROOT::Math::normal_pdf(min + i * bl, sigma_, mean_);
      }
      initialized_ = true;
    }
    return data_[idx];
  }
};

struct PoissonBand {
  double lambda_;
  std::vector<double> data_;
  bool initialized_ = false;

  // Store the model-related values
  PoissonBand(double lambda) {
    lambda_ = lambda;
  }

  // Returns the value of gaussian data at specified index
  double operator()(int length, double min, uint32_t bl, size_t idx) {
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
