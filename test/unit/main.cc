#include <iostream>
#include "backend.h"
#include "gtest/gtest.h"
#include "param.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
