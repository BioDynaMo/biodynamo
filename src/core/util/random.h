// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_UTIL_RANDOM_H_
#define CORE_UTIL_RANDOM_H_

#include <unordered_map>
#include "core/container/fixed_size_vector.h"
#include "core/container/math_array.h"
#include "core/util/root.h"

class TRandom;
class TF1;
class TF2;
class TF3;

namespace bdm {

// -----------------------------------------------------------------------------
/// Random number generator that generates samples from a distribution
template <typename TSample>
class DistributionRng {
 public:
  DistributionRng() {}
  DistributionRng(TRootIOCtor*) {}
  virtual ~DistributionRng() {}
  /// Draws a sample from the distribution
  TSample Sample();
  /// Draws two samples from the distribution.
  /// For 1D distributions this function calls `Sample()` twice.
  MathArray<TSample, 2> Sample2();
  /// Draws three samples from the distribution.
  /// For 1D and 2D distributions this function calls `Sample()` three times.
  MathArray<TSample, 3> Sample3();

  /// Returns an array of samples.
  /// Calls `Sample()` N times.
  template <uint64_t N>
  MathArray<TSample, N> SampleArray() {
    MathArray<TSample, N> ret;
    for (uint64_t i = 0; i < N; i++) {
      ret[i] = Sample();
    }
    return ret;
  }

 protected:
  virtual TSample SampleImpl(TRandom* rng) = 0;
  virtual MathArray<TSample, 2> Sample2Impl(TRandom* rng);
  virtual MathArray<TSample, 3> Sample3Impl(TRandom* rng);
  BDM_CLASS_DEF(DistributionRng, 1);
};

// -----------------------------------------------------------------------------
class UniformRng : public DistributionRng<double> {
 public:
  UniformRng(double min, double max);
  virtual ~UniformRng();

 private:
  double min_, max_;
  double SampleImpl(TRandom* rng) override;
  BDM_CLASS_DEF_OVERRIDE(UniformRng, 1);
};

// -----------------------------------------------------------------------------
class GausRng : public DistributionRng<double> {
 public:
  GausRng(double mean, double sigma);
  virtual ~GausRng();

 private:
  double mean_, sigma_;
  double SampleImpl(TRandom* rng) override;
  BDM_CLASS_DEF_OVERRIDE(GausRng, 1);
};

// -----------------------------------------------------------------------------
class ExpRng : public DistributionRng<double> {
 public:
  ExpRng(double tau);
  virtual ~ExpRng();

 private:
  double tau_;
  double SampleImpl(TRandom* rng) override;
  BDM_CLASS_DEF_OVERRIDE(ExpRng, 1);
};

// -----------------------------------------------------------------------------
class LandauRng : public DistributionRng<double> {
 public:
  LandauRng(double mean, double sigma);
  virtual ~LandauRng();

 private:
  double mean_, sigma_;
  double SampleImpl(TRandom* rng) override;
  BDM_CLASS_DEF_OVERRIDE(LandauRng, 1);
};

// -----------------------------------------------------------------------------
class PoissonDRng : public DistributionRng<double> {
 public:
  PoissonDRng(double mean);
  virtual ~PoissonDRng();

 private:
  double mean_;
  double SampleImpl(TRandom* rng) override;
  BDM_CLASS_DEF_OVERRIDE(PoissonDRng, 1);
};

// -----------------------------------------------------------------------------
class BreitWignerRng : public DistributionRng<double> {
 public:
  BreitWignerRng(double mean, double gamma);
  virtual ~BreitWignerRng();

 private:
  double mean_, gamma_;
  double SampleImpl(TRandom* rng) override;
  BDM_CLASS_DEF_OVERRIDE(BreitWignerRng, 1);
};

// -----------------------------------------------------------------------------
class UserDefinedDistRng1D : public DistributionRng<double> {
 public:
  UserDefinedDistRng1D(TRootIOCtor* ioctor) : DistributionRng<double>(ioctor) {}
  UserDefinedDistRng1D(TF1* function, const char* option);
  virtual ~UserDefinedDistRng1D();
  void Draw(const char* option = "");
  TF1* GetTF1();

 private:
  double SampleImpl(TRandom* rng) override;
  // TODO use shared_ptr once ROOT supports IO of them
  TF1* function_ = nullptr;
  const char* option_ = nullptr;
  BDM_CLASS_DEF_OVERRIDE(UserDefinedDistRng1D, 1);
};

// -----------------------------------------------------------------------------
class UserDefinedDistRng2D : public DistributionRng<double> {
 public:
  UserDefinedDistRng2D(TRootIOCtor* ioctor) : DistributionRng<double>(ioctor) {}
  UserDefinedDistRng2D(TF2* function, const char* option);
  virtual ~UserDefinedDistRng2D();
  void Draw(const char* option = "");
  TF2* GetTF2();

