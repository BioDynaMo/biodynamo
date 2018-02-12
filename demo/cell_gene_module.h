#ifndef DEMO_CELL_GENE_MODULE_H_
#define DEMO_CELL_GENE_MODULE_H_

#include "biodynamo.h"

namespace bdm {

  using std::array;
  using std::vector;
  using std::string;

  //initial values for list of proteins
  const vector<array<double, Param::protein_amount>> init_vals = {{12.0, 1.7, 3.4}};

  // BDM_SIM_OBJECT(GeneCell, Cell) {
  //   BDM_SIM_OBJECT_HEADER(GeneCellExt, 1, substances_);

  //  public:
  //   vec<array<double, protein_amount>> substances_;
  //   GeneCellExt() {}
  //   explicit GeneCellExt(const array<double, 3>& position, array<double, protein_amount> init_substances)
  //    : Base(position), substances_(init_substances){}

  //   virtual ~GeneCellExt() {}

  //   // In this method described functions. These functions determine changes in concentration of proteins.
  //   // You should describe your own function and push it to the update_value vector.
  //   // Value of protein concentration inside substances_ vector.
  //   // update_value.push_back(<"your function">);
  //   vector<double> evolve(double curr_time){
  //     vector<double> update_value;
  //     update_value.push_back(2*substances_[0][0] - curr_time);
  //     update_value.push_back(1/substances_[0][1]);
  //     update_value.push_back(substances_[0][2]);
  //     assert(update_value.size() == protein_amount && "Amount of functions does not equal to amount of proteins\n");
  //     return update_value;
  //   }
  //   void DivideImpl(void* daughter, double volume_ratio, double phi, double theta)
  //     override {
  //       auto daughter_cast = static_cast<Self<Scalar>*>(daughter);
  //       for (int j =  0; j < protein_amount; j++){
  //         daughter_cast->substances_[0][j] = substances_[0][j] * 0.4;
  //         substances_[0][j] = substances_[0][j] * 0.6;
  //       }
  //       // forward call to implementation in CellExt
  //       Base::DivideImpl(daughter_cast, volume_ratio, phi, theta);
  //     }
  // };

  struct GeneCalculation : public BaseBiologyModule {
    double time_step = Param::simulation_time_step_;
    vector<array<double, Param::protein_amount>> substances_;
    // size_t& step = Param::step_global_;

    GeneCalculation() : BaseBiologyModule(gAllBmEvents), substances_(init_vals){}

    template <typename T>
    void Run(T* cell) {
      std::cout<< "Step " << Param::step_global_ <<"\n";
      if (Param::dE_solve_method == "Euler"){
        // Param::functions method needs in current time of simulation
        vector<double> update_value = Param::functions(Param::step_global_ * time_step, substances_);
        for (int i = 0; i < Param::protein_amount; i++){
          substances_[0][i] += update_value[i] * time_step;
          std::cout<<substances_[0][i] << " -- protein " << i <<"\n";
        }
      }
      else if (Param::dE_solve_method == "RK4"){
        vector<double> k1 = Param::functions(Param::step_global_ * time_step, substances_);
        for (int i = 0; i < Param::protein_amount; i++)
          substances_[0][i] += time_step*k1[i]/2.0f;
        vector<double> k2 = Param::functions(Param::step_global_ * time_step + time_step/2.0f, substances_);
        for (int i = 0; i < Param::protein_amount; i++)
          substances_[0][i] += time_step*k2[i]/2.0f - time_step*k1[i]/2.0f;
        vector<double> k3 = Param::functions(Param::step_global_ * time_step + time_step/2.0f, substances_);
        for (int i = 0; i < Param::protein_amount; i++)
          substances_[0][i] += time_step*k3[i] - time_step*k2[i]/2.0f;
        vector<double> k4 = Param::functions(Param::step_global_ * time_step + time_step, substances_);
        for (int i = 0; i < Param::protein_amount; i++){
          substances_[0][i] += time_step*(k1[i] + 2*k2[i] + 2*k3[i] + k4[i])/6.0f;
          std::cout<<substances_[0][i] << " -- protein " << i <<"\n";
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
  size_t cells_per_dim = 1;
  auto construct = [](const std::array<double, 3>& position) {
    assert(init_vals.size() == Param::protein_amount && "Amount of proteins incorrect\n");
    Cell cell(position);
    cell.SetDiameter(30);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.AddBiologyModule(GeneCalculation());
    return cell;
  };
  ModelInitializer::Grid3D(cells_per_dim, 20, construct);

    Scheduler<> scheduler;
    scheduler.Simulate(200);
  return 0;
}

}  // namespace bdm

#endif  // DEMO_CELL_GENE_MODULE_H_
