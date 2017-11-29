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
template <typename Backend, typename TA, typename TB>
inline void RunGetTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, TA, TB>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  // template specifier needed because A is dependant type
  auto a_vector = rm->template Get<TA>();
  EXPECT_EQ(0u, a_vector->size());
  a_vector->push_back(A(12));
  a_vector->push_back(A(34));
  EXPECT_EQ(12, (*rm->template Get<TA>())[0].GetData());
  EXPECT_EQ(34, (*rm->template Get<TA>())[1].GetData());
  EXPECT_EQ(2u, rm->template Get<TA>()->size());

  auto b_vector = rm->template Get<TB>();
  EXPECT_EQ(0u, b_vector->size());
  b_vector->push_back(B(3.14));
  b_vector->push_back(B(6.28));
  EXPECT_NEAR(3.14, (*rm->template Get<TB>())[0].GetData(), kEpsilon);
  EXPECT_NEAR(6.28, (*rm->template Get<TB>())[1].GetData(), kEpsilon);
  EXPECT_EQ(2u, rm->template Get<TB>()->size());
}

TEST(ResourceManagerTest, GetAos) {
  RunGetTest<Scalar, A, B>();
  RunGetTest<Scalar, SoaA, SoaB>();
}

TEST(ResourceManagerTest, GetSoa) {
  RunGetTest<Soa, A, B>();
  RunGetTest<Soa, SoaA, SoaB>();
}

template <typename Backend, typename TA, typename TB>
inline void RunApplyOnElementTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, TA, TB>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<TA>();
  a_collection->push_back(A(12));
  a_collection->push_back(A(34));
  rm->ApplyOnElement(SoHandle(0, 1),
                     [](auto&& element) { EXPECT_EQ(34, element.GetData()); });

  auto b_collection = rm->template Get<TB>();
  b_collection->push_back(B(3.14));
  b_collection->push_back(B(6.28));
  rm->ApplyOnElement(SoHandle(1, 0), [&](auto&& element) {
    EXPECT_NEAR(3.14, element.GetData(), kEpsilon);
  });
}

TEST(ResourceManagerTest, ApplyOnElementAos) {
  RunApplyOnElementTest<Scalar, A, B>();
  RunApplyOnElementTest<Scalar, SoaA, SoaB>();
}

TEST(ResourceManagerTest, ApplyOnElementSoa) {
  RunApplyOnElementTest<Soa, A, B>();
  RunApplyOnElementTest<Soa, SoaA, SoaB>();
}

template <typename Backend, typename TA, typename TB>
void RunApplyOnAllElementsTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, TA, TB>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<TA>();
  a_collection->push_back(A(12));
  a_collection->push_back(A(34));

  auto b_collection = rm->template Get<TB>();
  b_collection->push_back(B(3.14));
  b_collection->push_back(B(6.28));
  size_t counter = 0;
  rm->ApplyOnAllElements([&](auto&& element, SoHandle&& handle) {  // NOLINT
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
  RunApplyOnAllElementsTest<Scalar, A, B>();
  RunApplyOnAllElementsTest<Scalar, SoaA, SoaB>();
}

TEST(ResourceManagerTest, ApplyOnAllElementsSoa) {
  RunApplyOnAllElementsTest<Soa, SoaA, SoaB>();
  RunApplyOnAllElementsTest<Soa, A, B>();
}

template <typename Backend, typename TA, typename TB>
void RunGetNumSimObjects() {
  using CTParam = CompileTimeParam<Backend, TA, TB>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<TA>();
  a_collection->push_back(A(12));
  a_collection->push_back(A(34));
  a_collection->push_back(A(59));

  auto b_collection = rm->template Get<TB>();
  b_collection->push_back(B(3.14));
  b_collection->push_back(B(6.28));

  EXPECT_EQ(5u, rm->GetNumSimObjects());
}

TEST(ResourceManagerTest, GetNumSimObjectsAos) {
  RunGetNumSimObjects<Scalar, A, B>();
  RunGetNumSimObjects<Scalar, SoaA, SoaB>();
}

TEST(ResourceManagerTest, GetNumSimObjectsSoa) {
  RunGetNumSimObjects<Soa, SoaA, SoaB>();
  RunGetNumSimObjects<Soa, A, B>();
}

// This test uses Cells since SoaA, SoaB are strippted down simulatio objects
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

template <typename Backend, typename TA, typename TB>
void RunApplyOnAllTypesTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, TA, TB>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<TA>();
  a_collection->push_back(A(12));

  auto b_collection = rm->template Get<TB>();
  b_collection->push_back(B(3.14));
  b_collection->push_back(B(6.28));
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
  RunApplyOnAllTypesTest<Scalar, A, B>();
  RunApplyOnAllTypesTest<Scalar, SoaA, SoaB>();
}

