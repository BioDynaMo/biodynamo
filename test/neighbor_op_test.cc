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

  std::vector<int> expected_0 = {1,  2,  4,  5,  6,  8,  9,  16, 17, 18,
                                 20, 21, 22, 24, 25, 32, 33, 36, 37};
  std::vector<int> expected_4 = {0,  1,  2,  5,  6,  8,  9,  10, 12,
                                 13, 16, 17, 18, 20, 21, 22, 24, 25,
                                 26, 28, 29, 32, 33, 36, 37, 40, 41};
  std::vector<int> expected_42 = {
      5,  6,  7,  9,  10, 11, 13, 14, 15, 17, 18, 19, 20, 21, 22, 23, 24, 25,
      26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38, 39, 40, 41, 43, 44, 45,
      46, 47, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
  std::vector<int> expected_63 = {26, 27, 30, 31, 38, 39, 41, 42, 43, 45,
                                  46, 47, 54, 55, 57, 58, 59, 61, 62};

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
  RunTest(&cells, NeighborOp());
}

TEST(NeighborOpTest, ComputeSoa) {
  auto cells = Cell<>::NewEmptySoa();
  RunTest(&cells, NeighborOp());
}

TEST(NeighborNanoflannOpTest, ComputeAosoa) {
  std::vector<Cell<Scalar>> cells;
  RunTest(&cells, NeighborNanoflannOp());
}

TEST(NeighborNanoflannOpTest, ComputeSoa) {
  auto cells = Cell<>::NewEmptySoa();
  RunTest(&cells, NeighborNanoflannOp());
}

// TEST(NeighborGridOpTest, ComputeAosoa) {
//   std::vector<Cell<Scalar>> cells;
//   RunTest(&cells, NeighborGridOp());
// }

TEST(NeighborGridOpTest, ComputeSoa) {
  auto cells = Cell<>::NewEmptySoa();
  RunTest(&cells, NeighborGridOp());
}

}  // namespace neighbor_op_test_internal
}  // namespace bdm
