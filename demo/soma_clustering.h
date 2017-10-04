#ifndef DEMO_CELL_CLUSTERING_H_
#define DEMO_CELL_CLUSTERING_H_

#include "biodynamo.h"

#include <cstdlib>
#include <ctime>

namespace bdm {

#define SPEED 5
#define FINAL_NUM_CELLS 65536
// #define CCT 500
// #define L 820
// #define mu 0.1
#define DIV_THRESHOLD 16
#define PATH_THRESHOLD 2
// #define spatialScale 5.0

BDM_SIM_CLASS(MyCell, Cell) {
  BDM_CLASS_HEADER(MyCellExt, 1, cell_type_, division_count_, my_traveled_path_);

 public:
  MyCellExt() {}
  MyCellExt(const std::array<double, 3>& position) : Cell(position) {}

  void SetCellType(int t) { cell_type_[kIdx] = t; }
  void IncrementDivisionCount() { division_count_[kIdx]++; }
  void UpdateTraveledPath(double t) { my_traveled_path_[kIdx] += t; }

  int GetCellType() { return cell_type_[kIdx]; }
  int* GetCellTypePtr() { return cell_type_.data(); }
  double GetTraveledPath() { return my_traveled_path_[kIdx]; }
  size_t GetDivisionCount() { return division_count_[kIdx]; }

 private:
  vec<int> cell_type_;
  vec<size_t> division_count_;
  vec<double> my_traveled_path_;
};

// Helper function: returns norm of input array
static double Norm(double* arr) {
  double array_sum = arr[0] * arr[0] + arr[1] * arr[1] + arr[2] * arr[2];
  return std::sqrt(array_sum);
}

// 1a. Define cell division behaviour:
// Cells divide if they have traveled more than PATH_THRESHOLD
// and at most DIV_THRESHOLD times
struct DivisionAndMovementModule {
 public:
  template <typename T>
  void Run(T* cell) {
    if (GetNumSimObjects() < FINAL_NUM_CELLS) {
      // Initialize variables
      double random_cell_movement[3];

      // Generate random cell movement
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<> dis(-0.5, 0.5);
      random_cell_movement[0] = dis(gen);
      random_cell_movement[1] = dis(gen);
      random_cell_movement[2] = dis(gen);
      double normalized_movement = 8.0f / Norm(random_cell_movement);

      std::array<double, 3> displacement =
      { {random_cell_movement[0] * normalized_movement,
         random_cell_movement[1] * normalized_movement,
         random_cell_movement[2] * normalized_movement} };

      // Update cell's position
      cell->UpdatePosition(displacement);
    }
  }

  // Daughter cells inherit this biology module
  bool IsCopied(Event event) const { return true; }

  ClassDefNV(DivisionAndMovementModule, 1);
};

// // 1b. Define displacement behavior:
// // Cells move along the diffusion gradient (from low concentration to high)
// struct Chemotaxis {
//   template <typename T>
//   void Run(T* cell) {
//     auto dg_0 = GetDiffusionGrid("Substance_0");
//     auto dg_1 = GetDiffusionGrid("Substance_1");

//     auto& position = cell->GetPosition();
//     std::array<double, 3> gradient_0;
//     std::array<double, 3> gradient_1;
//     dg_0->GetGradient(position, gradient_0);
//     dg_1->GetGradient(position, gradient_1);

//     std::array<double, 3> diff_gradient;
//     for (int i = 0; i < 3; i++) {
//       diff_gradient[i] = cell->GetCellType()*(gradient_0[i] - gradient_1[i])*SPEED;
//     }

//     cell->UpdatePosition(diff_gradient);
//     cell->SetPosition(cell->GetMassLocation());
//   }

//   // Daughter cells inherit this biology module
//   bool IsCopied(Event event) const { return true; }
//   ClassDefNV(Chemotaxis, 1);
// };

// 1b. Define displacement behavior:
// Cells move along the diffusion gradient (from low concentration to high)
struct Chemotaxis {
  template <typename T>
  void Run(T* cell) {
    auto dg_0 = GetDiffusionGrid("Substance_0");
    auto dg_1 = GetDiffusionGrid("Substance_1");

    auto& position = cell->GetPosition();
    std::array<double, 3> gradient_0;
    std::array<double, 3> gradient_1;
    dg_0->GetGradient(position, gradient_0);
    dg_1->GetGradient(position, gradient_1);

    std::array<double, 3> diff_gradient;

    if (cell->GetCellType() == 1) {
      for (int i = 0; i < 3; i++) {
        diff_gradient[i] = gradient_1[i]*SPEED;
      }
    } else {
      for (int i = 0; i < 3; i++) {
        diff_gradient[i] = gradient_0[i]*SPEED;
      }
    }

    cell->UpdatePosition(diff_gradient);
    cell->SetPosition(cell->GetMassLocation());
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

// 1d. Limit cell positions within bounding box
struct BoundingBox {
  template <typename T>
  void Run(T* cell) {
    double lbound = 0;
    double rbound = 100;
    cell->SetPosition(cell->GetMassLocation());
    auto& pos = cell->GetPosition();
    for (int i = 0; i < 3; i++) {
      if (pos[i] < lbound) {
        cell->SetCoordinate(i, lbound);
      }
      if (pos[i] > rbound) {
        cell->SetCoordinate(i, rbound);
      }
    }
    cell->SetPosition(cell->GetMassLocation());
  }
  bool IsCopied(Event event) const { return true; }
  ClassDefNV(BoundingBox, 1);
};

// 2. Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<DivisionAndMovementModule, Chemotaxis,
                                 SubstanceSecretion, BoundingBox>;
  using AtomicTypes = VariadicTypedef<MyCell>;
};

inline int Simulate(const CommandLineOptions& options) {
  // 3. Define initial model - in this example: two cells
  auto construct_0 = [](const std::array<double, 3>& position) {
    MyCell cell(position);
    cell.SetDiameter(10);
    cell.SetCellType(1);
    cell.AddBiologyModule(BoundingBox());
    cell.AddBiologyModule(SubstanceSecretion());
    cell.AddBiologyModule(Chemotaxis());
    cell.AddBiologyModule(BoundingBox());
    return cell;
  };
  srand(static_cast<unsigned>(time(0)));
  ModelInitializer::CreateCellsRandom(0, 100, 120, construct_0);

  auto construct_1 = [](const std::array<double, 3>& position) {
    MyCell cell(position);
    cell.SetDiameter(10);
    cell.SetCellType(-1);
    cell.AddBiologyModule(BoundingBox());
    cell.AddBiologyModule(SubstanceSecretion());
    cell.AddBiologyModule(Chemotaxis());
    cell.AddBiologyModule(BoundingBox());
    return cell;
  };
  ModelInitializer::CreateCellsRandom(0, 100, 120, construct_1);

  // 3. Define the substances that cells may secrete
  // This needs to be done AFTER the cells have been specified
  ModelInitializer::DefineSubstance("Substance_0", 0.3);
  ModelInitializer::DefineSubstance("Substance_1", 0.3);

  // 4. Run simulation for N timesteps
  Param::use_paraview_ = true;
  Param::write_to_file_ = true;
  Param::write_freq_ = 10;
  Scheduler<> scheduler;

  scheduler.Simulate(1000);
  return 0;
}

}  // namespace bdm

#endif  // DEMO_CELL_CLUSTERING_H_
