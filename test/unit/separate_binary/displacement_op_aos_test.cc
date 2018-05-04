#include "compile_time_param.h"
#include "gtest/gtest.h"
#include "unit/displacement_op_test.h"

namespace bdm {

template <typename TBackend>
struct CompileTimeParam {
  template <typename TTBackend>
  using Self = CompileTimeParam<TTBackend>;
  using Backend = TBackend;
  using SimulationBackend = Scalar;
  using BiologyModules = Variant<NullBiologyModule>;
  using AtomicTypes = VariadicTypedef<Cell>;
};

namespace displacement_op_test_internal {

TEST(DisplacementOpTest, ComputeAos) { RunTest(); }

}  // namespace displacement_op_test_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
