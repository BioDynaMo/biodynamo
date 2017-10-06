// I/O related code must be in header file
#include "unit/resource_manager_test.h"
#include "cell.h"

namespace bdm {
namespace resource_manager_test_internal {

/// Create ResourceManager with two types, use Get function to obtain container
/// of the specified type, push_back values and check if they have correctly
/// been added inside the ResourceManager
/// @tparam A type one: scalar or soa backend
/// @tparam B type two: scalar or soa backend
template <typename Backend, typename A, typename B>
inline void RunGetTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, A, B>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  // template specifier needed because A is dependant type
  auto a_vector = rm->template Get<A>();
  EXPECT_EQ(0u, a_vector->size());
  a_vector->push_back(AScalar(12));
  a_vector->push_back(AScalar(34));
  EXPECT_EQ(12, (*rm->template Get<A>())[0].GetData());
  EXPECT_EQ(34, (*rm->template Get<A>())[1].GetData());
  EXPECT_EQ(2u, rm->template Get<A>()->size());

  auto b_vector = rm->template Get<B>();
  EXPECT_EQ(0u, b_vector->size());
  b_vector->push_back(BScalar(3.14));
  b_vector->push_back(BScalar(6.28));
  EXPECT_NEAR(3.14, (*rm->template Get<B>())[0].GetData(), kEpsilon);
  EXPECT_NEAR(6.28, (*rm->template Get<B>())[1].GetData(), kEpsilon);
  EXPECT_EQ(2u, rm->template Get<B>()->size());
}

TEST(ResourceManagerTest, GetAos) {
  RunGetTest<Scalar, AScalar, BScalar>();
  RunGetTest<Scalar, ASoa, BSoa>();
}

TEST(ResourceManagerTest, GetSoa) {
  RunGetTest<Soa, AScalar, BScalar>();
  RunGetTest<Soa, ASoa, BSoa>();
}

template <typename Backend, typename A, typename B>
inline void RunApplyOnElementTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, A, B>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<A>();
  a_collection->push_back(AScalar(12));
  a_collection->push_back(AScalar(34));
  rm->ApplyOnElement(SoHandle(0, 1),
                     [](auto& element) { EXPECT_EQ(34, element.GetData()); });

  auto b_collection = rm->template Get<B>();
  b_collection->push_back(BScalar(3.14));
  b_collection->push_back(BScalar(6.28));
  rm->ApplyOnElement(SoHandle(1, 0), [&](auto& element) {
    EXPECT_NEAR(3.14, element.GetData(), kEpsilon);
  });
}

TEST(ResourceManagerTest, ApplyOnElementAos) {
  RunApplyOnElementTest<Scalar, AScalar, BScalar>();
  RunApplyOnElementTest<Scalar, ASoa, BSoa>();
}

TEST(ResourceManagerTest, ApplyOnElementSoa) {
  RunApplyOnElementTest<Soa, AScalar, BScalar>();
  RunApplyOnElementTest<Soa, ASoa, BSoa>();
}

template <typename Backend, typename A, typename B>
void RunApplyOnAllElementsTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, A, B>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<A>();
  a_collection->push_back(AScalar(12));
  a_collection->push_back(AScalar(34));

  auto b_collection = rm->template Get<B>();
  b_collection->push_back(BScalar(3.14));
  b_collection->push_back(BScalar(6.28));
  size_t counter = 0;
  rm->ApplyOnAllElements([&](auto& element, SoHandle&& handle) {  // NOLINT
    counter++;
    switch (counter) {
      case 1:
        EXPECT_EQ(12, element.GetData());
        break;
      case 2:
        EXPECT_EQ(34, element.GetData());
        break;
      case 3:
        EXPECT_NEAR(3.14, element.GetData(), kEpsilon);
        break;
      case 4:
        EXPECT_NEAR(6.28, element.GetData(), kEpsilon);
        break;
    }
  });

  EXPECT_EQ(4u, counter);
}

TEST(ResourceManagerTest, ApplyOnAllElementsAos) {
  RunApplyOnAllElementsTest<Scalar, AScalar, BScalar>();
  RunApplyOnAllElementsTest<Scalar, ASoa, BSoa>();
}

