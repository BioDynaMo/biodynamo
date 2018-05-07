#ifndef DEMO_CELL_DIVISION_MODULE_H_
#define DEMO_CELL_DIVISION_MODULE_H_

#include "biodynamo.h"

namespace bdm {

// -----------------------------------------------------------------------------
// This model creates a grid of 128x128x128 cells. Each cell grows untill a
// specific volume, after which it proliferates (i.e. divides).
// -----------------------------------------------------------------------------

// 1. Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  // use predefined biology module GrowDivide
  using BiologyModules = Variant<GrowDivide>;
  // use default Backend and AtomicTypes
};

inline int Simulate(int argc, const char** argv) {
  // 2. Initialize BioDynaMo
  InitializeBiodynamo(argc, argv);

  // 3. Define initial model - in this example: 3D grid of cells
  size_t cells_per_dim = 128;
  auto construct = [](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(30);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.AddBiologyModule(GrowDivide());
    return cell;
  };
  ModelInitializer::Grid3D(cells_per_dim, 20, construct);

  // 4. Run simulation for one timestep
  Scheduler<> scheduler;
  scheduler.Simulate(1);
  return 0;
}

}  // namespace bdm

#endif  // DEMO_CELL_DIVISION_MODULE_H_
