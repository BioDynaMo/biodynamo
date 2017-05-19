#include "physics/default_force.h"
#include "gtest/gtest.h"
#include "physics/physical_sphere.h"

namespace bdm {

using physics::DefaultForce;
using physics::PhysicalSphere;

TEST(TargetValue_DefaultForce, forceOnASphereFromASphere) {
  PhysicalSphere cell1;
  cell1.setAdherence(0.4);
  cell1.setDiameter(10);
  cell1.setMass(1.1);
  std::array<double, 3> pos1 = {0, 0, 0};
  cell1.setMassLocation(pos1);

  PhysicalSphere cell2;
  cell2.setAdherence(0.4);
  cell2.setDiameter(10);
  cell2.setMass(1.1);
  std::array<double, 3> pos2 = {0, 5, 0};
  cell2.setMassLocation(pos2);

  DefaultForce force;
  auto force1 = force.forceOnASphereFromASphere(&cell1, &cell2);
  auto force2 = force.forceOnASphereFromASphere(&cell2, &cell1);

  EXPECT_NEAR(0, force1[0], 1e-5);
  EXPECT_NEAR(-10.900980486407214, force1[1], 1e-5);
  EXPECT_NEAR(0, force1[2], 1e-5);

  EXPECT_NEAR(0, force2[0], 1e-5);
  EXPECT_NEAR(10.900980486407214, force2[1], 1e-5);
  EXPECT_NEAR(0, force2[2], 1e-5);
}


}  // namespace bdm
