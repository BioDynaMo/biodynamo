#include "gtest/gtest.h"
#include "cell.h"
#include "grid.h"

namespace bdm {

Cell<Soa> CellFactory(size_t cells_per_dim) {

  const double space = 20;

  auto cells = Cell<>::NewEmptySoa();
  cells.reserve(cells_per_dim * cells_per_dim * cells_per_dim);
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell<Scalar> cell({i * space, j * space, k * space});
        cell.SetDiameter(30);
        cell.UpdateVolume();
        cells.push_back(cell);
      }
    }
  }
  return cells;
}

TEST(GridTest, SetupGrid) {
  auto cells = CellFactory(4);
  Grid grid(cells.GetAllPositions(), 20);

  vector<vector<size_t>> neighbors(cells.GetAllPositions().size());
  auto fill_neighbor_list = [&neighbors] (size_t cell_id, size_t i) { neighbors[i].push_back(cell_id); };
  
  grid.ForEachNeighbor(fill_neighbor_list);

  
  for (auto i: neighbors[4])
    std::cout << i << ' ';
  std::cout << std::endl;

}

}  // namespace bdm