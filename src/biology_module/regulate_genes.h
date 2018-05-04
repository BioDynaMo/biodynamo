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
/// through variable `Param::numerical_ode_solver_`.
struct RegulateGenes : public BaseBiologyModule {
  RegulateGenes() : BaseBiologyModule(gAllBmEvents) {}

  explicit RegulateGenes(BmEvent event) : BaseBiologyModule(event) {}

  /// AddGene adds a new differential equation.
  /// \param first_derivative differential equation in the form:
  ///        `slope = f(time, last_concentration)` -- e.g.:
  ///
  ///              [](double time, double last_concentration) {
  ///                 return 1 - time * last_concentration;
  ///              }
  ///
  /// \param initial_concentration
  void AddGene(std::function<double(double, double)> first_derivative,
               double initial_concentration) {
    first_derivatives_.push_back(first_derivative);
    concentrations_.push_back(initial_concentration);
  }

  const std::vector<double>& GetConcentrations() const {
    return concentrations_;
  }

  /// Method Run() contains the implementation for Runge-Khutta and Euler
  /// methods for solving ODE.
  template <typename T>
  void Run(T* cell) {
    const auto& timestep = Param::simulation_time_step_;
    const auto absolute_time = Param::total_steps_ * timestep;

    if (Param::numerical_ode_solver_ == Param::NumericalODESolver::kEuler) {
      // Euler
      for (uint64_t i = 0; i < first_derivatives_.size(); i++) {
        double slope = first_derivatives_[i](absolute_time, concentrations_[i]);
        concentrations_[i] += slope * timestep;
      }
    } else if (Param::numerical_ode_solver_ ==
               Param::NumericalODESolver::kRK4) {
      // Runge-Kutta 4
      for (uint64_t i = 0; i < first_derivatives_.size(); i++) {
        double interval_midpoint = absolute_time + timestep / 2.0;
        double interval_endpoint = absolute_time + timestep;

        double k1 = first_derivatives_[i](absolute_time, concentrations_[i]);
        double k2 = first_derivatives_[i](
            interval_midpoint, concentrations_[i] + timestep * k1 / 2.0);
        double k3 = first_derivatives_[i](
            interval_midpoint, concentrations_[i] + timestep * k2 / 2.0);
        double k4 = first_derivatives_[i](interval_endpoint,
                                          concentrations_[i] + timestep * k3);

        concentrations_[i] += timestep / 6.0 * (k1 + 2 * k2 + 2 * k3 + k4);
      }
    }
  }

 private:
  /// Store the current concentration for each gene
  std::vector<double> concentrations_ = {};

  /// Store the gene differential equations, which define how the concentration
  /// change.
  /// New functions can be added through method AddGene()
  std::vector<std::function<double(double, double)>> first_derivatives_ = {};

  ClassDefNV(RegulateGenes, 1);
};

}  // namespace bdm

#endif  // BIOLOGY_MODULE_REGULATE_GENES_H_
