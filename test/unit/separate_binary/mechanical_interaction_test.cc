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

  TEST(MechanicalInteraction, StraightxCylinderGrowth) {
    Param::Reset();
    Rm()->Clear();

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
  }

  TEST(MechanicalInteraction, StraightyCylinderGrowth) {
    Param::Reset();
    Rm()->Clear();

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite({0, 1, 0}).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 1, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);

    std::array<double, 3> direction = {0, 1, 0};
    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(300, direction);
      ne.RunDiscretization();
      scheduler.Simulate(1);
      if (i%10==0) {
        neAxis = ne.GetSpringAxis();

        EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
        EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);
      }
    }
  }

  TEST(MechanicalInteraction, StraightzCylinderGrowth) {
    Param::Reset();
    Rm()->Clear();

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite({0, 0, 1}).Get();

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

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite({1, 1, 0}).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);

    std::array<double, 3> direction = { 1, 1, 0 };
    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(300, direction);
      ne.RunDiscretization();
      scheduler.Simulate(1);
      if (i%10==0) {
        neAxis = ne.GetSpringAxis();

        EXPECT_NEAR(neAxis[0], neAxis[1], abs_error<double>::value);
        EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);
      }
    }
  }

  TEST(MechanicalInteraction, DiagonalxyzCylinderGrowth) {
    Param::Reset();
    Rm()->Clear();

    Param::live_visualization_ = true;

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite({1, 1, 1}).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    EXPECT_NEAR(neAxis[0], 0.57735026918962584, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 0.57735026918962584, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 0.57735026918962584, abs_error<double>::value);

    std::array<double, 3> direction = { 1, 1, 1 };
    for (int i = 0; i < 37; i++) {

      ne.ElongateTerminalEnd(300, direction);
      ne.RunDiscretization();
      scheduler.Simulate(1);

      neAxis = ne.GetSpringAxis();

      std::cout << "xz spring axis: " << neAxis[0] << " ; " << neAxis[2] << std::endl;

      EXPECT_NEAR(neAxis[0], neAxis[1], abs_error<double>::value);
      EXPECT_NEAR(neAxis[0], neAxis[2], abs_error<double>::value);
    }
  }

  TEST(MechanicalInteraction, DiagonalSpecialDirectionCylinderGrowth) {
    Param::Reset();
    Rm()->Clear();

    //Param::live_visualization_ = true;

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

//    auto ne = neuron.ExtendNewNeurite(2.0, DegreesToRadians(36.6992), DegreesToRadians(63.4349)).Get();
    auto ne = neuron.ExtendNewNeurite({1, 1, 1}).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    EXPECT_NEAR(neAxis[0], 0.57735026918962584, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 0.57735026918962584, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 0.57735026918962584, abs_error<double>::value);

//    std::array<double, 3> direction = { 1.5, 2.3, 3.8 };
    std::array<double, 3> direction = { 2, 1, 1 };

    for (int i = 0; i < 98; i++) {
      ne.ElongateTerminalEnd(300, direction);
      ne.RunDiscretization();
      scheduler.Simulate(1);

      neAxis = ne.GetSpringAxis();

      EXPECT_TRUE(std::round(1e9*neAxis[1])==std::round(1e9*neAxis[2]));
    }
  }

  TEST(MechanicalInteraction, StraightCylinderGrowthObstacle) {
    Param::Reset();
    Rm()->Clear();

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto neuron2 = Rm()->New<Neuron>();
    neuron2.SetPosition({0, 0, 30});
    neuron2.SetMass(1);
    neuron2.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite({0, 0, 1}).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 1, abs_error<double>::value);

    std::array<double, 3> direction = { 0, 0, 1 };
    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(100, direction);
      ne.RunDiscretization();
      scheduler.Simulate(1);
      if (i%10==0) {
        neAxis = ne.GetSpringAxis();

        EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
        EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
      }
    }

  }


  // TODO(jean) Fix test
  TEST(DISABLED_MechanicalInteraction, NotStraightCylinderGrowthObstacle) {
    Param::Reset();
    Rm()->Clear();

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto neuron2 = Rm()->New<Neuron>();
    neuron2.SetPosition({0, 0, 30});
    neuron2.SetMass(1);
    neuron2.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite({0, 0, 1}).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();

    EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 1, abs_error<double>::value);

    std::array<double, 3> direction = { 0.01, 0, 1 };
    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(100, direction);
      ne.RunDiscretization();
      scheduler.Simulate(1);
    }

    neAxis = ne.GetSpringAxis();
    EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
  }



  TEST(MechanicalInteraction, DoubleStraightCylinderGrowth) {
    Param::Reset();
    Rm()->Clear();

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite({0, 1, 0}).Get();
    auto ne2 = neuron.ExtendNewNeurite({1, 0, 0}).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();
    std::array<double, 3> neAxis2 = ne2.GetSpringAxis();

    EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 1, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);

    EXPECT_NEAR(neAxis2[0], 1, abs_error<double>::value);
    EXPECT_NEAR(neAxis2[1], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis2[2], 0, abs_error<double>::value);

    std::array<double, 3> direction = {0, 1, 0};
    std::array<double, 3> direction2 = {1, 0, 0};

    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(300, direction);
      ne.RunDiscretization();
      ne2.ElongateTerminalEnd(300, direction2);
      ne2.RunDiscretization();

      scheduler.Simulate(1);
      if (i%10==0) {
        neAxis = ne.GetSpringAxis();
        neAxis2 = ne2.GetSpringAxis();

        EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
        EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);

        EXPECT_NEAR(neAxis2[1], 0, abs_error<double>::value);
        EXPECT_NEAR(neAxis2[2], 0, abs_error<double>::value);
      }
    }
  }

  TEST(MechanicalInteraction, BifurcationCylinderGrowth) {
    Param::Reset();
    Rm()->Clear();

    gErrorIgnoreLevel = kWarning;
    // Param::export_visualization_ = true;

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite({0, 0, 1}).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();
    std::array<double, 3> neAxis2;

    EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 1, abs_error<double>::value);

    std::array<double, 3> direction = {0, 0.5, 1};
    std::array<double, 3> direction2 = {0.5, 0, 1};

    for (int i = 0; i < 10; i++) {
      ne.ElongateTerminalEnd(100, {0, 0, 1});
      ne.RunDiscretization();
      scheduler.Simulate(1);
    }

