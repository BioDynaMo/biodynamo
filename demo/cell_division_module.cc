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

void Simulate(size_t cells_per_dim = 128) {
  // 3. Define initial model - in this example: 3D grid of cells
  auto construct = [](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(30);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.UpdateVolume();
    cell.AddBiologyModule(GrowthModule());
    return cell;
  };
  ModelInitializer::Grid3D(cells_per_dim, 20, construct);

  // 4. Run simulation for one timestep
  Scheduler scheduler;
  scheduler.Simulate(1);
}

}  // namespace bdm

int main() { bdm::Simulate(); }
