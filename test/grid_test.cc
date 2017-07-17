#include "grid.h"
#include "cell.h"
#include "gtest/gtest.h"

namespace bdm {

Cell<Soa> CellFactory(size_t cells_per_dim) {
  const double space = 20;

  auto cells = Cell<>::NewEmptySoa();
  cells.reserve(cells_per_dim * cells_per_dim * cells_per_dim);
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell<Scalar> cell({i * space, j * space, k * space});
        cell.SetDiameter(10);
        cells.push_back(cell);
      }
    }
  }
  return cells;
}

TEST(GridTest, SetupGrid) {
  auto cells = CellFactory(4);
  auto& grid = Grid::GetInstance();
  grid.Initialize(&cells);

  vector<vector<size_t>> neighbors(cells.size());

// Lambda that fills a vector of neighbors for each cell (excluding itself)
#pragma omp parallel for
  for (size_t i = 0; i < cells.size(); i++) {
    auto&& cell = cells[i];
    auto fill_neighbor_list = [&](size_t n) {
      if (cell.id() != n) {
        neighbors[cell.id()].push_back(n);
      }
    };

    grid.ForEachNeighbor(fill_neighbor_list, &cell);
  }

  std::vector<size_t> expected_0 = {1, 4, 5, 16, 17, 20, 21};
  std::vector<size_t> expected_4 = {0, 1, 5, 8, 9, 16, 17, 20, 21, 24, 25};
  std::vector<size_t> expected_42 = {21, 22, 23, 25, 26, 27, 29, 30, 31,
                                     37, 38, 39, 41, 43, 45, 46, 47, 53,
                                     54, 55, 57, 58, 59, 61, 62, 63};
  std::vector<size_t> expected_63 = {42, 43, 46, 47, 58, 59, 62};

  std::sort(neighbors[0].begin(), neighbors[0].end());
  std::sort(neighbors[4].begin(), neighbors[4].end());
  std::sort(neighbors[42].begin(), neighbors[42].end());
  std::sort(neighbors[63].begin(), neighbors[63].end());

  if (expected_0 != neighbors[0]) {
    FAIL();
  }
  if (expected_4 != neighbors[4]) {
    FAIL();
  }
  if (expected_42 != neighbors[42]) {
    FAIL();
  }
  if (expected_63 != neighbors[63]) {
    FAIL();
  }
}

TEST(GridTest, UpdateGrid) {
  auto cells = CellFactory(4);
  auto& positions = cells.GetAllPositions();
  auto& grid = Grid::GetInstance();
  grid.Initialize(&cells);

  // Remove cells 1 and 42 (they are swapped with the last two cells)
  cells.DelayedRemove(1);
  cells.DelayedRemove(42);
  cells.Commit();

  // Update the grid
  grid.UpdateGrid(&cells);

  vector<vector<size_t>> neighbors(positions.size());

// Lambda that fills a vector of neighbors for each cell (excluding itself)
#pragma omp parallel for
  for (size_t i = 0; i < cells.size(); i++) {
    auto&& cell = cells[i];
    auto fill_neighbor_list = [&](size_t n) {
      if (cell.id() != n) {
        neighbors[cell.id()].push_back(n);
      }
    };

    grid.ForEachNeighbor(fill_neighbor_list, &cell);
  }

  std::vector<size_t> expected_0 = {4, 5, 16, 17, 20, 21};
  std::vector<size_t> expected_5 = {0,  2,  4,  6,  8,  9,  10, 16,
                                    17, 18, 20, 21, 22, 24, 25, 26};
  std::vector<size_t> expected_41 = {20, 21, 22, 24, 25, 26, 28, 29, 30,
                                     36, 37, 38, 40, 42, 44, 45, 46, 52,
                                     53, 54, 56, 57, 58, 60, 61};
  std::vector<size_t> expected_61 = {40, 41, 42, 44, 45, 46, 56, 57, 58, 60};

  std::sort(neighbors[0].begin(), neighbors[0].end());
  std::sort(neighbors[5].begin(), neighbors[5].end());
  std::sort(neighbors[41].begin(), neighbors[41].end());
  std::sort(neighbors[61].begin(), neighbors[61].end());

  if (expected_0 != neighbors[0]) {
    FAIL();
  }
  if (expected_5 != neighbors[5]) {
    FAIL();
  }
  if (expected_41 != neighbors[41]) {
    FAIL();
  }
  if (expected_61 != neighbors[61]) {
    FAIL();
  }
}

}  // namespace bdm
