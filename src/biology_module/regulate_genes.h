#ifndef BIOLOGY_MODULE_REGULATE_GENES_H_
#define BIOLOGY_MODULE_REGULATE_GENES_H_

#include <vector>

#include <Rtypes.h>
#include "biology_module_util.h"
#include "param.h"

namespace bdm {
/// This module simulates expression of genes and contains all required
/// additional variables for tracking of the concentration of proteins.
/// Thus, can work with any type of simulation object.
/// It has the implementation of Euler and Runge-Kutta numerical methods
/// for solving ODE. Both methods implemented inside the body of method Run().
///  The user determines which method is picked in particular simulation
/// through variable numerical_de_solver_ from structure Param.

struct RegulateGenes : public BaseBiologyModule {
  /// Variable substances_ stores current value for each simulating protein.
  std::vector<double> substances_ = {};

  /// Variable functions_ stores functions which define how the concentration of
  /// proteins
  /// is being changed. New functions can be added through method AddFunction()
  std::vector<std::function<double(double, double)>> functions_ = {};

  RegulateGenes() : BaseBiologyModule(gAllBmEvents) {}

  explicit RegulateGenes(BmEvent event) : BaseBiologyModule(event) {}

  /// Method AddFunction adds new function to the vector functions_ and
  /// the initial value to the corresponding protein.
  void AddFunction(std::function<double(double, double)> function,
                   double initial_concentration_) {
    functions_.push_back(function);
    substances_.push_back(initial_concentration_);
  }

  /// Method Calculate() takes the current simulation time and last
  /// concentrations of proteins and calculates new values for proteins by
  /// variable functions_
  std::vector<double> const Calculate(double time,
                                      const std::vector<double>& last_value) {
    std::vector<double> updated_value;
    updated_value.reserve(functions_.size());
    for (unsigned int i = 0; i < functions_.size(); i++) {
      updated_value.push_back(functions_[i](time, last_value[i]));
    }
    return updated_value;
  }

  /// Method Run() contains the implementation for Runge-Khutta and Euler
  /// methods for solving ODE.
  template <typename T>
  void Run(T* cell) {
    if (Param::numerical_de_solver_ == Param::NumericalDESolver::kEuler) {
      // Calculate method needs in current time of simulation
      std::vector<double> updated_value = Calculate(
          Param::total_steps_ * Param::simulation_time_step_, substances_);
      for (unsigned int i = 0; i < functions_.size(); i++) {
        substances_[i] += updated_value[i] * Param::simulation_time_step_;
      }
    } else if (Param::numerical_de_solver_ == Param::NumericalDESolver::kRK4) {
      std::vector<double> k1 = Calculate(
          Param::total_steps_ * Param::simulation_time_step_, substances_);
      for (unsigned int i = 0; i < functions_.size(); i++) {
        substances_[i] += Param::simulation_time_step_ * k1[i] / 2.0f;
      }
      std::vector<double> k2 =
          Calculate(Param::total_steps_ * Param::simulation_time_step_ +
                        Param::simulation_time_step_ / 2.0f,
                    substances_);
      for (unsigned int i = 0; i < functions_.size(); i++) {
        substances_[i] += Param::simulation_time_step_ * k2[i] / 2.0f -
                          Param::simulation_time_step_ * k1[i] / 2.0f;
      }
      std::vector<double> k3 =
          Calculate(Param::total_steps_ * Param::simulation_time_step_ +
                        Param::simulation_time_step_ / 2.0f,
                    substances_);
      for (unsigned int i = 0; i < functions_.size(); i++) {
        substances_[i] += Param::simulation_time_step_ * k3[i] -
                          Param::simulation_time_step_ * k2[i] / 2.0f;
      }
      std::vector<double> k4 =
          Calculate(Param::total_steps_ * Param::simulation_time_step_ +
                        Param::simulation_time_step_,
                    substances_);
      for (unsigned int i = 0; i < functions_.size(); i++) {
        substances_[i] += Param::simulation_time_step_ *
                          (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]) / 6.0f;
      }
    }
  }
  ClassDefNV(RegulateGenes, 1);
};

}  // namespace bdm

#endif  // BIOLOGY_MODULE_REGULATE_GENES_H_
