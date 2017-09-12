#include "fixed_size_vector.h"
#include "gtest/gtest.h"

namespace bdm {

TEST(FixedSizeVector, All) {
  FixedSizeVector<int, 4> vector;

  ASSERT_EQ(0u, vector.size());

  vector.push_back(0);
  ASSERT_EQ(1u, vector.size());

  vector.push_back(1);
  vector.push_back(2);
  vector.push_back(3);
  ASSERT_EQ(4u, vector.size());

  for (int i = 0; i < 4; i++) {
    ASSERT_EQ(i, vector[i]);
  }

  for (int i = 0; i < 4; i++) {
    vector[i]++;
  }

  size_t counter = 0;
  for (auto element : vector) {
    ASSERT_EQ(counter + 1, vector[counter]);
    counter++;
  }
}

}  // namespace bdm
