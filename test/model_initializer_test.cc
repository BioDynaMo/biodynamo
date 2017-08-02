#include "model_initializer.h"
#include "gtest/gtest.h"
#include "backend.h"
#include "biology_module_util.h"
#include "cell.h"
#include "resource_manager.h"
#include "variadic_template_parameter_util.h"
#include "test_util.h"

namespace bdm {
namespace model_initializer_test_internal {

TEST(ModelInitializerTest, Grid3D) {
  auto rm = ResourceManager<>::Get();

  ModelInitializer::Grid3D(2, 12, [](const std::array<double, 3>& pos){
    Cell cell(pos);
    return cell;
  });

  auto cells = rm->Get<Cell>();
  EXPECT_EQ(8u, cells->size());
  EXPECT_ARR_EQ({0, 0, 0}, (*cells)[0].GetPosition());
  EXPECT_ARR_EQ({0, 0, 12}, (*cells)[1].GetPosition());
  EXPECT_ARR_EQ({0, 12, 0}, (*cells)[2].GetPosition());
  EXPECT_ARR_EQ({0, 12, 12}, (*cells)[3].GetPosition());
  EXPECT_ARR_EQ({12, 0, 0}, (*cells)[4].GetPosition());
  EXPECT_ARR_EQ({12, 0, 12}, (*cells)[5].GetPosition());
  EXPECT_ARR_EQ({12, 12, 0}, (*cells)[6].GetPosition());
  EXPECT_ARR_EQ({12, 12, 12}, (*cells)[7].GetPosition());
}

}  // namespace model_initializer_test_internal
}  // namespace bdm
