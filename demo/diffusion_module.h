#ifndef DEMO_DIFFUSION_MODULE_H_
#define DEMO_DIFFUSION_MODULE_H_

#include "biodynamo.h"

namespace bdm {

// 1. Define compile time parameter
struct CompileTimeParam : public DefaultCompileTimeParam<> {};

inline int Simulate(const CommandLineOptions& options) {
  // 2. Define initial model - in this example: two cells
  auto construct = [](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(30);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    return cell;
  };
  std::vector<std::array<double, 3>> positions;
  positions.push_back({0, 0, 0});
  positions.push_back({60, 60, 60});
  ModelInitializer::CreateCells(positions, construct);

  // 3. Define the substances that cells secrete
  ModelInitializer::DefineSubstance("Kalium", 0.4);

  // 4. Run simulation for N timesteps
  Param::use_paraview_ = true;
  Scheduler<> scheduler(options.backup_file_, options.restore_file_);
  scheduler.Simulate(10000);
  return 0;
}

}  // namespace bdm

#endif  // DEMO_DIFFUSION_MODULE_H_
