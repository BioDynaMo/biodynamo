#include "cell_division_module.h"
#include "cell.h"
#include "scheduler.h"
#include "simulation_object_util.h"

namespace bdm {

// 1. Define growth behaviour in the corresponding header file
//    (separation needed for it be picked up in the IO dictionary)

// 2. Define biology modules that should be used in this simulation
typedef variant<GrowthModule> BiologyModules;

// 3. Use predefined cell class as is and pass biology module definitions
//    Hence the biology module template parameter can be ommitted later on
template <typename Backend = Scalar>
using MyCell = Cell<Backend, BiologyModules>;

void Simulate(size_t cells_per_dim = 128) {
  // 4. Get cell container
  auto cells = ResourceManager<MyCell<Soa>>::Get()->GetCells();
  cells->reserve(cells_per_dim * cells_per_dim * cells_per_dim);

  // 5. Define initial model - in this case 3D grid of cells
  double space = 20;
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        MyCell<Scalar> cell({i * space, j * space, k * space});
        cell.SetDiameter(30);
        cell.SetAdherence(0.4);
        cell.SetMass(1.0);
        cell.UpdateVolume();
        cell.AddBiologyModule(GrowthModule());
        cells->push_back(cell);
      }
    }
  }

  // 6. Run simulation for one timestep
  Scheduler scheduler;
  scheduler.Simulate<MyCell<Soa>>(1);
}

}  // namespace bdm

int main() { bdm::Simulate(); }
