#include <gtest/gtest.h>
#include "simulation_object.h"
#include <typeinfo>

namespace bdm {

template <typename T>
struct is_std_array {
  static const bool value = false;
};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> {
  static const bool value = true;
};

template <typename T, typename U>
typename std::enable_if<!is_std_array<typename std::remove_reference<decltype(std::declval<T>()[0])>::type>::value>::type
CopyUtil(T* dest, std::size_t dest_v_idx, std::size_t dest_idx,
const U& src, std::size_t src_v_idx, std::size_t src_idx) {
  (*dest)[dest_v_idx][dest_idx] = src[src_v_idx][src_idx];
}

template <typename T, typename U>
typename std::enable_if<is_std_array<typename std::remove_reference<decltype(std::declval<T>()[0])>::type>::value>::type
CopyUtil(T* dest, std::size_t dest_v_idx, std::size_t dest_idx,
const U& src, std::size_t src_v_idx, std::size_t src_idx) {
  for(std::size_t i = 0; i < (*dest)[0].size(); i++) {
    (*dest)[dest_v_idx][i][dest_idx] = src[src_v_idx][i][src_idx];
  }
}

// The following tests check if code insertion in new classes works as intended
// Therefore BdmSimObject is extended in two stages: first by CellExt and
// then by NeuronExt

template <typename Base = BdmSimObject<>>
class CellExt : public Base {
  BDM_CLASS_HEADER(CellExt, CellExt<>,
                   CellExt<typename Base::template Self<Backend>>, position_,
                   diameter_);

 public:
  explicit CellExt(const std::array<real_v, 3>& pos) : position_{{pos}} {}

  CellExt() : position_{{1, 2, 3}} {}

  const std::array<real_v, 3>& GetPosition() const { return position_[idx_]; }
  const real_v& GetDiameter() const { return diameter_[idx_]; }

  void SetDiameter(const real_v& diameter) {
    diameter_[idx_] = diameter;
  }

  /* only compiled if Backend == Soa(Ref)Backend */
  /* template parameter required for enable_if - otherwise compile error */
  template <typename T = Backend>
  typename enable_if<is_same<T, VcSoaRefBackend>::value ||
                     is_same<T, VcSoaBackend>::value>::type
  push_back(const Self<ScalarBackend>& other) {
    if (Base::elements() == 0 || Base::is_full()) {

      typename decltype(position_)::value_type tmp;
      tmp[0][0] = other.position_[0][0][0];
      tmp[1][0] = other.position_[0][1][0];
      tmp[2][0] = other.position_[0][2][0];
      position_.push_back(tmp);

      typename decltype(diameter_)::value_type tmp1;
      tmp1[0] = other.diameter_[0][0];
      diameter_.push_back(tmp1);
      // tmp[0] = other.diameter_[0];

    } else {
      // data_member[size_][size_last_vector_] = other.data_member[0][0];
      // diameter_[Base::size_ - 1][Base::size_last_vector_] = other.diameter_[0][0];
      CopyUtil(&diameter_, Base::size_ - 1, Base::size_last_vector_, other.diameter_, 0, 0);

      // std::cout << "diameter_ " << is_std_array<decltype(diameter_)>::value << std::endl;
      // std::cout << "diameter_ " << is_std_array<typename std::remove_reference<decltype(diameter_[0])>::type>::value << std::endl;
      // std::cout << "position_ " << is_std_array<typename std::remove_reference<decltype(position_[0])>::type>::value << std::endl;
      // std::cout << "foo " << is_std_array<std::array<real_v, 3>>::value << std::endl;
      // std::cout << "     " << typeid(position_).name() << std::endl;
      // std::cout << "     " << typeid(position_[0]).name() << std::endl;
      // std::cout << "     " << typeid(std::array<real_v, 3>()).name() << std::endl;
      // std::array<real_v, 3> tmp;
      // std::cout << "     " << typeid(tmp).name() << std::endl;
      // std::cout << "     " << typeid(&tmp).name() << std::endl;
      // typename std::remove_reference<decltype(position_[0])>::type tmp1;
      // std::cout << "     " << typeid(tmp1).name() << std::endl;
      CopyUtil(&position_, Base::size_ - 1, Base::size_last_vector_, other.position_, 0, 0);
      // position_[Base::size_- 1][0][Base::size_last_vector_] = other.position_[0][0][0];
      // position_[Base::size_- 1][1][Base::size_last_vector_] = other.position_[0][1][0];
      // position_[Base::size_- 1][2][Base::size_last_vector_] = other.position_[0][2][0];
    }
    Base::push_back(other);
  }

