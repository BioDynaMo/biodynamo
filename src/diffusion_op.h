#ifndef DIFFUSION_OP_H_
#define DIFFUSION_OP_H_

#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "cell.h"
#include "diffusion_grid.h"
#include "grid.h"
#include "inline_vector.h"
#include "resource_manager.h"

using std::setw;
using std::setprecision;

namespace bdm {

// For debugging purposes we can print out the concentration values / gradient
// values of a diffusion grid
template <typename TResourceManager = ResourceManager<>>
void PrintGradient() {
  auto& dg = TResourceManager::Get()->GetDiffusionGrids()[0];
  double* g = dg->GetAllGradients();
  double* c = dg->GetAllConcentrations();
  std::ofstream gfile;
  std::ofstream cfile;
  gfile.open("gradients.csv", std::fstream::out | std::fstream::app);
  cfile.open("concentrations.csv", std::fstream::out | std::fstream::app);
  gfile << std::endl << std::endl;
  cfile << std::endl << std::endl;
  auto nba = dg->GetNumBoxesArray();
  auto l = nba[0] * nba[1];
  // select the layers (in z dimension) of interest
  int il1 = 1;
  int il2 = 2;
  int il3 = 3;
  for (int i = il1*l; i < ((il1+1)*l); i++) {
    cfile << std::setprecision(10) << c[i] << ",";
    gfile << std::setprecision(10)
          << g[3*i]
          << "," << g[3*i + 1]
          << "," << g[3*i + 2] << ", ,";
    if ((i + 1) % nba[0] == 0) {
      gfile << std::endl;
      cfile << std::endl;
    }
  }
  gfile << std::endl << std::endl;
  cfile << std::endl << std::endl;
  for (int i = il2*l; i < ((il2+1)*l); i++) {
    cfile << std::setprecision(10) << c[i] << ",";
    gfile << std::setprecision(10)
          << g[3*i]
          << "," << g[3*i + 1]
          << "," << g[3*i + 2] << ", ,";
    if ((i + 1) % nba[0] == 0) {
      gfile << std::endl;
      cfile << std::endl;
    }
  }

  gfile << std::endl << std::endl;
  cfile << std::endl << std::endl;
  for (int i = il3*l; i < ((il3+1)*l); i++) {
    cfile << std::setprecision(10) << c[i] << ",";
    gfile << std::setprecision(10)
          << g[3*i]
          << "," << g[3*i + 1]
          << "," << g[3*i + 2] << ", ,";
    if ((i + 1) % nba[0] == 0) {
      gfile << std::endl;
      cfile << std::endl;
    }
  }
  gfile << std::endl;
  cfile << std::endl;
  gfile.close();
  cfile.close();
}

/// A class that sets up diffusion grids of the substances in this simulation
template <typename TResourceManager = ResourceManager<>>
class DiffusionOp {
 public:
  DiffusionOp() {}
  virtual ~DiffusionOp() {}

  void MoveAlongDiffusionGradient(DiffusionGrid* dg) {
    auto rm = TResourceManager::Get();
    auto cells = rm->template Get<Cell>();
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      auto&& cell = (*cells)[i];
      auto& cell_pos = cell.GetPosition();
      array<double, 3> gradient;
      dg->GetGradient(cell_pos, gradient);

      // gradient[0] *= 100;
      // gradient[1] *= 100;
      // gradient[2] *= 100;

      std::cout << "Gradient Cell{" << i << "} = [" 
                << gradient[0] << ", "
                << gradient[1] << ", "
                << gradient[2] << "]"
                << std::endl;

      cell.UpdatePosition(gradient);
    }
  }

  template <typename TContainer, typename TGrid = Grid<>>
  void operator()(TContainer* cells, uint16_t type_idx) {
    auto& grid = TGrid::GetInstance();
    auto& diffusion_grids = TResourceManager::Get()->GetDiffusionGrids();
    for (auto dg : diffusion_grids) {
      if (!(dg->IsInitialized())){
        dg->Initialize(grid.GetDimensions(), 0.125*grid.GetBoxLength());
        dg->SetConcentrationThreshold(1e15);
      }

      PrintGradient();

      // Update the diffusion grid dimension if the neighbor grid
      // dimensions have changed
      if (grid.HasGrown()) {
        dg->Update(grid.GetDimensions());
      }

      MoveAlongDiffusionGradient(dg);

      dg->IncreaseConcentrationBy({{30,30,30}}, 4);

      dg->RunDiffusionStep();
      dg->CalculateGradient();
    }
  }
};  

}  // namespace bdm

#endif  // DIFFUSION_OP_H_
