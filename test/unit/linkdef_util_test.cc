#include "linkdef_util.h"
#include <gtest/gtest.h>

namespace bdm {
namespace linkdef_util_test_internal {

struct Has {
  static void AddToLinkDef(std::set<LinkDefDescriptor>& entries) {
    entries.insert({typeid(Has), false});
  }
};

struct HasNot {};

TEST(has_AddToLinkDefTest, All) {
  static_assert(bdm::has_AddToLinkDef<Has>(), "struct 'Has' has a method AddToLinkDef");
  static_assert(!bdm::has_AddToLinkDef<HasNot>(), "struct 'HasNot' does not have a method AddToLinkDef");
}

TEST(CallAddToLinkDef, All) {
  std::set<LinkDefDescriptor> entries;
  CallAddToLinkDef<HasNot>(entries);

  EXPECT_EQ(0u, entries.size());

  CallAddToLinkDef<Has>(entries);

  EXPECT_EQ(1u, entries.size());
}

}  // namespace linkdef_util_test_internal
}  // namespace bdm