  /* only compiled if Backend == VcBackend */
  /* template parameter required for enable_if - otherwise compile error */
  template <typename T = Backend>
  typename enable_if<is_same<T, VcBackend>::value>::type
  push_back(const Self<ScalarBackend>& other) {
    // diameter_[0][Base::size_] = other.diameter_[0][0];
    CopyUtil(&diameter_, 0, Base::size_, other.diameter_, 0, 0);
    CopyUtil(&position_, 0, Base::size_, other.position_, 0, 0);

    // position_[0][0][Base::size_] = other.position_[0][0][0];
    // position_[0][1][Base::size_] = other.position_[0][1][0];
    // position_[0][2][Base::size_] = other.position_[0][2][0];
    Base::push_back(other);
  }

  virtual void CopyTo(std::size_t src_v_idx, std::size_t src_idx,
              std::size_t dest_v_idx,
              std::size_t dest_idx,
              BdmSimObjectVectorBackend* destination) const override {
    Self<VcBackend>* dest = static_cast<Self<VcBackend>*>(destination);


    // dest->diameter_[0][dest_idx] = diameter_[src_v_idx][src_idx];
    CopyUtil(&dest->diameter_, 0, dest_idx, diameter_, src_v_idx, src_idx);
    CopyUtil(&dest->position_, 0, dest_idx, position_, src_v_idx, src_idx);

    // dest->position_[0][0][dest_idx] = position_[src_v_idx][0][src_idx];
    // dest->position_[0][1][dest_idx] = position_[src_v_idx][1][src_idx];
    // dest->position_[0][2][dest_idx] = position_[src_v_idx][2][src_idx];
    Base::CopyTo(src_v_idx, src_idx,
                 dest_v_idx,
                 dest_idx,
                 destination);
  }

 protected:
  BDM_PROTECTED_MEMBER(Container<std::array<real_v COMMA() 3>>, position_);
  BDM_PROTECTED_MEMBER(Container<real_v>, diameter_) = {real_v(6.28)};
};

// -----------------------------------------------------------------------------
// libraries for specific specialities add functionality - e.g. Neuroscience
class Neurite {
 public:
  Neurite() : id_(0) {}
  Neurite(std::size_t id) : id_(id) {}
  std::size_t id_;
};

// add Neurites to BaseCell
template <typename Base = CellExt<>>
class NeuronExt : public Base {
  BDM_CLASS_HEADER(NeuronExt, NeuronExt<>,
                   NeuronExt<typename Base::template Self<Backend>>, neurites_);

 public:
  template <class... A>
  explicit NeuronExt(const SimdArray<std::vector<Neurite>>& neurites,
                  const A&... a)
      : Base(a...) {
    neurites_[idx_] = neurites;
  }

  NeuronExt() = default;

  const SimdArray<std::vector<Neurite>>& GetNeurites() const {
    return neurites_[idx_];
  }

  /* only compiled if Backend == Soa(Ref)Backend */
  /* template parameter required for enable_if - otherwise compile error */
  template <typename T = Backend>
  typename enable_if<is_same<T, VcSoaRefBackend>::value ||
                     is_same<T, VcSoaBackend>::value>::type
  push_back(const Self<ScalarBackend>& other) {
    if (Base::elements() == 0 || Base::is_full()) {
      typename decltype(neurites_)::value_type tmp;
      tmp[0] = other.neurites_[0][0];
      neurites_.push_back(tmp);

    } else {
      // data_member[size_][size_last_vector_] = other.data_member[0][0];
      neurites_[Base::size_- 1][Base::size_last_vector_] = other.neurites_[0][0];
      // FIXME SimdArray is std::array -> wrong CopyUtil version gets chosen
      // CopyUtil(&neurites_, Base::size_ - 1, Base::size_last_vector_, other.neurites_, 0, 0);
    }
    Base::push_back(other);
  }

  /* only compiled if Backend == VcBackend */
  /* template parameter required for enable_if - otherwise compile error */
  template <typename T = Backend>
  typename enable_if<is_same<T, VcBackend>::value>::type
  push_back(const Self<ScalarBackend>& other) {
    neurites_[0][Base::size_] = other.neurites_[0][0];
    Base::push_back(other);
  }

