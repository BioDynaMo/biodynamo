#include "compile_time_param.h"
#include "transactional_vector.h"
#include "unit/dividing_cell_op_test.h"

namespace bdm {

template <typename TBackend>
struct CompileTimeParam : public DefaultCompileTimeParam<TBackend> {
  using SimulationBackend = Scalar;
};

namespace dividing_cell_op_test_internal {

TEST(DividingCellOpTest, ComputeAos) {
  TransactionalVector<Cell> cells;
  RunTest(&cells);
}

}  // namespace dividing_cell_op_test_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