TEST(ResourceManagerTest, ApplyOnAllElementsSoa) {
  RunApplyOnAllElementsTest<Soa, ASoa, BSoa>();
  RunApplyOnAllElementsTest<Soa, AScalar, BScalar>();
}

template <typename Backend, typename A, typename B>
void RunGetNumSimObjects() {
  using CTParam = CompileTimeParam<Backend, A, B>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<A>();
  a_collection->push_back(AScalar(12));
  a_collection->push_back(AScalar(34));
  a_collection->push_back(AScalar(59));

  auto b_collection = rm->template Get<B>();
  b_collection->push_back(BScalar(3.14));
  b_collection->push_back(BScalar(6.28));

  EXPECT_EQ(5u, rm->GetNumSimObjects());
}

TEST(ResourceManagerTest, GetNumSimObjectsAos) {
  RunGetNumSimObjects<Scalar, AScalar, BScalar>();
  RunGetNumSimObjects<Scalar, ASoa, BSoa>();
}

TEST(ResourceManagerTest, GetNumSimObjectsSoa) {
  RunGetNumSimObjects<Soa, ASoa, BSoa>();
  RunGetNumSimObjects<Soa, AScalar, BScalar>();
}

// This test uses Cells since ASoa, BSoa are strippted down simulatio objects
// and are themselves not thread safe.
template <typename Backend>
void RunApplyOnAllElementsParallelTest() {
  using CTParam = CompileTimeParam<Backend, Cell>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto cells = rm->template Get<Cell>();
  cells->push_back(Cell(3.14));
  cells->push_back(Cell(6.28));
  cells->push_back(Cell(9.42));

  rm->ApplyOnAllElementsParallel([](auto&& element, SoHandle handle) {
    const double kEpsilon = abs_error<double>::value;
    if (handle == SoHandle(0, 0)) {
      EXPECT_EQ(3.14, element.GetDiameter());
    } else if (handle == SoHandle(0, 1)) {
      EXPECT_EQ(6.28, element.GetDiameter());
    } else if (handle == SoHandle(0, 2)) {
      EXPECT_NEAR(9.42, element.GetDiameter(), kEpsilon);
    } else {
      FAIL();
    }
  });
}

TEST(ResourceManagerTest, ApplyOnAllElementsParallelAos) {
  RunApplyOnAllElementsParallelTest<Scalar>();
}

TEST(ResourceManagerTest, ApplyOnAllElementsParallelSoa) {
  RunApplyOnAllElementsParallelTest<Soa>();
}

template <typename Backend, typename A, typename B>
void RunApplyOnAllTypesTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, A, B>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<A>();
  a_collection->push_back(AScalar(12));

  auto b_collection = rm->template Get<B>();
  b_collection->push_back(BScalar(3.14));
  b_collection->push_back(BScalar(6.28));
  size_t counter = 0;
  rm->ApplyOnAllTypes([&](auto* container, uint16_t type_idx) {
    counter++;
    switch (counter) {
      case 1:
        EXPECT_EQ(1u, container->size());
        EXPECT_EQ(12, (*container)[0].GetData());
        break;
      case 2:
        EXPECT_EQ(2u, container->size());
        EXPECT_NEAR(3.14, (*container)[0].GetData(), kEpsilon);
        EXPECT_NEAR(6.28, (*container)[1].GetData(), kEpsilon);
        break;
    }
  });

  EXPECT_EQ(2u, counter);
}

TEST(ResourceManagerTest, ApplyOnAllTypesAos) {
  RunApplyOnAllTypesTest<Scalar, AScalar, BScalar>();
  RunApplyOnAllTypesTest<Scalar, ASoa, BSoa>();
}

TEST(ResourceManagerTest, ApplyOnAllTypesSoa) {
  RunApplyOnAllTypesTest<Soa, AScalar, BScalar>();
  RunApplyOnAllTypesTest<Soa, ASoa, BSoa>();
}

TEST(ResourceManagerTest, IOAos) { RunIOAosTest(); }

TEST(ResourceManagerTest, IOSoa) { RunIOSoaTest(); }

TEST(ResourceManagerTest, RmFunction) {
  EXPECT_EQ(ResourceManager<>::Get(), Rm());
}