 private:
  double SampleImpl(TRandom* rng) override;
  MathArray<double, 2> Sample2Impl(TRandom* rng) override;
  // TODO use shared_ptr once ROOT supports IO of them
  TF2* function_ = nullptr;
  const char* option_ = nullptr;
  BDM_CLASS_DEF_OVERRIDE(UserDefinedDistRng2D, 1);
};

// -----------------------------------------------------------------------------
class UserDefinedDistRng3D : public DistributionRng<double> {
 public:
  UserDefinedDistRng3D(TRootIOCtor* ioctor) : DistributionRng<double>(ioctor) {}
  UserDefinedDistRng3D(TF3* function, const char* option);
  virtual ~UserDefinedDistRng3D();
  void Draw(const char* option = "");
  TF3* GetTF3();

 private:
  double SampleImpl(TRandom* rng) override;
  MathArray<double, 2> Sample2Impl(TRandom* rng) override;
  MathArray<double, 3> Sample3Impl(TRandom* rng) override;
  // TODO use shared_ptr once ROOT supports IO of them
  TF3* function_ = nullptr;
  const char* option_ = nullptr;
  BDM_CLASS_DEF_OVERRIDE(UserDefinedDistRng3D, 1);
};

// -----------------------------------------------------------------------------
struct UserDefinedDist {
  double (*ud_function)(const double*, const double*) = nullptr;
  FixedSizeVector<double, 10> parameters;
  double xmin = 0;
  double xmax = 0;
  double ymin = 0;
  double ymax = 0;
  double zmin = 0;
  double zmax = 0;
  bool operator==(const UserDefinedDist& other) const {
    return (ud_function == other.ud_function) &&
           (parameters == other.parameters) && (xmin == other.xmin) &&
           (xmax == other.xmax) && (ymin == other.ymin) &&
           (ymax == other.ymax) && (zmin == other.zmin) && (zmax == other.zmax);
  }
};

}  // namespace bdm
namespace std {

template <>
struct hash<bdm::UserDefinedDist> {
  std::size_t operator()(const bdm::UserDefinedDist& uddist) const noexcept {
    std::size_t h = 7;
    h *= 31 * reinterpret_cast<std::size_t>(uddist.ud_function);
    for (auto el : uddist.parameters) {
      h *= 31 * std::size_t(el * 1000);
    }
    h *= 31 * std::size_t(uddist.xmin * 1000);
    h *= 31 * std::size_t(uddist.xmax * 1000);
    h *= 31 * std::size_t(uddist.ymin * 1000);
    h *= 31 * std::size_t(uddist.ymax * 1000);
    h *= 31 * std::size_t(uddist.zmin * 1000);
    h *= 31 * std::size_t(uddist.zmax * 1000);
    return h;
  }
};

}  // namespace std
namespace bdm {

// -----------------------------------------------------------------------------
class BinomialRng : public DistributionRng<int> {
 public:
  BinomialRng(int ntot, double prob);
  virtual ~BinomialRng();

 private:
  int ntot_;
  double prob_;
  int SampleImpl(TRandom* rng) override;
  BDM_CLASS_DEF_OVERRIDE(BinomialRng, 1);
};

// -----------------------------------------------------------------------------
class PoissonRng : public DistributionRng<int> {
 public:
  PoissonRng(double mean);
  virtual ~PoissonRng();

 private:
  double mean_;
  int SampleImpl(TRandom* rng) override;
  BDM_CLASS_DEF_OVERRIDE(PoissonRng, 1);
};

// -----------------------------------------------------------------------------
/// Decorator for ROOT's TRandom
/// Uses TRandom3 as default random number generator
/// \see https://root.cern/doc/master/classTRandom.html
class Random {
 public:
  Random();
  explicit Random(TRootIOCtor*);
  Random(const Random& other);
  ~Random();
  Random& operator=(const Random& other);

  /// Forwards call to ROOT's `TRandom`.\n
  /// Returns a uniform deviate on the interval (0, max).
  /// \see https://root.cern/doc/master/classTRandom.html
  double Uniform(double max = 1.0);
  /// Forwards call to ROOT's `TRandom`.\n
  /// Returns a uniform deviate on the interval (min, max).
  /// \see https://root.cern/doc/master/classTRandom.html
  double Uniform(double min, double max);

  /// Returns an array of uniform random numbers in the interval (0, max)
  template <uint64_t N>
  MathArray<double, N> UniformArray(double max = 1.0) {
    MathArray<double, N> ret;
    for (uint64_t i = 0; i < N; i++) {
      ret[i] = Uniform(max);
    }
    return ret;
  }

  /// Returns an array of uniform random numbers in the interval (min, max)
  template <uint64_t N>
  MathArray<double, N> UniformArray(double min, double max) {
    MathArray<double, N> ret;
    for (uint64_t i = 0; i < N; i++) {
      ret[i] = Uniform(min, max);
    }
    return ret;
  }

  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  double Gaus(double mean = 0.0, double sigma = 1.0);
  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  double Exp(double tau);
  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  double Landau(double mean = 0, double sigma = 1);
  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  double PoissonD(double mean);
  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  double BreitWigner(double mean = 0, double gamma = 1);

  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  unsigned Integer(int max);
  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  int Binomial(int ntot, double prob);
  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  int Poisson(double mean);

  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  MathArray<double, 2> Circle(double radius);
  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  MathArray<double, 3> Sphere(double radius);

  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  void SetSeed(uint64_t seed);

