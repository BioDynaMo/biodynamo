#include "transactional_vector.h"
#include <thread>
#include "gtest/gtest.h"

namespace bdm {
namespace transactional_vector_test_internal {

TEST(TransactionalVectorTest, All) {
  TransactionalVector<int> vector;

  EXPECT_EQ(0u, vector.DelayedPushBack(1));
  EXPECT_EQ(1u, vector.DelayedPushBack(2));
  EXPECT_EQ(2u, vector.DelayedPushBack(3));

  // changes have not been commited yet
  EXPECT_EQ(0u, vector.size());
  EXPECT_TRUE(vector.begin() == vector.end());
  EXPECT_TRUE(vector.cbegin() == vector.cend());

  vector.Commit();

  EXPECT_EQ(3u, vector.size());
  EXPECT_EQ(1, vector[0]);
  EXPECT_EQ(2, vector[1]);
  EXPECT_EQ(3, vector[2]);

  // test iterator
  int64_t counter = 0;
  for(auto i : vector) {
    EXPECT_EQ(counter++  + 1, i);
  }
  EXPECT_EQ(counter, 3);

  vector.DelayedRemove(0);

  // changes have not been commited yet
  EXPECT_EQ(3u, vector.size());
  EXPECT_TRUE((vector.begin() += 3) == vector.end());
  EXPECT_TRUE((vector.cbegin() += 3) == vector.cend());

  vector.Commit();

  EXPECT_EQ(2u, vector.size());
  EXPECT_EQ(3, vector[0]);
  EXPECT_EQ(2, vector[1]);

  // test iterator
  counter = 0;
  for(auto i : vector) {
    if(!counter++) {
      EXPECT_EQ(3, i);
    } else {
      EXPECT_EQ(2, i);
    }
  }
  EXPECT_EQ(counter, 2);

  vector.DelayedRemove(0);
  vector.DelayedRemove(1);
  vector.Commit();
  EXPECT_EQ(0u, vector.size());
  EXPECT_TRUE(vector.begin() == vector.end());
  EXPECT_TRUE(vector.cbegin() == vector.cend());

  // push_back
  vector.push_back(9);
  EXPECT_EQ(1u, vector.size());
  EXPECT_EQ(9, vector[0]);

  vector.DelayedPushBack(10);
  EXPECT_EQ(1u, vector.size());
  // clang on Travis OSX image doesn't catch exception
  // Therefore the following check is commented until this is fixed
  // try {
  //   vector.push_back(11);
  //   FAIL() << "Should have thrown a logic_error";
  // } catch(std::logic_error& e) {}
}

TEST(TransactionalVectorTest, DelayedRemove) {
  TransactionalVector<int> vector;
  for (uint64_t i = 0; i < 10; i++) {
    vector.push_back(i);
  }

  EXPECT_EQ(10u, vector.size());

  vector.DelayedRemove(5);
  vector.DelayedRemove(8);
  vector.DelayedRemove(3);

  EXPECT_EQ(10u, vector.size());

  auto updated_indices = vector.Commit();

  EXPECT_EQ(7u, vector.size());
  ASSERT_EQ(2u, updated_indices.size());
  EXPECT_EQ(5, updated_indices[9]);
  EXPECT_EQ(3, updated_indices[7]);

  EXPECT_EQ(0, vector[0]);
  EXPECT_EQ(1, vector[1]);
  EXPECT_EQ(2, vector[2]);
  EXPECT_EQ(7, vector[3]);
  EXPECT_EQ(4, vector[4]);
  EXPECT_EQ(9, vector[5]);
  EXPECT_EQ(6, vector[6]);
}

void PushBackElements(TransactionalVector<int> *vector, size_t start_value,
                      size_t num_elements) {
  for (size_t i = start_value; i < start_value + num_elements; i++) {
    vector->DelayedPushBack(i);
  }
}

void RemoveElements(TransactionalVector<int> *vector, size_t start_value,
                    size_t num_elements) {
  for (size_t i = start_value; i < start_value + num_elements; i++) {
    vector->DelayedRemove(i);
  }
}

TEST(TransactionalVectorTest, ThreadSafety) {
  TransactionalVector<int> vector;

  std::thread t1(PushBackElements, &vector, 0, 100);
  std::thread t2(PushBackElements, &vector, 100, 101);
  t1.join();
  t2.join();

  vector.Commit();

  // since insertion was done using two threads order of elements is unknown
  // sort so we know which elements are removed in the next step
  std::sort(vector.begin(), vector.end());

  std::thread t3(RemoveElements, &vector, 0, 20);
  std::thread t4(RemoveElements, &vector, 20, 21);
  t3.join();
  t4.join();

  vector.Commit();

  int sum = 0;
  for (size_t i = 0; i < vector.size(); i++) {
    sum += vector[i];
  }

  EXPECT_EQ(19280, sum);
}

}  // namespace transactional_vector_test_internal
}  // namespace bdm
