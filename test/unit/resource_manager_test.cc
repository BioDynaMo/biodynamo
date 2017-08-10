// I/O related code must be in header file
#include "unit/resource_manager_test.h"

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
void RunApplyOnAllElementsParallelTest() {
  using CTParam = CompileTimeParam<Backend, A, B>;
  auto rm = ResourceManager<CTParam>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<A>();
  a_collection->push_back(AScalar(12));
  a_collection->push_back(AScalar(34));

  auto b_collection = rm->template Get<B>();
  b_collection->push_back(BScalar(3.14));
  b_collection->push_back(BScalar(6.28));
  rm->ApplyOnAllElementsParallel([](auto& element, SoHandle handle) {  // NOLINT
    const double kEpsilon = abs_error<double>::value;
    if (handle == SoHandle(0, 0)) {
      EXPECT_EQ(12, element.GetData());
    } else if (handle == SoHandle(0, 1)) {
      EXPECT_EQ(34, element.GetData());
    } else if (handle == SoHandle(1, 0)) {
      EXPECT_NEAR(3.14, element.GetData(), kEpsilon);
    } else if (handle == SoHandle(1, 1)) {
      EXPECT_NEAR(6.28, element.GetData(), kEpsilon);
    }
  });
}

TEST(ResourceManagerTest, ApplyOnAllElementsParallelAos) {
  RunApplyOnAllElementsParallelTest<Scalar, AScalar, BScalar>();
  RunApplyOnAllElementsParallelTest<Scalar, ASoa, BSoa>();
}

TEST(ResourceManagerTest, ApplyOnAllElementsParallelSoa) {
  RunApplyOnAllElementsParallelTest<Soa, ASoa, BSoa>();
  RunApplyOnAllElementsParallelTest<Soa, AScalar, BScalar>();
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

TEST(ResourceManagerTest, IOSoa) { RunSoaTest(); }

}  // namespace resource_manager_test_internal
}  // namespace bdm