//    std::cout << "---- bifurcation ----" << std::endl;
//    auto&& ne2=ne.Bifurcate();
    auto branches = ne.Bifurcate();
    auto&& branch_l = branches[0].Get();
    auto&& branch_r = branches[1].Get();

    for (int i = 0; i < 20; i++) {
      branch_r.ElongateTerminalEnd(100, direction);
      branch_r.RunDiscretization();

      if (i < 5) {
        branch_l.ElongateTerminalEnd(100, direction2);
        branch_l.RunDiscretization();
      }

      scheduler.Simulate(1);

//      if (i%10==0) {
//        neAxis = ne.GetSpringAxis();
//        neAxis2 = ne2.GetSpringAxis();

//        EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
//        EXPECT_NEAR(neAxis2[1], 0, abs_error<double>::value);
//      }
    }
  }

  TEST(MechanicalInteraction, BranchCylinderGrowth) {
    Param::Reset();
    Rm()->Clear();

    // Param::export_visualization_ = true;

    auto neuron = Rm()->New<Neuron>();
    neuron.SetPosition({ 0, 0, 0 });
    neuron.SetMass(1);
    neuron.SetDiameter(10);

    auto ne = neuron.ExtendNewNeurite({0, 0, 1}).Get();

    auto& grid = Grid<>::GetInstance();
    grid.Initialize();
    Scheduler<> scheduler;

    std::array<double, 3> neAxis = ne.GetSpringAxis();
    std::array<double, 3> neAxis2;

    EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
    EXPECT_NEAR(neAxis[2], 1, abs_error<double>::value);

    std::array<double, 3> direction = {0, 0.5, 1};
    std::array<double, 3> direction2 = {0.5, 0, 1};

    for (int i = 0; i < 10; i++) {
      ne.ElongateTerminalEnd(100, {0, 0, 1});
      ne.RunDiscretization();
      scheduler.Simulate(1);
    }

//    std::cout << "---- branch cylinder ----" << std::endl;
//    dynamic_cast<Neurite>(ne.Branch(0.5, direction2));
    auto ne2 = ne.Branch(0.5, direction2);

    for (int i = 0; i < 100; i++) {
      ne.ElongateTerminalEnd(100, direction);
//      ne2.ElongateTerminalEnd(100, direction2);

      ne.RunDiscretization();
//      ne2.RunDiscretization();

      scheduler.Simulate(1);

  //     if (i%10==0) {
  //       neAxis = ne.GetSpringAxis();
  //       neAxis2 = ne2.GetSpringAxis();
  //
  //       EXPECT_NEAR(neAxis[0], 0, abs_error<double>::value);
  //       EXPECT_NEAR(neAxis2[1], 0, abs_error<double>::value);
  //     }
    }
  }


} // end namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
