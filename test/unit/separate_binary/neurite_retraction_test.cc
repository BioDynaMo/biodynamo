#include "backend.h"
#include "biodynamo.h"
#include "cell.h"
#include "gtest/gtest.h"
#include "neuroscience/compile_time_param.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/neuron_soma.h"
#include "unit/test_util.h"

namespace bdm {

template <typename TBackend>
struct CompileTimeParam
    : public DefaultCompileTimeParam<TBackend>,
      public experimental::neuroscience::DefaultCompileTimeParam<TBackend> {
  using AtomicTypes = VariadicTypedef<Cell, experimental::neuroscience::NeuronSoma,
                                      experimental::neuroscience::NeuriteElement>;
};

namespace experimental {
namespace neuroscience {

// TODO(jean) Fix this test
TEST(DISABLED_NeuriteElementBehaviour, StraightxCylinderGrowthRetract) {
  Param::Reset();
  Rm()->Clear();

  Param::live_visualization_ = true;

  auto neuron = Rm()->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
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
    if (i % 10 == 0) {
      neAxis = ne.GetSpringAxis();

      EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
      EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);
    }
  }

  std::cout << "\n---- start retraction" << std::endl;
  double neurite_length = ne.GetLength();

  for (int j = 0; j < 10000; j++) {
    ne.RetractTerminalEnd(10);
    ne.RunDiscretization();
    scheduler.Simulate(1);
    std::cout << "retraction step: " << j << std::endl;
    if (j % 10 == 0) {
      neAxis = ne.GetSpringAxis();

      EXPECT_NEAR(neAxis[1], 0, abs_error<double>::value);
      EXPECT_NEAR(neAxis[2], 0, abs_error<double>::value);
    }
    neurite_length = ne.GetLength();
    std::cout << "neurite length: " << neurite_length << std::endl;
  }

  neurite_length = ne.GetLength();
  std::cout << "final neurite length: " << neurite_length << std::endl;
}

// TODO(jean) fix test
TEST(DISABLED_NeuriteElementBehaviour, BranchingGrowth) {
  Param::Reset();
  Rm()->Clear();

  Param::run_mechanical_interactions_ = true;
  // Param::live_visualization_ = true;
  // Param::export_visualization_ = true;

  double diam_reduc_speed = 0.001;
  double branching_factor = 0.005;

  auto neuron = Rm()->New<NeuronSoma>();
  neuron.SetPosition({0, 0, 0});
  neuron.SetMass(1);
  neuron.SetDiameter(10);

  auto ne = neuron.ExtendNewNeurite({0, 0, 1}).Get();
  ne.SetDiameter(1);

  auto& grid = Grid<>::GetInstance();
  grid.Initialize();
  Scheduler<> scheduler;

  std::array<double, 3> previous_direction;
  std::array<double, 3> direction;

  for (int i = 0; i < 200; i++) {
    auto my_neurites = Rm()->Get<NeuriteElement>();
    int num_neurites = my_neurites->size();

    for (int neurite_nb = 0; neurite_nb < num_neurites;
         neurite_nb++) {  // for each neurite in simulation
      auto ne = (*my_neurites)[neurite_nb];

      if (ne.IsTerminal() && ne.GetDiameter() > 0.5) {
        previous_direction = ne.GetSpringAxis();
        direction = {gTRandom.Uniform(-10, 10), gTRandom.Uniform(-10, 10),
                     gTRandom.Uniform(0, 5)};

        std::array<double, 3> step_direction =
            Math::Add(previous_direction, direction);

        ne.ElongateTerminalEnd(10, step_direction);
        //          ne.SetDiameter(ne.GetDiameter()-diam_reduc_speed);
        ne.SetDiameter(1);

        if (gTRandom.Uniform(0, 1) < branching_factor * ne.GetDiameter()) {
          ne.Bifurcate();
        }
        //            ne.RunDiscretization();
      }
    }
    scheduler.Simulate(1);
  }
}  // end test

}  // end namespace neuroscience
}  // end namespace experimental
}  // end namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
