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
  // std::cout << static_cast<void*>(rm->template Get<Neuron>()) << std::endl;
  std::cout << rm->template Get<SpecializedNeuron>()->TotalSize() << std::endl;
  auto neuron = rm->New<SpecializedNeuron>(origin);
  neuron.SetDiameter(10);

  auto neurite_segment = neuron.ExtendNewNeurite({0, 0, 1}).Get();
  neurite_segment.SetDiameter(2);

  for (int i = 0; i < 200; ++i) {
    neurite_segment.ElongateTerminalEnd(10, {0, 0, 1});
    // ne->getPhysicalCylinder()->runDiscretization();
  }


  // EXPECT_NEAR(7.41, neurite_segment.GetLength(), kEpsilon);
  // EXPECT_NEAR(21, getTotalLength(ne->getPhysicalCylinder()), 1e-5);
}


}  // namespace neuroscience
}  // namespace bdm


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
