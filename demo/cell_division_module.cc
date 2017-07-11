#include "biodynamo.h"

namespace bdm {

// 1. Define growth behaviour
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

// 2. Define compile time parameter
struct CompileTimeParam : public DefaultCompileTimeParam<> {
  using BiologyModules = variant<GrowthModule>;
  // use default Backend and AtomicTypes
};

void Simulate(size_t cells_per_dim = 32) {
  // 3. Define initial model - in this example: 3D grid of cells
  auto cells = ResourceManager<>::Get()->Get<Cell>();
  cells->reserve(cells_per_dim * cells_per_dim * cells_per_dim);
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

  // 4. Run simulation for one timestep
  Scheduler scheduler;
  scheduler.Simulate(1);
}

}  // namespace bdm

int main() { bdm::Simulate(); }
