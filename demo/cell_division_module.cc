#include "biodynamo.h"

namespace bdm {

// 1. Define atomic simulation objects which should be used in this simulation
BDM_DEFINE_ATOMIC_TYPES(Cell);
// 2. Define backend
BDM_DEFINE_BACKEND(Soa);

// 3. Define growth behaviour
struct GrowthModule {
  template <typename T>
  void Run(T* cell) {
    if (cell->GetDiameter() <= 40) {
      cell->ChangeVolume(300);
    } else {
      Divide(*cell);
    }
  }

  bool IsCopied(Event event) const { return true; }
};

// 5. Define biology modules that should be used in this simulation
//    Simulation objects automatically pick up this definition
BDM_DEFINE_BIOLOGY_MODULES(GrowthModule);

void Simulate(size_t cells_per_dim = 128) {
  // 5. Get cell container
  auto cells = ResourceManager<>::Get()->Get<Cell>();
  cells->reserve(cells_per_dim * cells_per_dim * cells_per_dim);

  // 6. Define initial model - in this case 3D grid of cells
  double space = 20;
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell cell({i * space, j * space, k * space});
        cell.SetDiameter(30);
        cell.SetAdherence(0.4);
        cell.SetMass(1.0);
        cell.UpdateVolume();
        cell.AddBiologyModule(GrowthModule());
        cells->push_back(cell);
      }
    }
  }

  // 7. Run simulation for one timestep
  Scheduler scheduler;
  scheduler.Simulate(1);
}

}  // namespace bdm

int main() { bdm::Simulate(); }
