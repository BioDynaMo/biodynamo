#include "grid.h"
#include <omp.h>
#include <array>
#include <sstream>
#include "cell.h"
#include "timing.h"

namespace bdm {

using std::array;

int Run(size_t cells_per_dim) {
  const double space = 20;

  // create 3D grid of cells
  auto cells = Cell<>::NewEmptySoa();
  cells.reserve(cells_per_dim * cells_per_dim * cells_per_dim);
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell<Scalar> cell({i * space, j * space, k * space});
        cell.SetDiameter(30);
        cells.push_back(cell);
      }
    }
  }

  auto build_timer = new Timing("build   ");
  Grid grid(cells.GetAllPositions(), 30, Grid::HIGH);
  delete build_timer;

  auto iterate_timer = new Timing("iterate ");
  grid.SetNeighborsWithinRadius(&cells, 700);
  delete iterate_timer;

  return 0;
}

}  // namespace bdm

int main(int argc, const char* argv[]) {
  size_t cells;
  size_t threads;
  std::istringstream(std::string(argv[1])) >> cells;
  std::istringstream(std::string(argv[2])) >> threads;
  omp_set_num_threads(threads);

  return bdm::Run(cells);
}