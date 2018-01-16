#include "neuroscience/neuron.h"
#include "gtest/gtest.h"

#include "compile_time_param.h"
#include "neuroscience/compile_time_param.h"

// FIXME move to neuroscience directory

namespace bdm {

namespace neuroscience {
BDM_SIM_OBJECT(SpecializedNeurite, Neurite) {
  BDM_SIM_OBJECT_HEADER(SpecializedNeuriteExt, 1, foo_);
public:
  SpecializedNeuriteExt() {}
private:
  vec<int> foo_;
};
}

template <typename TBackend>
struct CompileTimeParam
    : public DefaultCompileTimeParam<TBackend>,
      public neuroscience::DefaultCompileTimeParam<TBackend> {
  using TNeuron = neuroscience::SpecializedNeuron;
  using TNeurite = neuroscience::SpecializedNeurite;
  using AtomicTypes = VariadicTypedef<neuroscience::SpecializedNeuron, neuroscience::SpecializedNeurite>;
};

namespace neuroscience {

TEST(NeuronTest, Scalar) {
  Neurite neurite;
  Neuron neuron;
  typename Neuron::template Self<Scalar> neuron1;
  SpecializedNeuron sneuron;
}

TEST(NeuronTest, Soa) {
  SoaNeuron neuron;
  SoaSpecializedNeuron sneuron;
  typename SpecializedNeuron::template Self<Soa> soan;
  typename CompileTimeParam<>::TNeuron soan1;
}

TEST(NeuronTest, ExtendNeuriteAndElongate) {
  auto* rm = Rm();
  Rm()->Clear();
  const double kEpsilon = 1e-6; // TODO abs_error<double>::value;
  std::array<double, 3> origin = {0, 0, 0};

  auto neuron = rm->New<SpecializedNeuron>(origin);
  neuron.SetDiameter(20);

  // auto neurite_segment = neuron.ExtendNewNeurite({0, 0, 1}).Get();
  auto neurite_segment = neuron.ExtendNewNeurite({0, 0, 1}).Get();



  // TODO LB necessary? or should it be removed?
  // auto commit = [](auto* sim_objects, uint16_t type_idx) {
  //   sim_objects->Commit();
  // };
  // rm->ApplyOnAllTypes(commit);

  neurite_segment.SetDiameter(2);
  std::cout << neurite_segment << std::endl;
  for (int i = 0; i < 1; ++i) {
    neurite_segment.ElongateTerminalEnd(10, {0, 0, 1});
    // neurite_segment.RunDiscretization();
  }

  std::cout << std::endl << neurite_segment << std::endl;




  EXPECT_NEAR(7.41, neurite_segment.GetLength(), kEpsilon);
  // EXPECT_NEAR(21, getTotalLength(ne->getPhysicalCylinder()), 1e-5);
}


}  // namespace neuroscience
}  // namespace bdm


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
