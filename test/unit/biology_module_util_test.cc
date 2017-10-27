#include "unit/biology_module_util_test.h"

namespace bdm {
namespace biology_module_util_test_internal {

TEST(BiologyModuleUtilTest, RunVisitor) { RunRunVisitor(); }

TEST(BiologyModuleUtilTest, CopyVisitorIsCopied) {
  std::vector<Variant<CopyTestBiologyModule>> destination_module_vector;
  CopyVisitor<std::vector<Variant<CopyTestBiologyModule>>> visitor(
      gCellDivision, &destination_module_vector);

  CopyTestBiologyModule module;
  module.expected_event_ = gCellDivision;
  Variant<CopyTestBiologyModule> variant = module;

  gCopyCtorCalled = false;
  visit(visitor, variant);
  EXPECT_EQ(1u, destination_module_vector.size());
  EXPECT_TRUE(gCopyCtorCalled);
}

TEST(BiologyModuleUtilTest, CopyVisitorIsNotCopied) {
  std::vector<Variant<CopyTestBiologyModule>> destination_module_vector;
  CopyVisitor<std::vector<Variant<CopyTestBiologyModule>>> visitor(
      gCellDivision, &destination_module_vector);

  CopyTestBiologyModule module;
  module.expected_event_ = gCellDivision;
  module.is_copied_return_value_ = false;
  Variant<CopyTestBiologyModule> variant = module;

  gCopyCtorCalled = false;
  visit(visitor, variant);
  EXPECT_EQ(0u, destination_module_vector.size());
  EXPECT_FALSE(gCopyCtorCalled);
}

TEST(BaseBiologyModuleTest, CopyNever) {
  BaseBiologyModule bbm;

  for (uint64_t i = 0; i < 64; i++) {
    BmEvent e = 1 << i;
    EXPECT_FALSE(bbm.IsCopied(e));
  }
}

TEST(BaseBiologyModuleTest, CopyAlways) {
  BaseBiologyModule bbm(gAllBmEvents);

  for (uint64_t i = 0; i < 64; i++) {
    BmEvent e = 1 << i;
    EXPECT_TRUE(bbm.IsCopied(e));
  }
}

TEST(BaseBiologyModuleTest, CopyOnSingleEvent) {
  uint64_t one = 1;
  BaseBiologyModule bbm(one << 5);

  for (uint64_t i = 0; i < 64; i++) {
    BmEvent e = one << i;
    if (i != 5) {
      EXPECT_FALSE(bbm.IsCopied(e));
    } else {
      EXPECT_TRUE(bbm.IsCopied(e));
    }
  }
}

TEST(BaseBiologyModuleTest, CopyOnEventList) {
  uint64_t one = 1;
  BaseBiologyModule bbm({one << 5, one << 19, one << 49});

  for (uint64_t i = 0; i < 64; i++) {
    BmEvent e = one << i;
    if (i != 5 && i != 19 && i != 49) {
      EXPECT_FALSE(bbm.IsCopied(e));
    } else {
      EXPECT_TRUE(bbm.IsCopied(e));
    }
  }
}

TEST(UniqueBmEventFactoryTest, All) {
  auto uef = UniqueBmEventFactory::Get();

  auto event_id_1 = uef->NewUniqueBmEvent();
  auto event_id_2 = uef->NewUniqueBmEvent();

  EXPECT_EQ(event_id_1, event_id_2 >> 1);
}

}  // namespace biology_module_util_test_internal
}  // namespace bdm
