#ifndef DEMO_DIFFUSION_MODULE_H_
#define DEMO_DIFFUSION_MODULE_H_

#include "biodynamo.h"

namespace bdm {

// 1a. Define growth behaviour:
// Cells divide if the diameter reaches a specific value
struct GrowthModule {
  template <typename T>
  void Run(T* cell) {
    if (cell->GetDiameter() <= 32) {
      cell->ChangeVolume(1500);
    } else {
      Divide(*cell);
    }
  }

  bool IsCopied(Event event) const { return true; }
  ClassDefNV(GrowthModule, 1);
};

// 1b. Define displacement behavior:
// Cells move along the diffusion gradient (from low concentration to high)
struct Chemotaxis {
  template <typename T>
  void Run(T* cell) {
    auto dg = GetDiffusionGrid("Kalium");
    dg->SetConcentrationThreshold(1e15);

    auto& position = cell->GetPosition();
    std::array<double, 3> gradient;
    dg->GetGradient(position, gradient);
    gradient[0] *= 0.5;
    gradient[1] *= 0.5;
    gradient[2] *= 0.5;

    cell->UpdatePosition(gradient);
  }

  bool IsCopied(Event event) const { return true; }
  ClassDefNV(Chemotaxis, 1);
};

// 1c. Define secretion behavior:
// One cell is assigned to secrete Kalium artificially at one location
struct KaliumSecretion {
  template <typename T>
  void Run(T* cell) {
    auto dg = GetDiffusionGrid("Kalium");
    array<double, 3> secretion_position = {50, 50, 50};
    dg->IncreaseConcentrationBy(secretion_position, 4);
    std::array<uint32_t, 3> box_coords;
    dg->GetBoxCoords(secretion_position, box_coords);
  }

  bool IsCopied(Event event) const { return false; }
  ClassDefNV(KaliumSecretion, 1);
};

// 2. Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<GrowthModule, Chemotaxis, KaliumSecretion>;
  // use default Backend and AtomicTypes
};

inline int Simulate(const CommandLineOptions& options) {
  // 3. Define initial model - in this example: two cells
  auto construct = [](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(30);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.AddBiologyModule(Chemotaxis());
    cell.AddBiologyModule(GrowthModule());
    if (position[0] == 0 && position[1] == 0 && position[2] == 0) {
      cell.AddBiologyModule(KaliumSecretion());
    }
    return cell;
  };
  std::vector<std::array<double, 3>> positions;
  positions.push_back({0, 0, 0});
  positions.push_back({100, 0, 0});
  positions.push_back({0, 100, 0});
  positions.push_back({0, 0, 100});
  positions.push_back({0, 100, 100});
  positions.push_back({100, 0, 100});
  positions.push_back({100, 100, 0});
  positions.push_back({100, 100, 100});
  ModelInitializer::CreateCells(positions, construct);

  // 3. Define the substances that cells may secrete
  // This needs to be done AFTER the cells have been specified
  ModelInitializer::DefineSubstance("Kalium", 0.4);

  // 4. Run simulation for N timesteps
  Param::use_paraview_ = true;
  Scheduler<> scheduler(options.backup_file_, options.restore_file_);
  scheduler.Simulate(3500);
  return 0;
}

}  // namespace bdm

#endif  // DEMO_DIFFUSION_MODULE_H_
