#include <gtest/gtest.h>
#include "multiform_object.h"

namespace bdm {

template <template <typename, typename, int> class TMemberSelector =
              SelectAllMembers>
class Simple {
  using SelfUnique = Simple<>;

 public:
  BDM_PUBLIC_MEMBER(double, a_) = 1.23;

  Simple() {}

  double GetA() { return a_; }
  double GetB() { return b_; }
  double GetC() { return c_; }

 protected:
  BDM_PROTECTED_MEMBER(double, b_) = 4.56;

 private:
  BDM_PRIVATE_MEMBER(double, c_) = 7.89;
};

TEST(MultiformObjectSimple, SelectAllMembers) {
  Simple<> simple;
  EXPECT_EQ(24u, sizeof(simple));

  Simple<SelectAllMembers> simple1;
  EXPECT_EQ(24u, sizeof(simple1));
}

NEW_MEMBER_SELECTOR(SelectB, Simple<>, b_);

TEST(MultiformObjectSimple, SelectB) {
  Simple<SelectB> only_b;
  EXPECT_EQ(8u, sizeof(only_b));
  EXPECT_EQ(4.56, only_b.GetB());
}

NEW_MEMBER_SELECTOR(SelectAC, Simple<>, a_, Simple<>, c_);

TEST(MultiformObjectSimple, SelectAC) {
  Simple<SelectAC> a_and_c;
  EXPECT_EQ(16u, sizeof(a_and_c));
  EXPECT_EQ(1.23, a_and_c.GetA());
  EXPECT_EQ(7.89, a_and_c.GetC());
}

NEW_MEMBER_REMOVER(RemoveB, Simple<>, b_);

TEST(MultiformObjectSimple, RemoveB) {
  Simple<RemoveB> a_and_c;
  EXPECT_EQ(16u, sizeof(a_and_c));
  EXPECT_EQ(1.23, a_and_c.GetA());
  EXPECT_EQ(7.89, a_and_c.GetC());
}

NEW_MEMBER_REMOVER(RemoveAC, Simple<>, a_, Simple<>, c_);

TEST(MultiformObjectSimple, RemoveAC) {
  Simple<RemoveAC> only_b;
  EXPECT_EQ(8u, sizeof(only_b));
  EXPECT_EQ(4.56, only_b.GetB());
}

// -----------------------------------------------------------------------------
// more complex example with two level class hierarchy

template <typename Base = BdmSimObject<>>
class CellExt1 : public Base {
  BDM_CLASS_HEADER(CellExt1, CellExt1<>,
                   CellExt1<typename Base::template Self<Backend>>, position_,
                   diameter_);

 public:
  explicit CellExt1(const std::array<real_v, 3>& pos) : position_{{pos}} {}

  CellExt1() : position_{{1, 2, 3}} {}

  const std::array<real_v, 3>& GetPosition() const { return position_[idx_]; }
  const real_v& GetDiameter() const { return diameter_[idx_]; }

  void SetDiameter(const real_v& diameter) { diameter_[idx_] = diameter; }

 protected:
  BDM_PROTECTED_MEMBER(Container<std::array<real_v COMMA() 3>>, position_);
  BDM_PROTECTED_MEMBER(Container<real_v>, diameter_) = {real_v(6.28)};
};

class Neurite {
 public:
  Neurite() : id_(0) {}
  explicit Neurite(std::size_t id) : id_(id) {}
  std::size_t id_;
};

// add Neurites to BaseCell
template <typename Base = CellExt1<>>
class NeuronExt1 : public Base {
  BDM_CLASS_HEADER(NeuronExt1, NeuronExt1<>,
                   NeuronExt1<typename Base::template Self<Backend>>, neurites_);

 public:
  template <class... A>
  explicit NeuronExt1(const SimdArray<std::vector<Neurite>>& neurites,
                     const A&... a)
      : Base(a...) {
    neurites_[idx_] = neurites;
  }

  NeuronExt1() = default;

  const SimdArray<std::vector<Neurite>>& GetNeurites() const {
    return neurites_[idx_];
  }

 private:
  BDM_PRIVATE_MEMBER(Container<SimdArray<std::vector<Neurite>>>,
                     neurites_) = {{}};
};

// define easy to use templated type alias
template <template <typename, typename, int> class TMemberSelector =
              SelectAllMembers>
using Neuron1 = NeuronExt1<CellExt1<BdmSimObject<TMemberSelector, ScalarBackend>>>;

TEST(MultiformObjectComplex, SelectAll1) {
  Neuron1<> all_members;
  EXPECT_EQ(64u, sizeof(all_members));
}

NEW_MEMBER_SELECTOR(SelectDiameter, CellExt1<>, diameter_);

TEST(MultiformObjectComplex, SelectDiameter) {
  Neuron1<SelectDiameter> only_diameter;
  EXPECT_EQ(16u, sizeof(only_diameter));
  EXPECT_EQ(6.28, only_diameter.GetDiameter()[0]);
}

NEW_MEMBER_SELECTOR(SelectPositionNeurites, CellExt1<>, position_, NeuronExt1<>,
                    neurites_);

TEST(MultiformObjectComplex, SelectPositionNeurites) {
  Neuron1<SelectPositionNeurites> neuron;
  EXPECT_EQ(56u, sizeof(neuron));
  auto& position = neuron.GetPosition();
  EXPECT_EQ(1, position[0][0]);
  EXPECT_EQ(2, position[1][0]);
  EXPECT_EQ(3, position[2][0]);
  EXPECT_EQ(0u, neuron.GetNeurites()[0].size());
}

NEW_MEMBER_REMOVER(RemovePositionNeurites, CellExt1<>, position_, NeuronExt1<>,
                   neurites_);

TEST(MultiformObjectComplex, RemovePositionNeurites) {
  Neuron1<RemovePositionNeurites> only_diameter;
  EXPECT_EQ(16u, sizeof(only_diameter));
  EXPECT_EQ(6.28, only_diameter.GetDiameter()[0]);
}

NEW_MEMBER_REMOVER(RemoveDiameter, CellExt1<>, diameter_);

TEST(MultiformObjectComplex, RemoveDiameter) {
  Neuron1<RemoveDiameter> neuron;
  EXPECT_EQ(56u, sizeof(neuron));
  auto& position = neuron.GetPosition();
  EXPECT_EQ(1, position[0][0]);
  EXPECT_EQ(2, position[1][0]);
  EXPECT_EQ(3, position[2][0]);
  EXPECT_EQ(0u, neuron.GetNeurites()[0].size());
}

// TODO(lukas) robustness tests

}  // namespace bdm
