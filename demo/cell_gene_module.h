#ifndef DEMO_CELL_GENE_MODULE_H_
#define DEMO_CELL_GENE_MODULE_H_

#include <string>
#include <vector>

#include "biodynamo.h"

namespace bdm {

using std::array;
using std::vector;
using std::string;

// 2. Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<GeneCalculation>;
  using AtomicTypes = VariadicTypedef<Cell>;
};

inline int Simulate(int argc, const char** argv) {
  // 3. Initialize BioDynaMo
  InitializeBioDynamo(argc, argv);
  // initial values for list of proteins
  vector<double> init_vals = {12.0, 1.7, 3.4};

  GeneCalculation geneExmpl;
  geneExmpl.AddFunction(
      [&](double curr_time, double substances_) -> double { return 5; }, 1);
  geneExmpl.AddFunction(
      [&](double curr_time, double substances_) -> double { return 5; }, 5);
  geneExmpl.AddFunction(
      [&](double curr_time, double substances_) -> double { return 5; }, 7);
  size_t cells_per_dim = 1;
  auto construct = [&](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(30);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.AddBiologyModule(geneExmpl);
    return cell;
  };
  ModelInitializer::Grid3D(cells_per_dim, 20, construct);

  Scheduler<> scheduler;
  scheduler.Simulate(200);
  return 0;
}

}  // namespace bdm

#endif  // DEMO_CELL_GENE_MODULE_H_
