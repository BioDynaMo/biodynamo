#include <gtest/gtest.h>
#include "backend.h"
#include "default_force.h"
#include "test_util.h"

namespace bdm {

/// Tests the forces that are created between the reference sphere and its
/// overlapping neighbors
/// implementation uses virual bigger radii to have distant interaction
TEST(DefaultForce, General) {
  using real_v = VcBackend::real_v;
  using real_t = real_v::value_type;
  if (real_v::Size < 2) {
    FAIL() << "Backend must at least support two elements for this test";
  }
  std::array<real_v, 3> ref_mass_location = {1.1, 1.0, 0.9};
  real_v ref_diameter = 8;
  real_v ref_iof_coefficient = 0.15;
  real_v nb_x((const real_t[]){0, 5});
  real_v nb_y((const real_t[]){0, 5});
  real_v nb_z((const real_t[]){0, 0});
  std::array<real_v, 3> nb_mass_location = {nb_x, nb_y, nb_z};
  real_v nb_diameter((const real_t[]){5, 10});
  real_v nb_iof_coefficient((const real_t[]){0.15, 0.15});
  std::array<real_v, 3> result;

  DefaultForce<VcBackend> force;
  force.forceBetweenSpheres(ref_mass_location, ref_diameter,
                            ref_iof_coefficient, nb_mass_location, nb_diameter,
                            nb_iof_coefficient, &result);

  // x component
  EXPECT_NEAR(result[0][0], 7.1429184067241138, abs_error<real_t>::value);
  EXPECT_NEAR(result[0][1], -5.7454658831720176, abs_error<real_t>::value);
  // y component
  EXPECT_NEAR(result[1][0], 6.4935621879310119, abs_error<real_t>::value);
  EXPECT_NEAR(result[1][1], -5.892785521202069, abs_error<real_t>::value);
  // z component
  EXPECT_NEAR(result[2][0], 5.8442059691379109, abs_error<real_t>::value);
  EXPECT_NEAR(result[2][1], 1.3258767422704656, abs_error<real_t>::value);
}

/// Tests the special case that non of the neighbors overlap
/// with the reference cell
TEST(DefaultForce, AllNonOverlapping) {
  using real_v = VcBackend::real_v;
  using real_t = real_v::value_type;
  std::array<real_v, 3> ref_mass_location = {0, 0, 0};
  real_v ref_diameter = 8;
  real_v ref_iof_coefficient = 0.15;
  real_v nb_x(11.01);
  real_v nb_y(0);
  real_v nb_z(0);
  std::array<real_v, 3> nb_mass_location = {nb_x, nb_y, nb_z};
  real_v nb_diameter(8);
  real_v nb_iof_coefficient(0.15);
  std::array<real_v, 3> result;

  DefaultForce<VcBackend> force;
  force.forceBetweenSpheres(ref_mass_location, ref_diameter,
                            ref_iof_coefficient, nb_mass_location, nb_diameter,
                            nb_iof_coefficient, &result);

  // x component
  EXPECT_NEAR(result[0].sum(), 0, abs_error<real_t>::value);
  // y component
  EXPECT_NEAR(result[1].sum(), 0, abs_error<real_t>::value);
  // z component
  EXPECT_NEAR(result[2].sum(), 0, abs_error<real_t>::value);
}

/// Tests the case that one neighbor does not overlap and the other
/// does
TEST(DefaultForce, OneNonOverlapping) {
  using real_v = VcBackend::real_v;
  using real_t = real_v::value_type;
  if (real_v::Size < 2) {
    FAIL() << "Backend must at least support two elements for this test";
  }
  std::array<real_v, 3> ref_mass_location = {0, 0, 0};
  real_v ref_diameter = 8;
  real_v ref_iof_coefficient = 0.15;
  real_v nb_x((const real_t[]){11.01, 8});
  real_v nb_y((const real_t[]){0, 0});
  real_v nb_z((const real_t[]){0, 0});
  std::array<real_v, 3> nb_mass_location = {nb_x, nb_y, nb_z};
  real_v nb_diameter((const real_t[]){8, 8});
  real_v nb_iof_coefficient((const real_t[]){0.15, 0.15});
  std::array<real_v, 3> result;

  DefaultForce<VcBackend> force;
  force.forceBetweenSpheres(ref_mass_location, ref_diameter,
                            ref_iof_coefficient, nb_mass_location, nb_diameter,
                            nb_iof_coefficient, &result);

  // x component
  EXPECT_NEAR(result[0][0], 0, abs_error<real_t>::value);
  // y component
  EXPECT_NEAR(result[1][0], 0, abs_error<real_t>::value);
  // z component
  EXPECT_NEAR(result[2][0], 0, abs_error<real_t>::value);
}

/// Tests the special case that neighbor and reference cell
/// are at the same position -> should return random force
TEST(DefaultForce, AllAtSamePosition) {
  using real_v = VcBackend::real_v;
  std::array<real_v, 3> ref_mass_location = {0, 0, 0};
  real_v ref_diameter = 8;
  real_v ref_iof_coefficient = 0.15;
  real_v nb_x(0);
  real_v nb_y(0);
  real_v nb_z(0);
  std::array<real_v, 3> nb_mass_location = {nb_x, nb_y, nb_z};
  real_v nb_diameter(8);
  real_v nb_iof_coefficient(0.15);
  std::array<real_v, 3> result;

  DefaultForce<VcBackend> force;
  force.forceBetweenSpheres(ref_mass_location, ref_diameter,
                            ref_iof_coefficient, nb_mass_location, nb_diameter,
                            nb_iof_coefficient, &result);

  // random number must be in interval [-3.0, 3.0]
  // x component
  EXPECT_NEAR(result[0][0], 0, 3);
  EXPECT_NEAR(result[0][1], 0, 3);
  // y component
  EXPECT_NEAR(result[1][0], 0, 3);
  EXPECT_NEAR(result[1][1], 0, 3);
  // z component
  EXPECT_NEAR(result[2][0], 0, 3);
  EXPECT_NEAR(result[2][1], 0, 3);
}

}  // namespace bdm
