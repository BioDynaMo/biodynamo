#include "neighbor_op.h"
#include "cell.h"
#include "gtest/gtest.h"
#include "inline_vector.h"
#include "neighbor_nanoflann_op.h"
#include "neighbor_pcl_op.h"
#include "test_util.h"

#include <chrono>

namespace bdm {
namespace neighbor_op_test_internal {

template <typename T, typename Op>
void RunTest(T* cells, const Op& op) {
  for (int i = 0; i < 3000; i++) {
    cells->push_back(Cell<>({i*30.0, i*30.0, i*30.0}));
  }

  // execute operation
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  op.Compute(cells);
  std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
  std::cout << "op.Compute = " 
            << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
            << "us\n\n";
  
  // // check results
  // // cell 1
  // InlineVector<int, 8> expected_1;
  // expected_1.push_back(1);
  // EXPECT_EQ(expected_1, (*cells)[0].GetNeighbors());
  // // cell 2
  // InlineVector<int, 8> expected_2;
  // expected_2.push_back(0);
  // expected_2.push_back(2);
  // EXPECT_EQ(expected_2, (*cells)[1].GetNeighbors());
  // // cell 3
  // InlineVector<int, 8> expected_3;
  // expected_3.push_back(1);
  // EXPECT_EQ(expected_3, (*cells)[2].GetNeighbors());
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

TEST(NeighborPclOpTest, ComputeAosoa) {
  std::vector<Cell<Scalar>> cells;
  RunTest(&cells, NeighborPclOp());
}

TEST(NeighborPclOpTest, ComputeSoa) {
  auto cells = Cell<>::NewEmptySoa();
  RunTest(&cells, NeighborPclOp());
}

}  // namespace neighbor_op_test_internal
}  // namespace bdm
