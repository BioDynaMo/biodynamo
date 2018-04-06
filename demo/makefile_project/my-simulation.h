#ifndef DEMO_MAKEFILE_PROJECT_MY_SIMULATION_H_
#define DEMO_MAKEFILE_PROJECT_MY_SIMULATION_H_

#include "biodynamo.h"

namespace bdm {

// Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {};

inline int Simulate(int argc, const char** argv) {
  InitializeBioDynamo(argc, argv);

  // Define initial model - in this example: single cell at origin
  Cell cell({0, 0, 0});
  cell.SetDiameter(30);
  ResourceManager<>::Get()->push_back(cell);

  // Run simulation for one timestep
  Scheduler<> scheduler;
  scheduler.Simulate(1);

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // DEMO_MAKEFILE_PROJECT_MY_SIMULATION_H_
