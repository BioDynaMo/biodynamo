#ifndef INTEGRATION_SUBSTANCE_INITIALIZATION_H_
#define INTEGRATION_SUBSTANCE_INITIALIZATION_H_

#include <vector>

#include "biodynamo.h"
#include "substance_initializers.h"

namespace bdm {

// -----------------------------------------------------------------------------
// In this integration test we should how to make use of the 'substance
// initializers', in order to initialize the concentration of a particular
// substance. We create a gaussian distribution along each axis.
// -----------------------------------------------------------------------------

// 1. Create list of substances
enum Substances { kSubstance };

// 2. Use default compile-time parameters to let the compiler know we are not
// using any new biology modules or cell types
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {};

inline int Simulate(int argc, const char** argv) {
  InitializeBioDynamo(argc, argv);

  // 3. Define initial model
  // Create an artificial bounds for the simulation space
  Param::bound_space_ = true;
  Param::min_bound_ = 0;
  Param::max_bound_ = 250;

  // Create one cell at a random position
  auto construct = [](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(10);
    return cell;
  };
  ModelInitializer::CreateCellsRandom(Param::min_bound_, Param::max_bound_, 1,
                                      construct);

  // 3. Define the substances in our simulation
  // Order: substance id, substance_name, diffusion_coefficient, decay_constant,
  // resolution
  ModelInitializer::DefineSubstance(kSubstance, "Substance", 0.5, 0.1, 1);

  // Order: substance id, substance name, initialization model, along which axis
  // (0 = x, 1 = y, 2 = z). See the documentation of `GaussianBand` for
  // information about its arguments
  ModelInitializer::InitializeSubstance(kSubstance, "Substance",
                                        GaussianBand(120, 5, Axis::kXAxis));

  // auto initer_y = [](double x, double y, double z) {
  //   return ROOT::Math::normal_pdf(y, 5, 120);
  // };

  // 4. Run simulation for N timesteps
  Scheduler<> scheduler;
  scheduler.Simulate(20);

  return 0;
}

}  // namespace bdm

#endif  // INTEGRATION_SUBSTANCE_INITIALIZATION_H_
