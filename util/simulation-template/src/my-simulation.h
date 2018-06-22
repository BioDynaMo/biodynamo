#ifndef MY_SIMULATION_H_
#define MY_SIMULATION_H_

#include "biodynamo.h"

namespace bdm {

// Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {};

inline int Simulate(int argc, const char** argv) {
  Simulation<> simulation(argc, argv);

  // Define initial model - in this example: single cell at origin
  auto* rm = simulation.GetResourceManager();
  auto&& cell = rm->New<Cell>(30);

  // Run simulation for one timestep
  simulation.GetScheduler()->Simulate(1);

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

} // namespace bdm

#endif // MY_SIMULATION_H_
