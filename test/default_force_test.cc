#include "default_force.h"
#include "backend.h"
#include "gtest/gtest.h"
#include "test_util.h"

namespace bdm {

/// Tests the forces that are created between the reference sphere and its
/// overlapping neighbors
/// implementation uses virual bigger radii to have distant interaction
TEST(DefaultForce, General) {
  std::array<double, 3> ref_mass_location = {1.1, 1.0, 0.9};
  double ref_diameter = 8;
  double ref_iof_coefficient = 0.15;
  double nb_diameter = 5;
  double nb_iof_coefficient = 0.15;
  std::array<double, 3> nb_mass_location = {0, 0, 0};
  std::array<double, 3> result;

  DefaultForce force;
  force.ForceBetweenSpheres(ref_mass_location, ref_diameter,
                            ref_iof_coefficient, nb_mass_location, nb_diameter,
                            nb_iof_coefficient, &result);

  EXPECT_NEAR(7.1429184067241138, result[0], abs_error<double>::value);
  EXPECT_NEAR(6.4935621879310119, result[1], abs_error<double>::value);
  EXPECT_NEAR(5.8442059691379109, result[2], abs_error<double>::value);

  nb_diameter = 10;
  nb_mass_location = {5, 5, 0};
  force.ForceBetweenSpheres(ref_mass_location, ref_diameter,
                            ref_iof_coefficient, nb_mass_location, nb_diameter,
                            nb_iof_coefficient, &result);

  EXPECT_NEAR(-5.7454658831720176, result[0], abs_error<double>::value);
  EXPECT_NEAR(-5.892785521202069, result[1], abs_error<double>::value);
  EXPECT_NEAR(1.3258767422704656, result[2], abs_error<double>::value);
}

/// Tests the special case that non of the neighbors overlap
/// with the reference cell
TEST(DefaultForce, AllNonOverlapping) {
  std::array<double, 3> ref_mass_location = {0, 0, 0};
  double ref_diameter = 8;
  double ref_iof_coefficient = 0.15;
  std::array<double, 3> nb_mass_location = {11.01, 0, 0};
  double nb_diameter = 8;
  double nb_iof_coefficient = 0.15;
  std::array<double, 3> result;

  DefaultForce force;
  force.ForceBetweenSpheres(ref_mass_location, ref_diameter,
                            ref_iof_coefficient, nb_mass_location, nb_diameter,
                            nb_iof_coefficient, &result);

  EXPECT_NEAR(0, result[0], abs_error<double>::value);
  EXPECT_NEAR(0, result[1], abs_error<double>::value);
  EXPECT_NEAR(0, result[2], abs_error<double>::value);
}

/// Tests the special case that neighbor and reference cell
/// are at the same position -> should return random force
TEST(DefaultForce, AllAtSamePosition) {
  std::array<double, 3> ref_mass_location = {0, 0, 0};
  double ref_diameter = 8;
  double ref_iof_coefficient = 0.15;
  std::array<double, 3> nb_mass_location = {0, 0, 0};
  double nb_diameter = 8;
  double nb_iof_coefficient = 0.15;
  std::array<double, 3> result;

  DefaultForce force;
  force.ForceBetweenSpheres(ref_mass_location, ref_diameter,
                            ref_iof_coefficient, nb_mass_location, nb_diameter,
                            nb_iof_coefficient, &result);

  // random number must be in interval [-3.0, 3.0]
  EXPECT_NEAR(0, result[0], 3);
  EXPECT_NEAR(0, result[1], 3);
  EXPECT_NEAR(0, result[2], 3);
}

}  // namespace bdm
