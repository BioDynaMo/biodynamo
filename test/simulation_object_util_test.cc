#include "simulation_object_util.h"

#include <array>
#include "gtest/gtest.h"

#include "simulation_object.h"
#include "transactional_vector.h"

namespace bdm {
namespace simulation_object_util_test_internal {

// The following tests check if code insertion in new classes works as intended
// Therefore SimulationObject is extended in two stages: first by CellExt and
// then by NeuronExt

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
  Neurite() : id_(0) {}
  explicit Neurite(std::size_t id) : id_(id) {}
  std::size_t id_;
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

// define easy to use templated type alias
template <typename Backend = Scalar>
using Neuron = NeuronExt<CellExt<SimulationObject<Backend>>>;

template <typename T>
void RunDefaultConstructorTest(const T& neuron) {
  EXPECT_EQ(1u, neuron.size());

  EXPECT_EQ(6.28, neuron.GetDiameter());
  auto& position = neuron.GetPosition();
  EXPECT_EQ(1, position[0]);
  EXPECT_EQ(2, position[1]);
  EXPECT_EQ(3, position[2]);
  auto neurites = neuron.GetNeurites();
  EXPECT_EQ(0u, neurites.size());
}

TEST(SimulationObjectUtilTest, DefaultConstructor) {
  // are data members in all extensions correctly initialized?
  Neuron<Scalar> neuron;
  RunDefaultConstructorTest(Neuron<Scalar>());
  RunDefaultConstructorTest(Neuron<Soa>());
}

TEST(SimulationObjectUtilTest, NewEmptySoa) {
  // Create an empty SOA container
  // Creating it using e.g. `Neuron<SOA> soa;` will already have one element
  // inside - (the one with default parameters)
  auto neurons = Neuron<>::NewEmptySoa();
  EXPECT_EQ(0u, neurons.size());
  EXPECT_EQ(0u, neurons.neurites_.size());
  EXPECT_EQ(0u, neurons.diameter_.size());
  EXPECT_EQ(0u, neurons.position_.size());
}

TEST(SimulationObjectUtilTest, NonDefaultConstructor) {
  std::vector<Neurite> neurites;
  neurites.push_back(Neurite(2));
  neurites.push_back(Neurite(3));

  Neuron<> neuron(neurites, std::array<double, 3>{4, 5, 6});

  EXPECT_EQ(6.28, neuron.GetDiameter());
  auto& position = neuron.GetPosition();
  EXPECT_EQ(4, position[0]);
  EXPECT_EQ(5, position[1]);
  EXPECT_EQ(6, position[2]);
  EXPECT_EQ(2u, neurites.size());
}

TEST(SimulationObjectUtilTest, SoaRef) {
  Neuron<Soa> neurons;

  auto neurons_ref = neurons[0];

  EXPECT_EQ(1u, neurons_ref.size());

  // check if changes are visible for the referenced object
  neurons_ref.SetDiameter(12.34);
  EXPECT_EQ(12.34, neurons.GetDiameter());
  neurons_ref.push_back(Neuron<Scalar>());
  EXPECT_EQ(2u, neurons.size());
}

TEST(SimulationObjectUtilTest, Soa_push_back_AndSubscriptOperator) {
  std::vector<Neurite> neurites;
  neurites.push_back(Neurite(2));
  neurites.push_back(Neurite(3));

  Neuron<> neuron1(neurites, std::array<double, 3>{4, 5, 6});

  neurites.push_back(Neurite(4));
  Neuron<> neuron2(neurites, std::array<double, 3>{9, 8, 7});

  auto neurons = Neuron<>::NewEmptySoa();
  neurons.push_back(neuron1);
  neurons.push_back(neuron2);

  EXPECT_EQ(2u, neurons.size());

  EXPECT_EQ(6.28, neurons[0].GetDiameter());
  auto& position1 = neurons[0].GetPosition();
  EXPECT_EQ(4, position1[0]);
  EXPECT_EQ(5, position1[1]);
  EXPECT_EQ(6, position1[2]);
  EXPECT_EQ(2u, neurons[0].GetNeurites().size());

  // test if return type of subscript operator has SoaRef backend
  auto element1 = neurons[1];
  Neuron<SoaRef>* cast_result = dynamic_cast<Neuron<SoaRef>*>(&element1);
  EXPECT_TRUE(cast_result != nullptr);

  EXPECT_EQ(6.28, neurons[1].GetDiameter());
  auto& position2 = neurons[1].GetPosition();
  EXPECT_EQ(9, position2[0]);
  EXPECT_EQ(8, position2[1]);
  EXPECT_EQ(7, position2[2]);
  EXPECT_EQ(3u, neurons[1].GetNeurites().size());
}

TEST(SimulationObjectUtilTest, Soa_clear) {
  Neuron<Soa> neurons;
  EXPECT_EQ(1u, neurons.size());
  neurons.clear();
  EXPECT_EQ(0u, neurons.size());
  EXPECT_EQ(0u, neurons.neurites_.size());
  EXPECT_EQ(0u, neurons.diameter_.size());
  EXPECT_EQ(0u, neurons.position_.size());
}

TEST(SimulationObjectUtilTest, Soa_reserve) {
  Neuron<Soa> neurons;
  neurons.reserve(10);
  EXPECT_EQ(10u, neurons.neurites_.capacity());
  EXPECT_EQ(10u, neurons.diameter_.capacity());
  EXPECT_EQ(10u, neurons.position_.capacity());
}

TEST(SimulationObjectUtilTest, Soa_AssignmentOperator) {
  std::vector<Neurite> neurites;
  neurites.push_back(Neurite(2));
  neurites.push_back(Neurite(3));

  Neuron<> neuron1(neurites, std::array<double, 3>{4, 5, 6});

  neurites.push_back(Neurite(4));
  Neuron<> new_neuron1(neurites, std::array<double, 3>{9, 8, 7});
  new_neuron1.SetDiameter(123);

  auto neurons = Neuron<>::NewEmptySoa();
  neurons.push_back(neuron1);

  EXPECT_EQ(1u, neurons.size());

  neurons[0] = new_neuron1;
  EXPECT_EQ(123, neurons[0].GetDiameter());
  auto& position = neurons[0].GetPosition();
  EXPECT_EQ(9, position[0]);
  EXPECT_EQ(8, position[1]);
  EXPECT_EQ(7, position[2]);
  EXPECT_EQ(3u, neurons[0].GetNeurites().size());
}

template <typename TContainer>
void RunDivideTest(TContainer* neurons) {
  Neuron<Scalar> neuron;
  neurons->push_back(neuron);

  auto&& new_neuron = Divide((*neurons)[0], neurons, 1.0, 2.0, 3.0);

  EXPECT_EQ(987u, new_neuron.GetNeurites()[0].id_);
  EXPECT_EQ(5, new_neuron.GetPosition()[0]);
  EXPECT_EQ(4, new_neuron.GetPosition()[1]);
  EXPECT_EQ(3, new_neuron.GetPosition()[2]);

  // commit invalidates new_neuron
  neurons->Commit();

  ASSERT_EQ(2u, neurons->size());
  // new_neuron got invalidated by `Commit()`, but is now accessible in neurons
  EXPECT_EQ(987u, (*neurons)[1].GetNeurites()[0].id_);
  EXPECT_EQ(5, (*neurons)[1].GetPosition()[0]);
  EXPECT_EQ(4, (*neurons)[1].GetPosition()[1]);
  EXPECT_EQ(3, (*neurons)[1].GetPosition()[2]);
  EXPECT_EQ(1.123, (*neurons)[0].GetDiameter());
}

TEST(SimulationObjectUtilTest, Aos_Divide) {
  TransactionalVector<Neuron<>> neurons;
  RunDivideTest(&neurons);
}

TEST(SimulationObjectUtilTest, Soa_Divide) {
  auto neurons = Neuron<>::NewEmptySoa();
  RunDivideTest(&neurons);
}

template <typename TContainer>
void RunDeleteTest(TContainer* neurons) {
  Neuron<Scalar> neuron;
  neurons->push_back(neuron);

  Delete(neurons, 0);

  neurons->Commit();

  EXPECT_EQ(0u, neurons->size());
}

TEST(SimulationObjectUtilTest, Aos_Delete) {
  TransactionalVector<Neuron<>> neurons;
  RunDeleteTest(&neurons);
}

TEST(SimulationObjectUtilTest, Soa_Delete) {
  auto neurons = Neuron<>::NewEmptySoa();
  RunDeleteTest(&neurons);
}

}  // namespace simulation_object_util_test_internal
}  // namespace bdm
