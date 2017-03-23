#include "make_thread_safe.h"

#include <memory>
#include <typeinfo>

#include "gtest/gtest.h"
#include "backend.h"
#include "daosoa.h"

namespace bdm {
namespace make_thread_safe_test_internal {

template <typename TBackend>
struct Widget {
  using Backend = TBackend;

  template <typename T>
  using Self = Widget<T>;

  Widget() {}

  Widget(short i) : id(i) {}

  std::unique_ptr<Self<VcSoaRefBackend>> GetSoaRef() {
    auto ptr = new Widget<VcSoaRefBackend>(123);
    return std::unique_ptr<Self<VcSoaRefBackend>>(ptr);
  }

  const Self<VcSoaRefBackend> GetSoaRef() const {
    return Widget<VcSoaRefBackend>(123);
  }

  mutable short id = 1;
};

TEST(MakeThreadSafeTest, NonConstSoaContainer) {
  Widget<VcSoaBackend> widgets;
  auto thread_safe_widgets = make_thread_safe(&widgets);
  EXPECT_NE(typeid(&widgets).name(), typeid(thread_safe_widgets.get()).name());
  // cast required because pointers should be of different types
  EXPECT_TRUE(static_cast<void*>(&widgets) !=
              static_cast<void*>(thread_safe_widgets.get()));
  EXPECT_EQ(123, thread_safe_widgets->id);
}

TEST(MakeThreadSafeTest, NonConstAosoaContainer) {
  daosoa<Widget<VcSoaBackend>> widgets;
  auto thread_safe_widgets = make_thread_safe(&widgets);
  EXPECT_EQ(typeid(&widgets).name(), typeid(thread_safe_widgets).name());
  EXPECT_TRUE(&widgets == thread_safe_widgets);
}

TEST(MakeThreadSafeTest, ConstSoaContainer) {
  Widget<VcSoaBackend> widgets;
  const auto& thread_safe_widgets = make_thread_safe(widgets);
  EXPECT_NE(typeid(widgets).name(), typeid(thread_safe_widgets).name());
  // cast required because pointers should be of different types
  EXPECT_TRUE(static_cast<const void*>(&widgets) !=
              static_cast<const void*>(&thread_safe_widgets));
  EXPECT_EQ(123, thread_safe_widgets.id);
}

TEST(MakeThreadSafeTest, ConstAosoaContainer) {
  daosoa<Widget<VcSoaBackend>> widgets;
  const auto& thread_safe_widgets = make_thread_safe(widgets);
  EXPECT_EQ(typeid(widgets).name(), typeid(thread_safe_widgets).name());
  EXPECT_TRUE(&widgets == &thread_safe_widgets);
}

}  // namespace make_thread_safe_test_internal
}  // namespace bdm
