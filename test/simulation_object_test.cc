#include "simulation_object.h"
#include "gtest/gtest.h"

namespace bdm {

TEST(SimulationObjectTest, push_backAndClear) {
  SimulationObject<Soa> soa;
  // call clear, because creating a SOA object with default constructor will
  // already have one element inside
  soa.clear();
  EXPECT_EQ(0u, soa.size());
  SimulationObject<Scalar> so;
  soa.push_back(so);
  soa.push_back(so);
  EXPECT_EQ(2u, soa.size());
  soa.clear();
  EXPECT_EQ(0u, soa.size());
}

}  // namespace bdm
