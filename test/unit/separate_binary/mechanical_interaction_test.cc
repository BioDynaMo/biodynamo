#include "backend.h"
#include "cell.h"
#include "neuroscience/compile_time_param.h"
#include "neuroscience/neurite.h"
#include "neuroscience/neuron.h"
#include "unit/test_util.h"
#include "gtest/gtest.h"
#include "biodynamo.h"

namespace bdm {

  inline double DegreesToRadians(double degrees) {
  return degrees / 180 * Math::kPi;
}

  using neuroscience::Neurite;
  using neuroscience::Neuron;

  template <typename TBackend>
  struct CompileTimeParam : public DefaultCompileTimeParam<TBackend>, public neuroscience::DefaultCompileTimeParam<TBackend> {
    using AtomicTypes = VariadicTypedef<Cell, Neuron, Neurite>;
  };

  TEST(MechanicalInteraction, StraightCylinderGrowth) {
    Param::Reset();
    Rm()->Clear();

    //Param::live_visualization_ = true;

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, -100 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite(0, 0, 1).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 1, abs_error<double>::value);

    std::array<double, 3> direction = {0, 0, 1};
    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(300, direction);
      ne.RunDiscretization();
      scheduler.Simulate(1);
      if (i%10==0) {
        neAxis = ne.GetSpringAxis();

        EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
        EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
      }
    }

  }

  TEST(MechanicalInteraction, DiagonalxyCylinderGrowth) {
    Param::Reset();
    Rm()->Clear();

    Param::live_visualization_ = true;

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, -100 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite(1, 1, 0).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    EXPECT_NEAR(neAxis[0], 1, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 1, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);

    std::array<double, 3> direction = { 1, 1, 0 };
    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(300, direction);
      ne.RunDiscretization();
      scheduler.Simulate(1);
      if (i%10==0) {
        neAxis = ne.GetSpringAxis();

        EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);
      }
    }
  }

  TEST(MechanicalInteraction, DiagonalCylinderGrowth) {
    Param::Reset();
    Rm()->Clear();

    //Param::live_visualization_ = true;

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, -100 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite(2.0, DegreesToRadians(36.6992), DegreesToRadians(63.4349)).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 1, abs_error<double>::value);

    std::array<double, 3> direction = { 1.5, 2.3, 3.8 };
    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(300, direction);
      ne.RunDiscretization();
      scheduler.Simulate(1);
      if (i%10==0) {
        neAxis = ne.GetSpringAxis();

        EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
        EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
      }
    }
  }

  TEST(MechanicalInteraction, StraightCylinderGrowthObstacle) {
    Param::Reset();
    Rm()->Clear();

    //Param::live_visualization_ = true;

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto neuron2 = Rm()->New<Neuron>();
    neuron2.SetPosition({0, 0, 30});
    neuron2.SetMass(1);
    neuron2.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite(0, 0, 1).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 1, abs_error<double>::value);

    std::array<double, 3> direction = { 0, 0, 1 };
    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(50, direction);
      ne.RunDiscretization();
      scheduler.Simulate(1);
      if (i%10==0) {
        neAxis = ne.GetSpringAxis();

        EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
        EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
      }
    }

    auto position = ne.GetPosition();
    std::cout << position[0] << " ; " << position[1] << " ; " << position[2]  << std::endl;

  }

} // end namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
