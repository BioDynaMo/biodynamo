#include "simulation_object_vector_test.h"

namespace bdm {
namespace simulation_object_vector_test_internal {

TEST(SimulationObjectVector, All) { RunTest(); }

TEST(GridTest, ClearSuccessors) {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  auto cells = rm->Get<Cell>();

  Cell cell({0, 0, 0});
  cell.SetDiameter(30);
  cells->push_back(cell);

  SimulationObjectVector<SoHandle> successors;
  successors.Initialize();

  successors[SoHandle(0, 0)] = SoHandle(0, 0);

  auto data = successors[SoHandle(0, 0)];
  EXPECT_EQ(0, static_cast<int>(data.GetTypeIdx()));
  EXPECT_EQ(0, static_cast<int>(data.GetElementIdx()));

  successors.Initialize();

  auto max16 = std::numeric_limits<uint16_t>::max();
  auto max32 = std::numeric_limits<uint32_t>::max();

  data = successors[SoHandle(0, 0)];
  EXPECT_EQ(max16, data.GetTypeIdx());
  EXPECT_EQ(max32, data.GetElementIdx());
}

}  // namespace simulation_object_vector_test_internal
}  // namespace bdm
