#ifndef SIMULATION_OBJECT_UTIL_TEST_H_
#define SIMULATION_OBJECT_UTIL_TEST_H_

#include "simulation_object_util.h"

#include <array>
#include <vector>

#include <Rtypes.h>

#include "simulation_object.h"
#include "transactional_vector.h"
#include "io_util.h"

#define ROOTFILE "bdmFile.root"

#define FRIEND_TEST(test_case_name, test_name) \
  friend class test_case_name##_##test_name##_Test

namespace bdm {
namespace simulation_object_util_test_internal {

template <typename Base = SimulationObject<>>
class CellExt : public Base {
  BDM_CLASS_HEADER(CellExt, 1, position_, diameter_);

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
  BDM_CLASS_HEADER(NeuronExt, 1, neurites_);

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

// -----------------------------------------------------------------------------
// SOA object for IO test
template <typename Base = SimulationObject<Soa>>
class TestObject : public Base {
  BDM_CLASS_HEADER(TestObject, 1, id_);
public:
  TestObject() {}
  TestObject(int id) : id_(id) {}
  int GetId() const { return id_[kIdx]; }

private:
  vec<int> id_;
};

using SoaTestObject = TestObject<>;
using ScalarTestObject = TestObject<SimulationObject<Scalar>>;


inline void RunSoaIOTest() {
  auto objects = SoaTestObject::NewEmptySoa();
  for (size_t i = 0; i < 10; i++) {
    objects.push_back(ScalarTestObject(i));
  }

  // write to root file
  WritePersistentObject(ROOTFILE, "objects", objects, "new");

  // read back
  SoaTestObject *restored_objects = nullptr;
  GetPersistentObject(ROOTFILE, "objects", restored_objects);

  // validate
  EXPECT_EQ(10u, restored_objects->size());
  for (size_t i = 0; i < 10; i++) {
    EXPECT_EQ(i, (*restored_objects)[i].GetId());
  }

  // delete root file
  remove(ROOTFILE);
}

}  // namespace simulation_object_util_test_internal
}  // namespace bdm

#endif  // SIMULATION_OBJECT_UTIL_TEST_H_
