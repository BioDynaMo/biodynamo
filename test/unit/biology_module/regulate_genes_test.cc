#ifndef UNIT_BIOLOGY_MODULE_REGULATE_GENES_TEST
#define UNIT_BIOLOGY_MODULE_REGULATE_GENES_TEST

#include "biology_module/regulate_genes.h"
#include "gtest/gtest.h"

namespace bdm {
namespace regulate_genes_test_internal {

struct TestCell {};

TEST(RegulateGenesTest, EulerTest) {
  Param::numerical_ode_solver_ = Param::NumericalODESolver::kEuler;
  Param::total_steps_ = 1;

  auto func1 = [](double curr_time, double last_concentration) {
    return curr_time * last_concentration;
  };
  auto func2 = [](double curr_time, double last_concentration) {
    return curr_time * last_concentration + 1;
  };
  auto func3 = [](double curr_time, double last_concentration) {
    return curr_time * last_concentration + 2;
  };

  RegulateGenes regulate_genes;
  regulate_genes.AddGene(func1, 3);
  regulate_genes.AddGene(func2, 3);
  regulate_genes.AddGene(func3, 3);
  TestCell cell;
  regulate_genes.Run(&cell);

  const auto& concentrations = regulate_genes.GetConcentrations();
  EXPECT_NEAR(3.0003000000000002, concentrations[0], 1e-9);
  EXPECT_NEAR(3.0103, concentrations[1], 1e-9);
  EXPECT_NEAR(3.0203000000000002, concentrations[2], 1e-9);
}

// Example 1 from:
// https://ece.uwaterloo.ca/~dwharder/NumericalAnalysis/14IVPs/rk/examples.html
TEST(RegulateGenesTest, RK4Test) {
  Param::numerical_ode_solver_ = Param::NumericalODESolver::kRK4;
  Param::total_steps_ = 0;
  Param::simulation_time_step_ = 1;

  RegulateGenes regulate_genes;
  regulate_genes.AddGene(
      [](double curr_time, double last_concentration) {
        return 1 - curr_time * last_concentration;
      },
      1);
  TestCell cell;
  regulate_genes.Run(&cell);

  const auto& concentrations = regulate_genes.GetConcentrations();
  EXPECT_NEAR(1.3229166667, concentrations[0], 1e-9);
}

}  // namespace regulate_genes_test_internal
}  // namespace bdm

#endif  // UNIT_BIOLOGY_MODULE_REGULATE_GENES_TEST
