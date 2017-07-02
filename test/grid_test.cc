#include "gtest/gtest.h"
#include "cell.h"
#include "grid.h"

#include <vector>

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

  // Lambda that fills a vector of neighbors for each cell (excluding itself)
  auto fill_neighbor_list = [&neighbors] (size_t cell_id, size_t i) { 
    if (cell_id != i) {
      neighbors[i].push_back(cell_id); 
    }
  };
  
  grid.ForEachNeighbor(fill_neighbor_list);

  std::vector<size_t> expected_0 = {1, 4, 5, 16, 17, 20, 21};
  std::vector<size_t> expected_4 = {0, 1, 5, 8, 9, 16, 17, 20, 21, 24, 25};
  std::vector<size_t> expected_42 = {21, 22, 23, 25, 26, 27, 29, 30, 31, 37, 38, 39, 41, 43, 45, 46, 47, 53, 54, 55, 57, 58, 59, 61, 62, 63};
  std::vector<size_t> expected_63 =  {42, 43, 46, 47, 58, 59, 62};

  std::sort(neighbors[0].begin(), neighbors[0].end());
  std::sort(neighbors[4].begin(), neighbors[4].end());
  std::sort(neighbors[42].begin(), neighbors[42].end());
  std::sort(neighbors[63].begin(), neighbors[63].end());

  if (expected_0 != neighbors[0]) { FAIL(); }
  if (expected_4 != neighbors[4]) { FAIL(); }
  if (expected_42 != neighbors[42]) { FAIL(); }
  if (expected_63 != neighbors[63]) { FAIL(); }
}

}  // namespace bdm