// TODO
TEST(ResourceManagerTest, GetSimulationObject) {
  using CTParam = CompileTimeParam<Soa, AScalar, BScalar>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<AScalar>();
  a_collection->push_back(AScalar(12));

  // auto b_collection = rm->template Get<BScalar>();
  // b_collection->push_back(BScalar(3.14));
  // b_collection->push_back(BScalar(6.28));

  SoHandle soh1(0, 0);
  // auto&& so1 = rm->GetElement(soh1);
  // std::cout << so1.GetData() << std::endl;
  // EXPECT_EQ(12, so1.GetData());

  // SoHandle soh2(1, 1);
  // auto&& so2 = rm->GetElement(soh2);
  // EXPECT_EQ(6.28, so2.GetData());
}
//
// TEST(SoHandle, SOH_CALLMacro) {
//   Rm()->Clear();
//   Rm()->Get<Cell>()->push_back(Cell({1, 2, 3}));
//
//   SoHandle so_handle(0, 0);
//   // auto& pos =  SOH_CALL(so_handle, GetPosition());
// //   auto& pos = Rm()->ApplyOnElement(so_handle, [&](auto&& so, SoHandle) {
// //     return so.GetPosition();
// //   });
// //   EXPECT_ARR_NEAR(pos, {1, 2, 3})
//   auto&& so = Rm()->GetElement(so_handle);
//   const auto& pos = so.GetPosition();
//   EXPECT_ARR_NEAR(pos, {1, 2, 3});
//   so.SetDiameter(21);
//   EXPECT_NEAR(21, so.GetDiameter(), 1e-5);
// }

template <typename Backend, typename A, typename B>
void RunPushBackTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, A, B>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  rm->push_back(AScalar(12));
  rm->push_back(AScalar(34));

  rm->push_back(BScalar(3.14));
  rm->push_back(BScalar(6.28));

  rm->push_back(AScalar(87));

  auto as = rm->template Get<A>();
  auto bs = rm->template Get<B>();

  EXPECT_EQ((*as)[0].GetData(), 12);
  EXPECT_EQ((*as)[1].GetData(), 34);
  EXPECT_EQ((*as)[2].GetData(), 87);

  EXPECT_NEAR((*bs)[0].GetData(), 3.14, kEpsilon);
  EXPECT_NEAR((*bs)[1].GetData(), 6.28, kEpsilon);
}

TEST(ResourceManagerTest, push_backSoa) {
  RunPushBackTest<Soa, AScalar, BScalar>();
  RunPushBackTest<Soa, ASoa, BSoa>();
}

TEST(ResourceManagerTest, push_backAos) {
  RunPushBackTest<Scalar, AScalar, BScalar>();
  RunPushBackTest<Scalar, ASoa, BSoa>();
}

TEST(SoHandle, EqualsOperator) {
  EXPECT_EQ(SoHandle(0, 0), SoHandle(0, 0));
  EXPECT_EQ(SoHandle(1, 0), SoHandle(1, 0));
  EXPECT_EQ(SoHandle(0, 1), SoHandle(0, 1));
  EXPECT_EQ(SoHandle(1, 1), SoHandle(1, 1));

  EXPECT_FALSE(SoHandle(0, 0) == SoHandle(0, 1));
  EXPECT_FALSE(SoHandle(0, 0) == SoHandle(1, 0));
  EXPECT_FALSE(SoHandle(0, 0) == SoHandle(1, 1));
  EXPECT_FALSE(SoHandle(1, 0) == SoHandle(0, 0));
  EXPECT_FALSE(SoHandle(1, 0) == SoHandle(0, 1));
  EXPECT_FALSE(SoHandle(1, 0) == SoHandle(1, 1));
  EXPECT_FALSE(SoHandle(0, 1) == SoHandle(0, 0));
  EXPECT_FALSE(SoHandle(0, 1) == SoHandle(1, 0));
  EXPECT_FALSE(SoHandle(0, 1) == SoHandle(1, 1));
  EXPECT_FALSE(SoHandle(1, 1) == SoHandle(0, 0));
  EXPECT_FALSE(SoHandle(1, 1) == SoHandle(1, 0));
  EXPECT_FALSE(SoHandle(1, 1) == SoHandle(0, 1));
}

}  // namespace resource_manager_test_internal
}  // namespace bdm
