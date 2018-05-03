#include "gtest/gtest.h"

#include "cell.h"
#include "compile_time_param.h"

namespace bdm {

template <typename TBackend>
struct CompileTimeParam : public DefaultCompileTimeParam<TBackend> {
  using SimulationBackend = Scalar;
};

namespace simulation_object_util_test_aos_internal {

TEST(SimulationObjectUtilAosTest, RemoveFromSimulation) {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  auto* cells = rm->Get<Cell>();

  cells->push_back(Cell());
  EXPECT_EQ(1u, cells->size());

  auto&& to_be_removed = (*cells)[0];
  to_be_removed.RemoveFromSimulation();
  cells->Commit();
  EXPECT_EQ(0u, cells->size());
}

TEST(SimulationObjectUtilAosTest, GetSoPtr) {
  Rm()->Clear();
  for (uint64_t i = 0; i < 10; i++) {
    Rm()->New<Cell>(1);
  }

  Rm()->Get<Cell>()->Commit();
  EXPECT_EQ(10u, Rm()->GetNumSimObjects());

  auto cells = Rm()->Get<Cell>();
  for (uint64_t i = 0; i < 10; i++) {
    EXPECT_EQ(i, (*cells)[i].GetSoPtr().GetElementIdx());
  }
}

}  // namespace simulation_object_util_test_aos_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
