#include "transactional_vector.h"
#include <thread>
#include "gtest/gtest.h"

namespace bdm {
namespace transactional_vector_test_internal {

TEST(TransactionalVectorTest, All) {
  TransactionalVector<int> vector;

  vector.DelayedPushBack(1);
  vector.DelayedPushBack(2);
  vector.DelayedPushBack(3);

  // changes have not been commited yet
  EXPECT_EQ(0u, vector.size());

  vector.Commit();

  EXPECT_EQ(3u, vector.size());
  EXPECT_EQ(1, vector[0]);
  EXPECT_EQ(2, vector[1]);
  EXPECT_EQ(3, vector[2]);

  vector.DelayedRemove(0);

  // changes have not been commited yet
  EXPECT_EQ(3u, vector.size());

  vector.Commit();

  EXPECT_EQ(2u, vector.size());
  EXPECT_EQ(3, vector[0]);
  EXPECT_EQ(2, vector[1]);

  vector.DelayedRemove(0);
  vector.DelayedRemove(1);
  vector.Commit();
  EXPECT_EQ(0u, vector.size());
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
