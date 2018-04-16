#ifndef INTEGRATION_SOMA_CLUSTERING_H_
#define INTEGRATION_SOMA_CLUSTERING_H_

#include <vector>

#include "biodynamo.h"
#include "math_util.h"
#include "matrix.h"

namespace bdm {

// ----------------------------------------------------------------------------
// Starting with 8 cells, we let each cell grow in volume up until a point
// a cell must divide. This tests whether the GPU accelerated mechanical
// interactions properly handle the creation of new cells.
// -----------------------------------------------------------------------------

inline void EXPECT_ARR_NEAR(const std::array<double, 3>& actual, const std::array<double, 3>& expected, bool* ret) {
  for (size_t i = 0; i < actual.size(); i++) {
    if (std::fabs(expected[i] - actual[i]) > 1e-9) {
      *ret = false;
      std::cout << "Wrong result! Expected " << expected[i] << ", but instead got " << actual[i] << ", which is a difference of " << std::fabs(expected[i] - actual[i]) << ", which is larger than 1e-9" << std::endl;
    }
  }
}

// 2. Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<GrowDivide>;
};

inline void RunTest(bool* result) {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  auto cells = rm->template Get<Cell>();
  // Param::Reset();

  // We need to give every test the same seed for the RNG, because in the cell
  // division, random numbers are used. Within a single executable these numbers
  // vary. Also within the threads this needs to be enforced
#pragma omp parallel for 
  for(int i = 0; i < omp_get_num_threads(); ++i) {
    gRandom.SetSeed(1);
  }

  size_t cells_per_dim = 2;
  auto construct = [](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(30);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.AddBiologyModule(GrowDivide(30.05, 5000, {gAllBmEvents}));
    return cell;
  };

  for (size_t x = 0; x < cells_per_dim; x++) {
    double x_pos = x * 20.0;
    for (size_t y = 0; y < cells_per_dim; y++) {
      double y_pos = y * 20.0;
      for (size_t z = 0; z < cells_per_dim; z++) {
        auto new_simulation_object = construct({x_pos, y_pos, z * 20.0});
        cells->push_back(new_simulation_object);
      }
    }
  }

  // Run for 10 timesteps. In step 2 a division should take place. In step 3
  // these new cells are instantiated
  Scheduler<> scheduler;
  scheduler.Simulate(10);

  EXPECT_ARR_NEAR((*cells)[0].GetPosition(), {4.1399071506916413909, -5.9871942139195297727, 2.8344890446256703065}, result);
  EXPECT_ARR_NEAR((*cells)[1].GetPosition(), {-2.4263219149482031511, -1.4202336557809887019, 29.769029317615839147}, result);
  EXPECT_ARR_NEAR((*cells)[2].GetPosition(), {-4.9118212650644856865, 23.156656083480623209, -9.1231684411316447125}, result);
  EXPECT_ARR_NEAR((*cells)[3].GetPosition(), {4.3076765979041251597, 15.615300607043293368, 25.657658447555828474}, result);
  EXPECT_ARR_NEAR((*cells)[4].GetPosition(), {28.139314619772036963, -0.20987998233654170388, 4.6381417441282613012}, result);
  EXPECT_ARR_NEAR((*cells)[5].GetPosition(), {24.417550786690171094, 3.347525366344008102, 28.067824703341415216}, result);
  EXPECT_ARR_NEAR((*cells)[6].GetPosition(), {16.614520566718258721, 15.828015607618416638, -4.8357284569095106974}, result);
  EXPECT_ARR_NEAR((*cells)[7].GetPosition(), {14.446017269290647889, 22.250832446808978204, 20.180438615017894932}, result);
}

inline int Simulate(int argc, const char** argv) {
  bool result = true;

  // TODO(ahmad): after Trello card ("Fix inconsistency in cell state due to direct updates in Biology Modules")
  // enable multithreading, and adjust results if necessary
  omp_set_num_threads(1);

  // Run CPU version
  RunTest(&result);

  // Run GPU (CUDA) version
  Param::use_gpu_ = true;
  InitializeGPUEnvironment<>();
  RunTest(&result);

  // Run GPU (OpenCL) version
  Param::use_opencl_ = true;
  InitializeGPUEnvironment<>();
  RunTest(&result);

  return !result;
}

}  // namespace bdm

#endif  // INTEGRATION_SOMA_CLUSTERING_H_
