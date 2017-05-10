#include "simulation_object.h"
#include "gtest/gtest.h"

namespace bdm {

TEST(SimulationObjectTest, push_backAndClear) {
  SimulationObject<Soa> so;
  // call clear, because creating a SOA object with default constructor will
  // already have one element inside
  so.clear();
  EXPECT_EQ(0u, so.size());
  so.push_back(SimulationObject<>());
  so.push_back(SimulationObject<>());
  EXPECT_EQ(2u, so.size());
  so.clear();
  EXPECT_EQ(0u, so.size());
}

}  // namespace bdm
