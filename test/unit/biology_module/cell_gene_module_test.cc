#ifndef UNIT_GENE_MODULE_TEST
#define UNIT_GENE_MODULE_TEST
#include "cell_gene_module.h"
#include "gtest/gtest.h"

namespace bdm {

TEST(CellGeneModuleTest, FunctionStructureTest) {
  std::vector<double> init_values{{3, 3, 3}};
  Param::total_steps_ = 1;
  std::function<double(double, double)> func1 = [&](
      double curr_time, double substance_) -> double {
    return curr_time * substance_;
  };
  std::function<double(double, double)> func2 = [&](
      double curr_time, double substance_) -> double {
    return curr_time * substance_ + 1;
  };
  std::function<double(double, double)> func3 = [&](
      double curr_time, double substance_) -> double {
    return curr_time * substance_ + 2;
  };

  RegulateGenes regulate_genes;
  regulate_genes.AddFunction(func1, init_values[0]);
  regulate_genes.AddFunction(func2, init_values[1]);
  regulate_genes.AddFunction(func3, init_values[2]);

  vector<double> expected_result;

  expected_result.push_back(func1(1, init_values[0]));
  expected_result.push_back(func2(1, init_values[1]));
  expected_result.push_back(func3(1, init_values[2]));

  vector<double> actual_result = regulate_genes.Calculate(1, init_values);
  EXPECT_EQ(expected_result, actual_result);
}

TEST(GeneExpressionTest, EulerTest) {
  Param::d_e_solve_method_choosed_ = Param::DESolveMethod::kEuler;
  std::vector<double> init_values{{3, 3, 3}};
  Param::total_steps_ = 1;
  std::function<double(double, double)> func1 = [&](
      double curr_time, double substance_) -> double {
    return curr_time * substance_;
  };
  std::function<double(double, double)> func2 = [&](
      double curr_time, double substance_) -> double {
    return curr_time * substance_ + 1;
  };
  std::function<double(double, double)> func3 = [&](
      double curr_time, double substance_) -> double {
    return curr_time * substance_ + 2;
  };

  RegulateGenes regulate_genes;
  regulate_genes.AddFunction(func1, init_values[0]);
  regulate_genes.AddFunction(func2, init_values[1]);
  regulate_genes.AddFunction(func3, init_values[2]);
  Cell cell;
  regulate_genes.Run(&cell);

  init_values[0] += func1(Param::total_steps_ * Param::simulation_time_step_,
                          init_values[0]) *
                    Param::simulation_time_step_;
  init_values[1] += func2(Param::total_steps_ * Param::simulation_time_step_,
                          init_values[1]) *
                    Param::simulation_time_step_;
  init_values[2] += func3(Param::total_steps_ * Param::simulation_time_step_,
                          init_values[2]) *
                    Param::simulation_time_step_;

  EXPECT_NEAR(init_values[0], regulate_genes.substances_[0], 1e-9);
  EXPECT_NEAR(init_values[1], regulate_genes.substances_[1], 1e-9);
  EXPECT_NEAR(init_values[2], regulate_genes.substances_[2], 1e-9);
}

TEST(CellGeneModuleTest, RK4Test) {
  Param::d_e_solve_method_choosed_ = Param::DESolveMethod::kRK4;
  std::vector<double> init_values{{3, 3, 3}};
  int protein_amount = 3;
  Param::total_steps_ = 1;
  std::function<double(double, double)> func1 = [&](
      double curr_time, double substance_) -> double {
    return curr_time * substance_;
  };
  std::function<double(double, double)> func2 = [&](
      double curr_time, double substance_) -> double {
    return curr_time * substance_ + 1;
  };
  std::function<double(double, double)> func3 = [&](
      double curr_time, double substance_) -> double {
    return curr_time * substance_ + 2;
  };

  RegulateGenes regulate_genes;
  regulate_genes.AddFunction(func1, init_values[0]);
  regulate_genes.AddFunction(func2, init_values[1]);
  regulate_genes.AddFunction(func3, init_values[2]);
  Cell cell;
  regulate_genes.Run(&cell);

  vector<double> k1;
  k1.push_back(func1(Param::total_steps_ * Param::simulation_time_step_,
                     init_values[0]));
  k1.push_back(func2(Param::total_steps_ * Param::simulation_time_step_,
                     init_values[1]));
  k1.push_back(func3(Param::total_steps_ * Param::simulation_time_step_,
                     init_values[2]));
  for (int i = 0; i < protein_amount; i++) {
    init_values[i] += Param::simulation_time_step_ * k1[i] / 2.0f;
  }

  vector<double> k2;
  k2.push_back(func1(Param::total_steps_ * Param::simulation_time_step_ +
                         Param::simulation_time_step_ / 2.0f,
                     init_values[0]));
  k2.push_back(func2(Param::total_steps_ * Param::simulation_time_step_ +
                         Param::simulation_time_step_ / 2.0f,
                     init_values[1]));
  k2.push_back(func3(Param::total_steps_ * Param::simulation_time_step_ +
                         Param::simulation_time_step_ / 2.0f,
                     init_values[2]));
  for (int i = 0; i < protein_amount; i++) {
    init_values[i] += Param::simulation_time_step_ * k2[i] / 2.0f -
                      Param::simulation_time_step_ * k1[i] / 2.0f;
  }

  vector<double> k3;
  k3.push_back(func1(Param::total_steps_ * Param::simulation_time_step_ +
                         Param::simulation_time_step_ / 2.0f,
                     init_values[0]));
  k3.push_back(func2(Param::total_steps_ * Param::simulation_time_step_ +
                         Param::simulation_time_step_ / 2.0f,
                     init_values[1]));
  k3.push_back(func3(Param::total_steps_ * Param::simulation_time_step_ +
                         Param::simulation_time_step_ / 2.0f,
                     init_values[2]));
  for (int i = 0; i < protein_amount; i++) {
    init_values[i] += Param::simulation_time_step_ * k3[i] -
                      Param::simulation_time_step_ * k2[i] / 2.0f;
  }

  vector<double> k4;
  k4.push_back(func1(Param::total_steps_ * Param::simulation_time_step_ +
                         Param::simulation_time_step_,
                     init_values[0]));
  k4.push_back(func2(Param::total_steps_ * Param::simulation_time_step_ +
                         Param::simulation_time_step_,
                     init_values[1]));
  k4.push_back(func3(Param::total_steps_ * Param::simulation_time_step_ +
                         Param::simulation_time_step_,
                     init_values[2]));
  for (int i = 0; i < protein_amount; i++) {
    init_values[i] += Param::simulation_time_step_ *
                      (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]) / 6.0f;
  }

  EXPECT_NEAR(init_values[0], regulate_genes.substances_[0], 1e-9);
  EXPECT_NEAR(init_values[1], regulate_genes.substances_[1], 1e-9);
  EXPECT_NEAR(init_values[2], regulate_genes.substances_[2], 1e-9);
}

}  // namespace bdm
#endif  // UNIT_GENE_MODULE_TEST