  virtual void CopyTo(std::size_t src_v_idx, std::size_t src_idx,
              std::size_t dest_v_idx,
              std::size_t dest_idx,
              BdmSimObjectVectorBackend* destination) const override {
    Self<VcBackend>* dest = static_cast<Self<VcBackend>*>(destination);

    dest->neurites_[0][dest_idx] = neurites_[src_v_idx][src_idx];

    Base::CopyTo(src_v_idx, src_idx,
                 dest_v_idx,
                 dest_idx,
                 destination);
  }

 private:
  BDM_PRIVATE_MEMBER(Container<SimdArray<std::vector<Neurite>>>, neurites_) = {{}};

  FRIEND_TEST(SimulationObjectUtilTest, SoaBackend_clear);
  FRIEND_TEST(SimulationObjectUtilTest, SoaBackend_reserve);
  FRIEND_TEST(SimulationObjectUtilTest, SoaBackend_push_backScalarOnNonEmptySoa);
};

// define easy to use templated type alias
template <typename Backend=VcBackend>
using Neuron = NeuronExt<CellExt<BdmSimObject<SelectAllMembers, Backend>>>;


TEST(SimulationObjectUtilTest, DefaultConstructor) {
  // are data members in all extensions correctly initialized?
  Neuron<VcBackend> neuron;

  EXPECT_TRUE((VcBackend::real_v(6.28) == neuron.GetDiameter()).isFull());
  auto positions = neuron.GetPosition();
  EXPECT_TRUE((VcBackend::real_v(1) == positions[0]).isFull());
  EXPECT_TRUE((VcBackend::real_v(2) == positions[1]).isFull());
  EXPECT_TRUE((VcBackend::real_v(3) == positions[2]).isFull());

  auto neurites_array = neuron.GetNeurites();
  for (auto& neurites : neurites_array ) {
    EXPECT_EQ(0u, neurites.size());
  }
}

TEST(SimulationObjectUtilTest, NonDefaultConstructor) {
  // are data members in all extensions correctly initialized?
  using real_v = VcBackend::real_v;

  std::vector<Neurite> neurites;
  neurites.push_back(Neurite(2));
  neurites.push_back(Neurite(3));
  VcBackend::SimdArray<std::vector<Neurite>> neurite_v;
  for (std::size_t i = 0; i < neurite_v.size(); i++) neurite_v[i] = neurites;

  Neuron<VcBackend> neuron(neurite_v, std::array<real_v, 3>{real_v(4), real_v(5), real_v(6)});

  EXPECT_TRUE((VcBackend::real_v(6.28) == neuron.GetDiameter()).isFull());
  auto positions = neuron.GetPosition();
  EXPECT_TRUE((VcBackend::real_v(4) == positions[0]).isFull());
  EXPECT_TRUE((VcBackend::real_v(5) == positions[1]).isFull());
  EXPECT_TRUE((VcBackend::real_v(6) == positions[2]).isFull());

  auto& neurites_array = neuron.GetNeurites();
  for (auto& neurites : neurites_array ) {
    EXPECT_EQ(2u, neurites.size());
  }
}

TEST(SimulationObjectUtilTest, NewScalar) {
  using real_v = ScalarBackend::real_v;
  auto neuron = Neuron<VcBackend>::NewScalar();

  EXPECT_TRUE((real_v(6.28) == neuron.GetDiameter()).isFull());
  auto positions = neuron.GetPosition();
  EXPECT_TRUE((real_v(1) == positions[0]).isFull());
  EXPECT_TRUE((real_v(2) == positions[1]).isFull());
  EXPECT_TRUE((real_v(3) == positions[2]).isFull());

  auto neurites_array = neuron.GetNeurites();
  for (auto& neurites : neurites_array ) {
    EXPECT_EQ(0u, neurites.size());
  }
}

TEST(SimulationObjectUtilTest, NewEmptySoa) {
  auto neurons = Neuron<>::NewEmptySoa();
  neurons.size();
  EXPECT_EQ(0u, neurons.size());
  EXPECT_EQ(0u, neurons.vectors());
  EXPECT_EQ(0u, neurons.elements());
}

TEST(SimulationObjectUtilTest, GetSoaRef) {
  using real_v = VcBackend::real_v;
  Neuron<VcSoaBackend> neurons;
  auto neurons_ref = neurons.GetSoaRef();
  neurons_ref.SetDiameter(real_v(12.34));
  EXPECT_TRUE((real_v(12.34) == neurons.GetDiameter()).isFull());
}

TEST(SimulationObjectUtilTest, SoaBackend_push_backVector_AndSubscriptOperator) {
  using real_v = VcBackend::real_v;

  std::vector<Neurite> neurites;
  neurites.push_back(Neurite(2));
  neurites.push_back(Neurite(3));
  VcBackend::SimdArray<std::vector<Neurite>> neurite_v;
  for (std::size_t i = 0; i < neurite_v.size(); i++) neurite_v[i] = neurites;

  Neuron<VcBackend> neuron_v1(neurite_v, std::array<real_v, 3>{real_v(4), real_v(5), real_v(6)});

  neurites.push_back(Neurite(4));
  for (std::size_t i = 0; i < neurite_v.size(); i++) neurite_v[i] = neurites;
  Neuron<VcBackend> neuron_v2(neurite_v, std::array<real_v, 3>{real_v(9), real_v(8), real_v(7)});

  auto neurons = Neuron<>::NewEmptySoa();
  neurons.push_back(neuron_v1);
  neurons.push_back(neuron_v2);

  // switch to neuron_v2
  auto& result1 = neurons[1];

  // operator[] returns reference to *this
  EXPECT_TRUE(&result1 == &neurons);

  EXPECT_TRUE((VcBackend::real_v(6.28) == result1.GetDiameter()).isFull());
  auto positions = result1.GetPosition();
  EXPECT_TRUE((VcBackend::real_v(9) == positions[0]).isFull());
  EXPECT_TRUE((VcBackend::real_v(8) == positions[1]).isFull());
  EXPECT_TRUE((VcBackend::real_v(7) == positions[2]).isFull());

  auto neurite_v_actual = result1.GetNeurites();
  for (auto& neurites_s : neurite_v_actual ) {
    EXPECT_EQ(3u, neurites_s.size());
  }
}

TEST(SimulationObjectUtilTest, SoaBackend_push_backScalarOnEmptySoa) {
  auto neurons = Neuron<>::NewEmptySoa();
  EXPECT_EQ(0u, neurons.size());


  using real_v = ScalarBackend::real_v;
  std::vector<Neurite> neurites;
  neurites.push_back(Neurite(22));
  neurites.push_back(Neurite(33));
  Neuron<ScalarBackend> single_neuron({neurites}, std::array<real_v, 3>{real_v(6), real_v(3), real_v(9)});;
  single_neuron.SetDiameter({1.2345});

  neurons.push_back(single_neuron);
  EXPECT_EQ(1u, neurons.size());
  EXPECT_EQ(1u, neurons.vectors());
  EXPECT_EQ(1u, neurons.elements());

  // check if scalar data members have been copied correctly
  EXPECT_EQ(1.2345, neurons.GetDiameter()[0]);
  auto& position = neurons.GetPosition();
  EXPECT_EQ(6, position[0][0]);
  EXPECT_EQ(3, position[1][0]);
  EXPECT_EQ(9, position[2][0]);
  auto& neurites_actual = neurons.GetNeurites()[0];
  EXPECT_EQ(2u, neurites_actual.size());
  EXPECT_EQ(22u, neurites_actual[0].id_);
  EXPECT_EQ(33u, neurites_actual[1].id_);
}

TEST(SimulationObjectUtilTest, SoaBackend_push_backScalarOnNonEmptySoa) {
  Neuron<VcSoaBackend> neurons; // stores one vector neuron with default values
  EXPECT_EQ(1u, neurons.size());
  EXPECT_EQ(1u, neurons.vectors());
  EXPECT_EQ(4u, neurons.elements());  // FIXME replace 4 with VcBackend::kVecLen

  // simulate that the first vector only holds one scalar
  neurons.size_last_vector_ = 1;

  using real_v = ScalarBackend::real_v;
  std::vector<Neurite> neurites;
  neurites.push_back(Neurite(22));
  neurites.push_back(Neurite(33));
  Neuron<ScalarBackend> single_neuron({neurites}, std::array<real_v, 3>{real_v(6), real_v(3), real_v(9)});;
  single_neuron.SetDiameter({1.2345});

  neurons.push_back(single_neuron);
  EXPECT_EQ(1u, neurons.size());
  EXPECT_EQ(1u, neurons.vectors());
  EXPECT_EQ(2u, neurons.elements());

  // check if scalar data members have been copied correctly
  EXPECT_EQ(1.2345, neurons.GetDiameter()[1]);
  auto& position = neurons.GetPosition();
  EXPECT_EQ(6, position[0][1]);
  EXPECT_EQ(3, position[1][1]);
  EXPECT_EQ(9, position[2][1]);
  auto& neurites_actual = neurons.GetNeurites()[1];
  EXPECT_EQ(2u, neurites_actual.size());
  EXPECT_EQ(22u, neurites_actual[0].id_);
  EXPECT_EQ(33u, neurites_actual[1].id_);
}

TEST(SimulationObjectUtilTest, SoaBackend_clear) {
  Neuron<VcSoaBackend> neurons;
  EXPECT_EQ(1u, neurons.size());
  neurons.clear();
  EXPECT_EQ(0u, neurons.size());
  EXPECT_EQ(0u, neurons.neurites_.size());
  EXPECT_EQ(0u, neurons.diameter_.size());
  EXPECT_EQ(0u, neurons.position_.size());
}

TEST(SimulationObjectUtilTest, SoaBackend_reserve) {
  Neuron<VcSoaBackend> neurons;
  neurons.reserve(10);
  EXPECT_EQ(10u, neurons.neurites_.capacity());
  EXPECT_EQ(10u, neurons.diameter_.capacity());
  EXPECT_EQ(10u, neurons.position_.capacity());
}

TEST(SimulationObjectUtilTest, SoaBackend_Gather) {
  auto objects = Neuron<>::NewEmptySoa();

  // create objects
  for (size_t i = 0; i < 10; i++) {
    using real_v = ScalarBackend::real_v;
    std::vector<Neurite> neurites;
    neurites.push_back(Neurite(i));
    Neuron<ScalarBackend> scalar({neurites},
                                  std::array<real_v, 3>{real_v(i), real_v(i), real_v(i)});
    scalar.SetDiameter(i);
    objects.push_back(scalar);
  }

  aosoa<Neuron<VcBackend>, VcBackend> gathered;
  InlineVector<int, 8> indexes;
  indexes.push_back(5);
  indexes.push_back(3);
  indexes.push_back(9);
  indexes.push_back(2);
  objects.Gather(indexes, &gathered);
  // check if it returns the correct objects
  size_t target_n_vectors =
      4 / VcBackend::kVecLen + (4 % VcBackend::kVecLen ? 1 : 0);
  EXPECT_EQ(target_n_vectors, gathered.vectors());
  size_t counter = 0;
  for (size_t i = 0; i < gathered.vectors(); i++) {
    for (size_t j = 0; j < VcBackend::kVecLen; j++) {
      EXPECT_EQ(indexes[counter], gathered[i].GetDiameter()[j]);
      EXPECT_EQ(indexes[counter], gathered[i].GetPosition()[0][j]);
      EXPECT_EQ(indexes[counter], gathered[i].GetPosition()[1][j]);
      EXPECT_EQ(indexes[counter], gathered[i].GetPosition()[2][j]);
      EXPECT_EQ(indexes[counter], gathered[i].GetNeurites()[j][0].id_);
      counter++;
    }
  }
}

TEST(SimulationObjectUtilTest, VectorBackend_push_backScalar) {
  Neuron<VcBackend> neurons; // stores one vector neuron with default values
  EXPECT_EQ(4u, neurons.size());  // replace with VcBackend::kVecLen
  // EXPECT_EQ(1u, neurons.vectors());
  // EXPECT_EQ(4u, neurons.elements());  // FIXME replace 4 with VcBackend::kVecLen

  // simulate that the vector only holds one scalar - remaining slots are free
  neurons.SetSize(1);

  using real_v = ScalarBackend::real_v;
  std::vector<Neurite> neurites;
  neurites.push_back(Neurite(22));
  neurites.push_back(Neurite(33));
  Neuron<ScalarBackend> single_neuron({neurites}, std::array<real_v, 3>{real_v(6), real_v(3), real_v(9)});;
  single_neuron.SetDiameter({1.2345});

  neurons.push_back(single_neuron);
  // EXPECT_EQ(1u, neurons.size());
  // EXPECT_EQ(1u, neurons.vectors());
  // EXPECT_EQ(2u, neurons.elements());

  // check if scalar data members have been copied correctly
  EXPECT_EQ(1.2345, neurons.GetDiameter()[1]);
  auto& position = neurons.GetPosition();
  EXPECT_EQ(6, position[0][1]);
  EXPECT_EQ(3, position[1][1]);
  EXPECT_EQ(9, position[2][1]);
  auto& neurites_actual = neurons.GetNeurites()[1];
  EXPECT_EQ(2u, neurites_actual.size());
  EXPECT_EQ(22u, neurites_actual[0].id_);
  EXPECT_EQ(33u, neurites_actual[1].id_);
}



// TODO test assignment operator

}  // namespace bdm
