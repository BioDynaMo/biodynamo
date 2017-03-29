#include <gtest/gtest.h>
#include "cell.h"
#include "displacement_op.h"
#include "test_util.h"

namespace bdm {
namespace displacement_op_test_internal {

template <typename T>
void RunTest(T* cells) {
  using real_v = VcVectorBackend::real_v;
  using real_t = real_v::value_type;
  if (real_v::Size < 2) {
    FAIL() << "Backend must at least support two elements for this test";
  }
  // set up cells
  real_v diameter((const real_t[]){9, 11});
  real_v adherence((const real_t[]){0.3, 0.4});
  real_v mass((const real_t[]){1.4, 1.1});

  std::array<real_v, 3> position = {real_v((const real_t[]){0, 0}),
                                    real_v((const real_t[]){0, 5}),
                                    real_v((const real_t[]){0, 0})};

  // TODO(lukas) generate target values with this tf
  // std::array<real_v, 3> tractor_force = {
  //     real_v((const real_t[]){0.99, 1.01}),
  //     real_v((const real_t[]){0.98, 1.02}),
  //     real_v((const real_t[]){0.97, 1.03})
  // };

  InlineVector<int, 8> neighbor_1;
  neighbor_1.push_back(1);
  InlineVector<int, 8> neighbor_2;
  neighbor_2.push_back(0);
  std::array<InlineVector<int, 8>, VcVectorBackend::kVecLen> neighbors = {neighbor_1,
                                                                    neighbor_2};

  Cell<VcVectorBackend> cell(diameter);
  cell.SetDiameter(diameter);
  cell.SetPosition(position);
  cell.SetMassLocation(position);
  // cell.SetTractorForce(tractor_force);
  cell.SetAdherence(adherence);
  cell.SetMass(mass);
  cell.SetNeighbors(neighbors);
  cells->push_back(cell);

  // execute operation
  DisplacementOp op;
  op.Compute(cells);

  // check results
  auto& final_position = (*cells)[0].GetPosition();
  // cell 1
  EXPECT_NEAR(0, final_position[0][0], abs_error<real_t>::value);
  EXPECT_NEAR(-0.07797206232558615, final_position[1][0],
              abs_error<real_t>::value);
  EXPECT_NEAR(0, final_position[2][0], abs_error<real_t>::value);
  // cell 2
  EXPECT_NEAR(0, final_position[0][1], abs_error<real_t>::value);
  EXPECT_NEAR(5.0992371702325645, final_position[1][1],
              abs_error<real_t>::value);
  EXPECT_NEAR(0, final_position[2][1], abs_error<real_t>::value);

  // check if tractor_force has been reset to zero
  auto& final_tf = (*cells)[0].GetTractorForce();
  EXPECT_NEAR(0, final_tf[0].sum(), abs_error<real_t>::value);
  EXPECT_NEAR(0, final_tf[1].sum(), abs_error<real_t>::value);
  EXPECT_NEAR(0, final_tf[2].sum(), abs_error<real_t>::value);

  // remaining fields should remain unchanged
  EXPECT_TRUE((diameter == (*cells)[0].GetDiameter()).isFull());
  EXPECT_TRUE((adherence == (*cells)[0].GetAdherence()).isFull());
  EXPECT_TRUE((mass == (*cells)[0].GetMass()).isFull());
}

TEST(DisplacementOpTest, ComputeAosoa) {
  daosoa<Cell<VcVectorBackend>> cells;
  RunTest(&cells);
}

TEST(DisplacementOpTest, ComputeSoa) {
  auto cells = Cell<>::NewEmptySoa();
  RunTest(&cells);
}

}  // namespace displacement_op_test_internal
}  // namespace bdm