  /// Forwards call to ROOT's `TRandom`.\n
  /// \see https://root.cern/doc/master/classTRandom.html
  uint64_t GetSeed() const;

  /// Updates the internal random number generator
  /// \see https://root.cern/doc/master/classTRandom.html
  /// for a list of available choices
  void SetGenerator(TRandom* new_rng);

  /// Returns a random number generator that draws samples from a
  /// uniform distribution with given parameters.
  UniformRng GetUniformRng(double min = 0, double max = 1);
  /// Returns a random number generator that draws samples from a
  /// gaus distribution with given parameters.
  GausRng GetGausRng(double mean = 0, double sigma = 1);
  /// Returns a random number generator that draws samples from a
  /// exp distribution with given parameters.
  ExpRng GetExpRng(double tau);
  /// Returns a random number generator that draws samples from a
  /// Landau distribution with given parameters.
  LandauRng GetLandauRng(double mean = 0, double sigma = 1);
  /// Returns a random number generator that draws samples from a
  /// PoissonD distribution with given parameters.
  PoissonDRng GetPoissonDRng(double mean);
  /// Returns a random number generator that draws samples from a
  /// BreitWigner distribution with given parameters.
  BreitWignerRng GetBreitWignerRng(double mean = 0, double gamma = 1);

  /// Returns a random number generator that draws samples from a
  /// user-defined distribution specified by parameter `function` between min
  /// and max.\n
  /// The user-defined distribution must follow the following signature:
  //// `[](const double* x, const double* params) { ... }` and must return a
  /// double.\n
  /// The following call will create a random number generator that will
  /// sample from a student-t distribution with `r = 1.0`. in the
  /// interval [-5, 10[.\n
  /// `x` is the function variable.\n
  ///
  ///     auto distribution = [](const double* x, const double* params) {
  ///       return ROOT::Math::tdistribution_pdf(*x, 1.0);
  ///     };
  ///     GetUserDefinedDistRng(distribution, {}, -5, 10);
  ///
  /// Instead of hardcoding the parameter `1.0` one can also pass it
  /// as second parameter. Up to 10 parameters can be used in the
  /// user-defined function.
  ///
  ///     auto distribution = [](const double* x, const double* params) {
  ///       return ROOT::Math::tdistribution_pdf(*x, params[0]);
  ///     };
  ///     GetUserDefinedDistRng(distribution, {1.0}, -5, 10);
  ///
  /// For param `option` have a look at ROOT's `TF1::GetRandom` documentation.\n
  /// \see https://root.cern/doc/master/group__PdfFunc.html
  /// \see https://root.cern/doc/master/classTF1.html
  UserDefinedDistRng1D GetUserDefinedDistRng1D(
      double (*f)(const double*, const double*),
      const FixedSizeVector<double, 10>& params, double min, double max,
      const char* option = nullptr);

  /// \see `Random::GetUserDefinedDistRng1D`
  UserDefinedDistRng2D GetUserDefinedDistRng2D(
      double (*f)(const double*, const double*),
      const FixedSizeVector<double, 10>& params, double xmin, double xmax,
      double ymin, double ymax, const char* option = nullptr);

  /// \see `Random::GetUserDefinedDistRng1D`
  UserDefinedDistRng3D GetUserDefinedDistRng3D(
      double (*f)(const double*, const double*),
      const FixedSizeVector<double, 10>& params, double xmin, double xmax,
      double ymin, double ymax, double zmin, double zmax,
      const char* option = nullptr);

  /// Returns a random number generator that draws samples from a
  /// Binomial distribution with given parameters.
  BinomialRng GetBinomialRng(int ntot, double prob);
  /// Returns a random number generator that draws samples from a
  /// Poisson distribution with given parameters.
  PoissonRng GetPoissonRng(double mean);

 private:
  friend class DistributionRng<double>;
  friend class DistributionRng<int>;

  TRandom* generator_ = nullptr;
  /// Stores TF1 pointers that have been created for a specific user-defined
  /// 1D distribution
  std::unordered_map<UserDefinedDist, TF1*> udd_tf1_map_;  //!
  /// Stores TF2 pointers that have been created for a specific user-defined
  /// 2D distribution
  std::unordered_map<UserDefinedDist, TF2*> udd_tf2_map_;  //!
  /// Stores TF3 pointers that have been created for a specific user-defined
  /// 3D distribution
  std::unordered_map<UserDefinedDist, TF3*> udd_tf3_map_;  //!
  BDM_CLASS_DEF_NV(Random, 3);
};

}  // namespace bdm

#endif  // CORE_UTIL_RANDOM_H_
