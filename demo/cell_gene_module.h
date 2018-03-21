#ifndef DEMO_CELL_GENE_MODULE_H_
#define DEMO_CELL_GENE_MODULE_H_

#include "biodynamo.h"

namespace bdm {

  using std::array;
  using std::vector;
  using std::string;

  //initial values for list of proteins
  vector<double> init_vals = {12.0, 1.7, 3.4};

  struct GeneBM {
    vector<std::function<double(double, double)>> functions_;

    void addFunction(std::function<double(double, double)> function){
      functions_.push_back(function);
    }
    vector<double> calculate(double time, vector<double> protein){
      assert(functions_.size() == Param::protein_amount && "Amount of proteins is incorrect\n");
      vector<double> update_value;
      for (int i = 0; i < Param::protein_amount; i++){
        update_value.push_back(functions_[i](time, protein[i]));
      }
      return update_value;
    }
  };



  struct GeneCalculation : public BaseBiologyModule {
    double time_step = Param::simulation_time_step_;
    vector<double> substances_;
    GeneBM geneFunction;

    GeneCalculation() :
     BaseBiologyModule(gAllBmEvents), substances_(init_vals){}
    GeneCalculation(vector<double> init_vals) :
     BaseBiologyModule(gAllBmEvents), substances_(init_vals){}
    GeneCalculation(vector<double> init_vals, GeneBM geneFunction) :
     BaseBiologyModule(gAllBmEvents), substances_(init_vals), geneFunction(geneFunction){}

    template <typename T>
    void Run(T* cell) {
      std::cout<< "Step " << Param::total_steps_ <<"\n";
      if (Param::dE_solve_method_choosed == Param::dE_solve_method::Euler){
        // geneFunction.calculate method needs in current time of simulation
        vector<double> update_value = geneFunction.calculate(Param::total_steps_ * time_step, substances_);
        for (int i = 0; i < Param::protein_amount; i++){
          substances_[i] += update_value[i] * time_step;
          std::cout<<substances_[i] << " -- protein " << i <<"\n";
        }
      }
      else if (Param::dE_solve_method_choosed == Param::dE_solve_method::RK4){
        vector<double> k1 = geneFunction.calculate(Param::total_steps_ * time_step, substances_);
        for (int i = 0; i < Param::protein_amount; i++)
          substances_[i] += time_step*k1[i]/2.0f;
        vector<double> k2 = geneFunction.calculate(Param::total_steps_ * time_step + time_step/2.0f, substances_);
        for (int i = 0; i < Param::protein_amount; i++)
          substances_[i] += time_step*k2[i]/2.0f - time_step*k1[i]/2.0f;
        vector<double> k3 = geneFunction.calculate(Param::total_steps_ * time_step + time_step/2.0f, substances_);
        for (int i = 0; i < Param::protein_amount; i++)
          substances_[i] += time_step*k3[i] - time_step*k2[i]/2.0f;
        vector<double> k4 = geneFunction.calculate(Param::total_steps_ * time_step + time_step, substances_);
        for (int i = 0; i < Param::protein_amount; i++){
          substances_[i] += time_step*(k1[i] + 2*k2[i] + 2*k3[i] + k4[i])/6.0f;
          std::cout<<substances_[i] << " -- protein " << i <<"\n";
        }
      }
    }
    ClassDefNV(GeneCalculation, 1);
  };

// 2. Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<GeneCalculation>;
  using AtomicTypes = VariadicTypedef<Cell>;
};

inline int Simulate(int argc, const char** argv) {
  // 3. Initialize BioDynaMo
  InitializeBioDynamo(argc, argv);
  GeneBM geneExmpl;
  geneExmpl.addFunction( [&](double curr_time, double substances_) -> double {
    return 5;
  });
  geneExmpl.addFunction( [&](double curr_time, double substances_) -> double {
    return 5;
  });
  geneExmpl.addFunction( [&](double curr_time, double substances_) -> double {
    return 5;
  });
  size_t cells_per_dim = 1;
  auto construct = [&](const std::array<double, 3>& position) {
    assert(init_vals.size() == Param::protein_amount && "Amount of proteins is incorrect\n");
    Cell cell(position);
    cell.SetDiameter(30);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.AddBiologyModule(GeneCalculation(init_vals,geneExmpl));
    return cell;
  };
  ModelInitializer::Grid3D(cells_per_dim, 20, construct);

    Scheduler<> scheduler;
    scheduler.Simulate(200);
  return 0;
}

}  // namespace bdm

#endif  // DEMO_CELL_GENE_MODULE_H_
