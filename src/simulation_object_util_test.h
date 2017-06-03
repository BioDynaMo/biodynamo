#ifndef SIMULATION_OBJECT_UTIL_TEST_H_
#define SIMULATION_OBJECT_UTIL_TEST_H_

#include "simulation_object_util.h"

#include <array>
#include <vector>

#include <Rtypes.h>

#include "simulation_object.h"
#include "transactional_vector.h"

#define FRIEND_TEST(test_case_name, test_name) \
  friend class test_case_name##_##test_name##_Test

namespace bdm {
namespace simulation_object_util_test_internal {

template <typename Base = SimulationObject<>>
class CellExt : public Base {
  BDM_CLASS_HEADER(CellExt, position_, diameter_);

 public:
  explicit CellExt(const std::array<double, 3>& pos) : position_{{pos}} {}

  CellExt() : position_{{1, 2, 3}} {}

  void Divide(Self<Scalar>* daughter, double volume_ratio, double phi,
              double theta) {
    DivideImpl(daughter, volume_ratio, phi, theta);
  }

  virtual void DivideImpl(Self<Scalar>* daughter, double volume_ratio,
                          double phi, double theta) {
    daughter->position_[kIdx] = {5, 4, 3};
    diameter_[kIdx] = 1.123;
  }

  const std::array<double, 3>& GetPosition() const { return position_[kIdx]; }
  double GetDiameter() const { return diameter_[kIdx]; }

  void SetDiameter(double diameter) { diameter_[kIdx] = diameter; }

 protected:
  vec<std::array<double, 3>> position_;
  vec<double> diameter_ = {6.28};
};

// -----------------------------------------------------------------------------
// libraries for specific specialities add functionality - e.g. Neuroscience
class Neurite {
 public:
  explicit Neurite(TRootIOCtor* io_ctor) {}
  Neurite() : id_(0) {}
  explicit Neurite(std::size_t id) : id_(id) {}
  virtual ~Neurite() {}
  std::size_t id_;

 private:
  ClassDef(Neurite, 1);  // NOLINT
};

// add Neurites to BaseCell
template <typename Base = CellExt<>>
class NeuronExt : public Base {
  BDM_CLASS_HEADER(NeuronExt, neurites_);

 public:
  template <class... A>
  explicit NeuronExt(const std::vector<Neurite>& neurites, const A&... a)
      : Base(a...), neurites_{{neurites}} {}

  NeuronExt() = default;

  void DivideImpl(typename CellExt<>::template Self<Scalar>* daughter,
                  double volume_ratio, double phi, double theta) override {
    auto neuron = static_cast<Self<Scalar>*>(daughter);
    neuron->neurites_[kIdx].push_back(Neurite(987));
    Base::DivideImpl(daughter, volume_ratio, phi, theta);
  }

  const std::vector<Neurite>& GetNeurites() const { return neurites_[kIdx]; }

 private:
  vec<std::vector<Neurite>> neurites_ = {{}};

  FRIEND_TEST(SimulationObjectUtilTest, NewEmptySoa);
  FRIEND_TEST(SimulationObjectUtilTest, SoaRef);
  FRIEND_TEST(SimulationObjectUtilTest, Soa_clear);
  FRIEND_TEST(SimulationObjectUtilTest, Soa_reserve);
};

}  // namespace simulation_object_util_test_internal
}  // namespace bdm

#endif  // SIMULATION_OBJECT_UTIL_TEST_H_
