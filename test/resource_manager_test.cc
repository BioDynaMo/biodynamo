#include "resource_manager.h"

#include <vector>
#include "backend.h"
#include "gtest/gtest.h"
#include "test_util.h"

namespace bdm {
namespace resource_manager_test_internal {

using std::vector;

struct AScalar {
  using Backend = Scalar;
  explicit AScalar(int data) : data_(data) {}

  int GetData() { return data_; }

  int data_;
};

struct BScalar {
  using Backend = Scalar;
  explicit BScalar(double data) : data_(data) {}

  double GetData() { return data_; }

  double data_;
};

struct ASoa {
  using Backend = Soa;
  vector<int> data_;
  size_t idx_ = 0;

  int GetData() { return data_[idx_]; }

  size_t size() const { return data_.size(); }  // NOLINT

  void clear() { data_.clear(); }  // NOLINT

  void push_back(const AScalar& a) { data_.push_back(a.data_); }  // NOLINT

  ASoa& operator[](size_t idx) {
    idx_ = idx;
    return *this;
  }
};

struct BSoa {
  using Backend = Soa;
  vector<double> data_;
  size_t idx_ = 0;

  double GetData() { return data_[idx_]; }

  size_t size() const { return data_.size(); }  // NOLINT

  void clear() { data_.clear(); }  // NOLINT

  void push_back(const BScalar& b) { data_.push_back(b.data_); }  // NOLINT

  BSoa& operator[](size_t idx) {
    idx_ = idx;
    return *this;
  }
};

/// Create ResourceManager with two types, use Get function to obtain container
/// of the specified type, push_back values and check if they have correctly
/// been added inside the ResourceManager
/// @tparam A type one: scalar or soa backend
/// @tparam B type two: scalar or soa backend
template <typename A, typename B>
void RunGetTest() {
  const double kEpsilon = abs_error<double>::value;
  auto rm = ResourceManager<A, B>::Get();
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

TEST(ResourceManagerTest, GetAos) { RunGetTest<AScalar, BScalar>(); }

TEST(ResourceManagerTest, GetSoa) { RunGetTest<ASoa, BSoa>(); }

template <typename A, typename B>
void RunApplyOnElementTest() {
  const double kEpsilon = abs_error<double>::value;
  auto rm = ResourceManager<A, B>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<A>();
  a_collection->push_back(AScalar(12));
  a_collection->push_back(AScalar(34));
  rm->ApplyOnElement(rm->GenSoHandle(1, 0),
                     [](auto& element) { EXPECT_EQ(34, element.GetData()); });

  auto b_collection = rm->template Get<B>();
  b_collection->push_back(BScalar(3.14));
  b_collection->push_back(BScalar(6.28));
  rm->ApplyOnElement(rm->GenSoHandle(0, 1), [&](auto& element) {
    EXPECT_NEAR(3.14, element.GetData(), kEpsilon);
  });
}

TEST(ResourceManagerTest, ApplyOnElementAos) {
  RunApplyOnElementTest<AScalar, BScalar>();
}

TEST(ResourceManagerTest, ApplyOnElementSoa) {
  RunApplyOnElementTest<ASoa, BSoa>();
}

template <typename A, typename B>
void RunApplyOnAllElementsTest() {
  const double kEpsilon = abs_error<double>::value;
  auto rm = ResourceManager<A, B>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<A>();
  a_collection->push_back(AScalar(12));
  a_collection->push_back(AScalar(34));

  auto b_collection = rm->template Get<B>();
  b_collection->push_back(BScalar(3.14));
  b_collection->push_back(BScalar(6.28));
  size_t counter = 0;
  rm->ApplyOnAllElements([&](auto& element) {
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
  RunApplyOnAllElementsTest<AScalar, BScalar>();
}

TEST(ResourceManagerTest, ApplyOnAllElementsSoa) {
  RunApplyOnAllElementsTest<ASoa, BSoa>();
}

template <typename A, typename B>
void RunApplyOnAllTypesTest() {
  const double kEpsilon = abs_error<double>::value;
  auto rm = ResourceManager<A, B>::Get();
  rm->Clear();

  auto a_collection = rm->template Get<A>();
  a_collection->push_back(AScalar(12));

  auto b_collection = rm->template Get<B>();
  b_collection->push_back(BScalar(3.14));
  b_collection->push_back(BScalar(6.28));
  size_t counter = 0;
  rm->ApplyOnAllTypes([&](auto& container) {
    counter++;
    switch (counter) {
      case 1:
        EXPECT_EQ(1u, container.size());
        EXPECT_EQ(12, container[0].GetData());
        break;
      case 2:
        EXPECT_EQ(2u, container.size());
        EXPECT_NEAR(3.14, container[0].GetData(), kEpsilon);
        EXPECT_NEAR(6.28, container[1].GetData(), kEpsilon);
        break;
    }
  });

  EXPECT_EQ(2u, counter);
}

TEST(ResourceManagerTest, ApplyOnAllTypesAos) {
  RunApplyOnAllTypesTest<AScalar, BScalar>();
}

TEST(ResourceManagerTest, ApplyOnAllTypesSoa) {
  RunApplyOnAllTypesTest<ASoa, BSoa>();
}

}  // namespace resource_manager_test_internal
}  // namespace bdm
