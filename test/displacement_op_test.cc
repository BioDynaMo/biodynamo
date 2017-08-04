#include "displacement_op.h"
#include "cell.h"
#include "grid.h"
#include "gtest/gtest.h"
#include "test_util.h"

namespace bdm {
namespace displacement_op_test_internal {

template <typename T>
void RunTest(T* cells) {
  // Cell 1
  Cell cell;
  cell.SetAdherence(0.3);
  cell.SetDiameter(9);
  cell.SetMass(1.4);
  cell.SetPosition({0, 0, 0});
  cell.SetMassLocation({0, 0, 0});
  // cell.SetTractorForce(tractor_force);
  InlineVector<int, 8> neighbor_1;
  neighbor_1.push_back(1);
  cells->push_back(cell);

  // Cell 2
  cell.SetAdherence(0.4);
  cell.SetDiameter(11);
  cell.SetMass(1.1);
  cell.SetPosition({0, 5, 0});
  cell.SetMassLocation({0, 5, 0});
  // cell.SetTractorForce(tractor_force);
  InlineVector<int, 8> neighbor_2;
  neighbor_2.push_back(0);
  cells->push_back(cell);

  auto& grid = Grid::GetInstance();
  grid.Initialize(*cells);

  // execute operation
  DisplacementOp op;
  op.Compute(cells);

  // check results
  // cell 1
  auto final_position = (*cells)[0].GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(-0.07797206232558615, final_position[1],
              abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);
  // cell 2
  final_position = (*cells)[1].GetPosition();
  EXPECT_NEAR(0, final_position[0], abs_error<double>::value);
  EXPECT_NEAR(5.0992371702325645, final_position[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_position[2], abs_error<double>::value);

  // check if tractor_force has been reset to zero
  // cell 1
  auto final_tf = (*cells)[0].GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);
  // cell 2
  final_tf = (*cells)[1].GetTractorForce();
  EXPECT_NEAR(0, final_tf[0], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[1], abs_error<double>::value);
  EXPECT_NEAR(0, final_tf[2], abs_error<double>::value);

  // remaining fields should remain unchanged
  // cell 1
  EXPECT_NEAR(0.3, (*cells)[0].GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(9, (*cells)[0].GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.4, (*cells)[0].GetMass(), abs_error<double>::value);
  // cell 2
  EXPECT_NEAR(0.4, (*cells)[1].GetAdherence(), abs_error<double>::value);
  EXPECT_NEAR(11, (*cells)[1].GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(1.1, (*cells)[1].GetMass(), abs_error<double>::value);
}

TEST(DisplacementOpTest, ComputeAosoa) {
  std::vector<Cell> cells;
  RunTest(&cells);
}

TEST(DisplacementOpTest, ComputeSoa) {
  auto cells = Cell::NewEmptySoa();
  RunTest(&cells);
}

}  // namespace displacement_op_test_internal
}  // namespace bdm
