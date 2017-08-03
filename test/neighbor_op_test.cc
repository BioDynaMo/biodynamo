#include "neighbor_op.h"
#include "cell.h"
#include "gtest/gtest.h"
#include "inline_vector.h"
#include "neighbor_grid_op.h"
#include "neighbor_nanoflann_op.h"
#include "test_util.h"

namespace bdm {
namespace neighbor_op_test_internal {

template <typename TContainer>
void CellFactory(TContainer* cells, size_t cells_per_dim) {
  const double space = 20;

  cells->reserve(cells_per_dim * cells_per_dim * cells_per_dim);
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell<Scalar> cell({k * space, j * space, i * space});
        cell.SetDiameter(30);
        cells->push_back(cell);
      }
    }
  }
}

template <typename T, typename Op>
void RunTest(T* cells, const Op& op) {
  CellFactory(cells, 4);

  // execute operation
  op.Compute(cells);

  std::vector<int> expected_0 = {1, 4, 5, 16, 17, 20};
  std::vector<int> expected_4 = {0, 1, 5, 8, 9, 16, 20, 21, 24};
  std::vector<int> expected_42 = {22, 25, 26, 27, 30, 37, 38, 39, 41, 43, 45, 46, 47, 54, 57, 58, 59, 62};
  std::vector<int> expected_63 = {43, 46, 47, 58, 59, 62};

  auto neighbors_0 = ((*cells)[0].GetNeighbors()).make_std_vector();
  auto neighbors_4 = ((*cells)[4].GetNeighbors()).make_std_vector();
  auto neighbors_42 = ((*cells)[42].GetNeighbors()).make_std_vector();
  auto neighbors_63 = ((*cells)[63].GetNeighbors()).make_std_vector();

  std::sort(neighbors_0.begin(), neighbors_0.end());
  std::sort(neighbors_4.begin(), neighbors_4.end());
  std::sort(neighbors_42.begin(), neighbors_42.end());
  std::sort(neighbors_63.begin(), neighbors_63.end());

  // check results
  EXPECT_EQ(expected_0, neighbors_0);
  EXPECT_EQ(expected_4, neighbors_4);
  EXPECT_EQ(expected_42, neighbors_42);
  EXPECT_EQ(expected_63, neighbors_63);
}

TEST(NeighborOpTest, ComputeAosoa) {
  std::vector<Cell<Scalar>> cells;
  RunTest(&cells, NeighborOp(900));
}

TEST(NeighborOpTest, ComputeSoa) {
  auto cells = Cell<>::NewEmptySoa();
  RunTest(&cells, NeighborOp(900));
}

TEST(NeighborNanoflannOpTest, ComputeAosoa) {
  std::vector<Cell<Scalar>> cells;
  RunTest(&cells, NeighborNanoflannOp(900));
}

TEST(NeighborNanoflannOpTest, ComputeSoa) {
  auto cells = Cell<>::NewEmptySoa();
  RunTest(&cells, NeighborNanoflannOp(900));
}

// todo: make this work with the grid
// TEST(NeighborGridOpTest, ComputeAosoa) {
//   std::vector<Cell<Scalar>> cells;
//   RunTest(&cells, NeighborGridOp());
// }

TEST(NeighborGridOpTest, ComputeSoa) {
  auto cells = Cell<>::NewEmptySoa();
  RunTest(&cells, NeighborGridOp(Grid::kHigh, true, 900));
}

}  // namespace neighbor_op_test_internal
}  // namespace bdm
