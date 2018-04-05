#ifndef DEMO_GENE_REGULATION_H_
#define DEMO_GENE_REGULATION_H_

#include <string>
#include <vector>

#include "biodynamo.h"

namespace bdm {

using std::array;
using std::vector;
using std::string;

// 1. Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<RegulateGenes>;
  using AtomicTypes = VariadicTypedef<Cell>;
};

inline int Simulate(int argc, const char** argv) {
  // 2. Initialize BioDynaMo
  InitializeBioDynamo(argc, argv);

  // 3. Initialize RegulateGenes module.
  // To add functions to the module use RegulateGenes::AddFunction() function.
  // You should pass to the function two variables.
  // The first is of type  std::function<double(double, double)>.
  // This is the function by which concentration of the protein will be
  // calculated.
  // The second is double. This is the initial value for the protein.
  RegulateGenes regulate_example;
  regulate_example.AddFunction(
      [&](double curr_time, double substance) -> double {
        return curr_time * substance + 0.2f;
      },
      1);
  regulate_example.AddFunction(
      [&](double curr_time, double substance) -> double {
        return substance * substance / curr_time;
      },
      5);
  regulate_example.AddFunction(
      [&](double curr_time, double substance) -> double {
        return substance + curr_time + 3;
      },
      7);

  // 4. Define initial model -- in this example just one cell.
  auto construct = [&](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(30);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.AddBiologyModule(regulate_example);
    return cell;
  };
  const std::vector<std::array<double, 3>>& positions = {{0, 0, 0}};
  ModelInitializer::CreateCells(positions, construct);

  // 5. Run simulation for 200 timesteps
  Scheduler<> scheduler;
  scheduler.Simulate(200);
  return 0;
}

}  // namespace bdm

#endif  // DEMO_GENE_REGULATION_H_
