#include "backend.h"
#include "cell.h"
#include "neuroscience/compile_time_param.h"
#include "neuroscience/neurite.h"
#include "neuroscience/neuron.h"
#include "unit/test_util.h"
#include "gtest/gtest.h"
#include "biodynamo.h"

namespace bdm {

  using neuroscience::Neurite;
  using neuroscience::Neuron;

  template <typename TBackend>
  struct CompileTimeParam : public DefaultCompileTimeParam<TBackend>, public neuroscience::DefaultCompileTimeParam<TBackend> {
    using AtomicTypes = VariadicTypedef<Cell, Neuron, Neurite>;
  };

  TEST(NeuriteBehaviour, StraightxCylinderGrowthRetract) {
    Param::Reset();
    Rm()->Clear();

    Param::live_visualization_ = true;

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite({1, 0, 0}).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    EXPECT_NEAR(neAxis[0], 1, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);

    std::array<double, 3> direction = {1, 0, 0};
    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(300, direction);
      ne.RunDiscretization();
      scheduler.Simulate(1);
      if (i%10==0) {
        neAxis = ne.GetSpringAxis();

        EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
        EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);
      }
    }

    std::cout << "\n---- start retraction" << std::endl;
    double myNeuriteLength = ne.GetLength();

    for (int j = 0; j < 10000; j++) {
      ne.RetractTerminalEnd(10);
      ne.RunDiscretization();
      scheduler.Simulate(1);
      std::cout << "retraction step: " << j << std::endl;
      if (j%10==0) {
        neAxis = ne.GetSpringAxis();

        EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
        EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);
      }
      myNeuriteLength = ne.GetLength();
      std::cout << "neurite length: " << myNeuriteLength << std::endl;
    }

    myNeuriteLength = ne.GetLength();
    std::cout << "final neurite length: " << myNeuriteLength << std::endl;

  }


} // end namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
