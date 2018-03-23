#ifndef BIOLOGY_MODULE_GENE_CALCULATION_H_
#define BIOLOGY_MODULE_GENE_CALCULATION_H_

#include <iostream>  // TODO remove
#include <vector>

#include <Rtypes.h>
#include "biology_module_util.h"
#include "param.h"

namespace bdm {

struct GeneBM {
  std::vector<std::function<double(double, double)>> functions_;

  void addFunction(std::function<double(double, double)> function){
    functions_.push_back(function);
  }
  std::vector<double> calculate(double time, std::vector<double> protein){
    assert(functions_.size() == Param::protein_amount && "Amount of proteins is incorrect\n");
    std::vector<double> update_value;
    for (int i = 0; i < Param::protein_amount; i++){
      update_value.push_back(functions_[i](time, protein[i]));
    }
    return update_value;
  }
};



struct GeneCalculation : public BaseBiologyModule {
  double time_step = Param::simulation_time_step_;
  std::vector<double> substances_;
  GeneBM geneFunction;

  GeneCalculation() :
   BaseBiologyModule(gAllBmEvents), substances_({12.0, 1.7, 3.4}){}
  GeneCalculation(std::vector<double> init_vals) :
   BaseBiologyModule(gAllBmEvents), substances_(init_vals){}
  GeneCalculation(std::vector<double> init_vals, GeneBM geneFunction) :
   BaseBiologyModule(gAllBmEvents), substances_(init_vals), geneFunction(geneFunction){}

  template <typename T>
  void Run(T* cell) {
    std::cout<< "Step " << Param::total_steps_ <<"\n";
    if (Param::dE_solve_method_choosed == Param::dE_solve_method::Euler){
      // geneFunction.calculate method needs in current time of simulation
      std::vector<double> update_value = geneFunction.calculate(Param::total_steps_ * time_step, substances_);
      for (int i = 0; i < Param::protein_amount; i++){
        substances_[i] += update_value[i] * time_step;
        std::cout<<substances_[i] << " -- protein " << i <<"\n";
      }
    }
    else if (Param::dE_solve_method_choosed == Param::dE_solve_method::RK4){
      std::vector<double> k1 = geneFunction.calculate(Param::total_steps_ * time_step, substances_);
      for (int i = 0; i < Param::protein_amount; i++)
        substances_[i] += time_step*k1[i]/2.0f;
      std::vector<double> k2 = geneFunction.calculate(Param::total_steps_ * time_step + time_step/2.0f, substances_);
      for (int i = 0; i < Param::protein_amount; i++)
        substances_[i] += time_step*k2[i]/2.0f - time_step*k1[i]/2.0f;
      std::vector<double> k3 = geneFunction.calculate(Param::total_steps_ * time_step + time_step/2.0f, substances_);
      for (int i = 0; i < Param::protein_amount; i++)
        substances_[i] += time_step*k3[i] - time_step*k2[i]/2.0f;
      std::vector<double> k4 = geneFunction.calculate(Param::total_steps_ * time_step + time_step, substances_);
      for (int i = 0; i < Param::protein_amount; i++){
        substances_[i] += time_step*(k1[i] + 2*k2[i] + 2*k3[i] + k4[i])/6.0f;
        std::cout<<substances_[i] << " -- protein " << i <<"\n";
      }
    }
  }
  ClassDefNV(GeneCalculation, 1);
};

}  // namespace bdm

#endif  // BIOLOGY_MODULE_GENE_CALCULATION_H_
