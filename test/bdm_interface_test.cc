#include "bdm_interface.h"
#include <gtest/gtest.h>
#include "biology_module_util.h"
#include "test_util.h"

namespace bdm {

TEST(BDMInterfaceTest, Vector3DInterface) {
  Vector3DInterface v3Dinterf;

  EXPECT_EQ(0, v3Dinterf.coord_[0]);
  v3Dinterf.coord_[1] = 1.2;
  EXPECT_EQ(1.2, v3Dinterf.coord_[1]);

  ContinuousInterfaceData cont_interf_data;

  cont_interf_data.oxygen_level_gradient_.push_back(v3Dinterf);
  EXPECT_EQ(0, cont_interf_data.oxygen_level_gradient_[0].coord_[1]);
  EXPECT_EQ(1.2, cont_interf_data.oxygen_level_gradient_[8].coord_[1]);

  DiscontinuousInterfaceData disc_interf_data;
  disc_interf_data.ecm_density_gradient_ = v3Dinterf;
  EXPECT_EQ(1.2, disc_interf_data.ecm_density_gradient_.coord_[1]);

  BDMCubicDomain bdm_cubdom;
  bdm_cubdom.disc_fd_ = disc_interf_data;
  EXPECT_FALSE(bdm_cubdom.is_init());
  EXPECT_EQ(0, bdm_cubdom.disc_fd_.ecm_density_gradient_.coord_[0]);
}
}  // namespace bdm
