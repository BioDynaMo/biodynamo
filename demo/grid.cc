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
  // cells_per_dim++;

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
  // cells_per_dim++;
  Grid grid(cells.GetAllPositions(), 20);
  delete build_timer;

  // // Lambda that does a simple computation on all neighboring cells
  // std::vector<size_t> sum(cells.size());
  // auto add_cell_ids = [&sum] (size_t cell_id, size_t i) { 
  //   if (cell_id != i) {
  //     sum[i] += cell_id;
  //   }
  // };

  // auto iterate_timer = new Timing("iterate ");
  // grid.ForEachNeighbor(add_cell_ids);
  // delete iterate_timer;

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