TEST(ResourceManagerTest, ApplyOnAllTypesSoa) {
  RunApplyOnAllTypesTest<Soa, A, B>();
  RunApplyOnAllTypesTest<Soa, SoaA, SoaB>();
}

TEST(ResourceManagerTest, IOAos) { RunIOAosTest(); }

TEST(ResourceManagerTest, IOSoa) { RunIOSoaTest(); }

TEST(ResourceManagerTest, RmFunction) {
  EXPECT_EQ(ResourceManager<>::Get(), Rm());
}

template <typename Backend, typename TA, typename TB>
void RunGetTypeIndexTest() {
  using CTParam = CompileTimeParam<Backend, TA, TB>;
  using TRm = ResourceManager<CTParam>;
  EXPECT_EQ(0u, TRm::template GetTypeIndex<TA>());
  EXPECT_EQ(1u, TRm::template GetTypeIndex<TB>());
}

TEST(ResourceManagerTest, GetTypeIndexSoa) {
  RunGetTypeIndexTest<Soa, A, B>();
  RunGetTypeIndexTest<Soa, SoaA, SoaB>();
}

TEST(ResourceManagerTest, GetTypeIndexAos) {
  RunGetTypeIndexTest<Scalar, A, B>();
  RunGetTypeIndexTest<Scalar, SoaA, SoaB>();
}

template <typename Backend, typename TA, typename TB>
void RunPushBackTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, TA, TB>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  rm->push_back(A(12));
  rm->push_back(A(34));

  rm->push_back(B(3.14));
  rm->push_back(B(6.28));

  rm->push_back(A(87));

  auto as = rm->template Get<TA>();
  auto bs = rm->template Get<TB>();

  EXPECT_EQ((*as)[0].GetData(), 12);
  EXPECT_EQ((*as)[1].GetData(), 34);
  EXPECT_EQ((*as)[2].GetData(), 87);

  EXPECT_NEAR((*bs)[0].GetData(), 3.14, kEpsilon);
  EXPECT_NEAR((*bs)[1].GetData(), 6.28, kEpsilon);
}

TEST(ResourceManagerTest, push_backSoa) {
  RunPushBackTest<Soa, A, B>();
  RunPushBackTest<Soa, SoaA, SoaB>();
}

TEST(ResourceManagerTest, push_backAos) {
  RunPushBackTest<Scalar, A, B>();
  RunPushBackTest<Scalar, SoaA, SoaB>();
}

template <typename Backend, typename TA, typename TB>
void RunNewTest() {
  const double kEpsilon = abs_error<double>::value;
  using CTParam = CompileTimeParam<Backend, TA, TB>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto a0 = rm->template New<A>(12);
  auto a1 = rm->template New<A>(34);

  auto b0 = rm->template New<B>(3.14);
  auto b1 = rm->template New<B>(6.28);

  auto a2 = rm->template New<A>(87);

  EXPECT_EQ(a0.GetData(), 12);
  EXPECT_EQ(a1.GetData(), 34);
  EXPECT_EQ(a2.GetData(), 87);

  EXPECT_NEAR(b0.GetData(), 3.14, kEpsilon);
  EXPECT_NEAR(b1.GetData(), 6.28, kEpsilon);

  auto as = rm->template Get<TA>();
  auto bs = rm->template Get<TB>();

  EXPECT_EQ((*as)[0].GetData(), 12);
  EXPECT_EQ((*as)[1].GetData(), 34);
  EXPECT_EQ((*as)[2].GetData(), 87);

  EXPECT_NEAR((*bs)[0].GetData(), 3.14, kEpsilon);
  EXPECT_NEAR((*bs)[1].GetData(), 6.28, kEpsilon);

  // modify return value of new
  a1.SetData(321);
  EXPECT_EQ((*as)[1].GetData(), 321);
}

TEST(ResourceManagerTest, NewSoa) {
  // RunNewTest<Soa, A, B>();
  RunNewTest<Soa, SoaA, SoaB>();
}

TEST(ResourceManagerTest, NewAos) {
  // RunNewTest<Scalar, A, B>();
  // RunNewTest<Scalar, SoaA, SoaB>();
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
