#ifndef DEMO_CELL_CLUSTERING_H_
#define DEMO_CELL_CLUSTERING_H_

#include "biodynamo.h"

namespace bdm {

#define speed 1
#define final_num_cells 65536
// #define CCT 500
// #define L 820
// #define mu 0.1
#define divThreshold 16
#define pathThreshold 160
// #define spatialScale 5.0

BDM_SIM_CLASS(MyCell, Cell) {
  BDM_CLASS_HEADER(MyCellExt, 1, cell_type_, division_count_, my_traveled_path_,
                   sim_step_);

 public:
  MyCellExt() {}
  MyCellExt(const std::array<double, 3>& position) : Cell(position) {}

  void SetCellType(int t) { cell_type_[kIdx] = t; }
  void IncrementDivisionCount() { division_count_[kIdx]++; }
  void UpdateTraveledPath(double t) { my_traveled_path_[kIdx] += t; }
  void UpdateSimStep() { ++sim_step_[kIdx]; }

  int GetCellType() { return cell_type_[kIdx]; }
  int* GetCellTypePtr() { return cell_type_.data(); }
  double GetTraveledPath() { return my_traveled_path_[kIdx]; }
  size_t GetDivisionCount() { return division_count_[kIdx]; }
  int GetSimulationStep() { return sim_step_[0]; }

 private:
  vec<int> cell_type_;
  vec<size_t> division_count_;
  vec<double> my_traveled_path_;
  vec<int> sim_step_;
};

// Helper function: returns norm of input array
static double Norm(double* arr) {
  double array_sum = arr[0] * arr[0] + arr[1] * arr[1] + arr[2] * arr[2];
  return std::sqrt(array_sum);
}

// 1a. Define cell division behaviour:
// Cells divide if they have traveled more than pathThreshold
// and at most divThreshold times
struct DivisionAndMovementModule {
 public:
  template <typename T>
  void Run(T* cell) {
    if (GetNumSimObjects() < final_num_cells) {
      // Initialize variables
      double random_cell_movement[3];

      // Increment cell's path by specified amount
      cell->UpdateTraveledPath(8.0);

      // Generate random cell movement
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<> dis(-0.5, 0.5);
      random_cell_movement[0] = dis(gen);
      random_cell_movement[1] = dis(gen);
      random_cell_movement[2] = dis(gen);
      double normalized_movement = 8.0f / Norm(random_cell_movement);

      std::array<double, 3> displacement = {
          {random_cell_movement[0] * normalized_movement,
           random_cell_movement[1] * normalized_movement,
           random_cell_movement[2] * normalized_movement}};

      // Update cell's position
      cell->UpdatePosition(displacement);

      // Divide if conditions are met
      if (cell->GetTraveledPath() > pathThreshold &&
          cell->GetDivisionCount() < divThreshold) {
        auto& daughter = Divide(*cell);
        daughter.SetCellType((-cell->GetCellType()));
        cell->UpdateTraveledPath(-pathThreshold);
        cell->IncrementDivisionCount();
      }
    }
  }

  // Daughter cells inherit this biology module
  bool IsCopied(Event event) const { return true; }

  ClassDefNV(DivisionAndMovementModule, 1);
};

// 1b. Define displacement behavior:
// Cells move along the diffusion gradient (from low concentration to high)
struct Chemotaxis {
  template <typename T>
  void Run(T* cell) {
    if (GetNumSimObjects() >= final_num_cells) {
      auto dg_0 = GetDiffusionGrid("Substance_0");
      auto dg_1 = GetDiffusionGrid("Substance_1");

      auto& position = cell->GetPosition();
      std::array<double, 3> gradient_0;
      std::array<double, 3> gradient_1;
      dg_0->GetGradient(position, gradient_0);
      dg_1->GetGradient(position, gradient_1);

      std::array<double, 3> diff_gradient;
      for (int i = 0; i < 3; i++) {
        diff_gradient[i] =
            cell->GetCellType() * (gradient_0[i] - gradient_1[i]) * speed;
      }

      cell->UpdatePosition(diff_gradient);
      cell->SetPosition(cell->GetMassLocation());
    }
  }

  // Daughter cells inherit this biology module
  bool IsCopied(Event event) const { return true; }
  ClassDefNV(Chemotaxis, 1);
};

// 1c. Define secretion behavior:
struct SubstanceSecretion {
  template <typename T>
  void Run(T* cell) {
    DiffusionGrid* dg = nullptr;
    if (cell->GetCellType() == 1) {
      dg = GetDiffusionGrid("Substance_1");
    } else {
      dg = GetDiffusionGrid("Substance_0");
    }

    auto& secretion_position = cell->GetPosition();
    dg->IncreaseConcentrationBy(secretion_position, 1);
  }

  // Daughter cells inherit this biology module
  bool IsCopied(Event event) const { return true; }
  ClassDefNV(SubstanceSecretion, 1);
};

// 2. Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules =
      Variant<DivisionAndMovementModule, Chemotaxis, SubstanceSecretion>;
  using AtomicTypes = VariadicTypedef<MyCell>;
};

inline int Simulate(int timesteps) {
  // Create an artificial bounds for the simulation space
  Param::bound_space_ = true;
  Param::lbound_ = 0;
  Param::rbound_ = 1;

  // 3. Define initial model - in this example: two cells
  auto construct = [](const std::array<double, 3>& position) {
    MyCell cell(position);
    cell.SetDiameter(8);
    cell.SetCellType(1);
    cell.AddBiologyModule(SubstanceSecretion());
    cell.AddBiologyModule(Chemotaxis());
    cell.AddBiologyModule(DivisionAndMovementModule());
    return cell;
  };
  std::vector<std::array<double, 3>> positions;
  positions.push_back({0, 0, 0});
  ModelInitializer::CreateCells(positions, construct);

  // 3. Define the substances that cells may secrete
  // This needs to be done AFTER the cells have been specified
  ModelInitializer::DefineSubstance("Substance_0", 0.3, 0.01, 2);
  ModelInitializer::DefineSubstance("Substance_1", 0.3, 0.01, 2);

  // 4. Run simulation for N timesteps
  Param::use_paraview_ = true;
  Param::write_to_file_ = true;
  Param::write_freq_ = 200;
  Scheduler<> scheduler;

  // auto stopping_condition = []() {
  //   std::cout << GetNumSimObjects() << std::endl;
  //   return GetNumSimObjects() > 100;
  // };
  scheduler.Simulate(timesteps);
  return 0;
}

}  // namespace bdm

#endif  // DEMO_CELL_CLUSTERING_H_
