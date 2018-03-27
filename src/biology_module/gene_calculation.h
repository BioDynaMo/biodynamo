#ifndef BIOLOGY_MODULE_GENE_CALCULATION_H_
#define BIOLOGY_MODULE_GENE_CALCULATION_H_

#include <vector>

#include <Rtypes.h>
#include "biology_module_util.h"
#include "param.h"

namespace bdm {

struct GeneCalculation : public BaseBiologyModule {
  double* time_step = &Param::simulation_time_step_;
  size_t* total_steps_ = &Param::total_steps_;
  std::vector<double> substances_ = {};
  std::vector<std::function<double(double, double)>> functions_ = {};

  GeneCalculation() : BaseBiologyModule(gAllBmEvents) {}

  void AddFunction(std::function<double(double, double)> function,
                   double initial_concentration_) {
    functions_.push_back(function);
    substances_.push_back(initial_concentration_);
  }

  std::vector<double> Calculate(double time, std::vector<double> protein) {
    std::vector<double> update_value;
    for (unsigned int i = 0; i < functions_.size(); i++) {
      update_value.push_back(functions_[i](time, protein[i]));
    }
    return update_value;
  }

  template <typename T>
  void Run(T* cell) {
    if (Param::d_e_solve_method_choosed_ == Param::DESolveMethod::kEuler) {
      // Calculate method needs in current time of simulation
      std::vector<double> update_value =
          Calculate(*total_steps_ * *time_step, substances_);
      for (unsigned int i = 0; i < functions_.size(); i++) {
        substances_[i] += update_value[i] * *time_step;
      }
    } else if (Param::d_e_solve_method_choosed_ == Param::DESolveMethod::kRK4) {
      std::vector<double> k1 =
          Calculate(*total_steps_ * *time_step, substances_);
      for (unsigned int i = 0; i < functions_.size(); i++) {
        substances_[i] += *time_step * k1[i] / 2.0f;
      }
      std::vector<double> k2 = Calculate(
          *total_steps_ * *time_step + *time_step / 2.0f, substances_);
      for (unsigned int i = 0; i < functions_.size(); i++) {
        substances_[i] += *time_step * k2[i] / 2.0f - *time_step * k1[i] / 2.0f;
      }
      std::vector<double> k3 = Calculate(
          *total_steps_ * *time_step + *time_step / 2.0f, substances_);
      for (unsigned int i = 0; i < functions_.size(); i++) {
        substances_[i] += *time_step * k3[i] - *time_step * k2[i] / 2.0f;
      }
      std::vector<double> k4 =
          Calculate(*total_steps_ * *time_step + *time_step, substances_);
      for (unsigned int i = 0; i < functions_.size(); i++) {
        substances_[i] +=
            *time_step * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]) / 6.0f;
      }
    }
  }
  ClassDefNV(GeneCalculation, 1);
};

}  // namespace bdm

#endif  // BIOLOGY_MODULE_GENE_CALCULATION_H_
