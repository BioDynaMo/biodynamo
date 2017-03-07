#include <gtest/gtest.h>
//#include "cell.h"
#include "bdm_interface.h"

namespace bdm {

TEST(BDMInterfaceTest, Vector3DInterface) {
  static Vector3DInterface v3Dinterf;

  EXPECT_EQ(v3Dinterf.comp[0], 0);
  v3Dinterf.comp[1] = 1.2;
  EXPECT_EQ(v3Dinterf.comp[1], 1.2);

  ContinuousInterfaceData cont_interf_data;

  cont_interf_data.oxygen_level_gradient.push_back(v3Dinterf);
  EXPECT_EQ(cont_interf_data.oxygen_level_gradient[0].comp[1], 0);
  EXPECT_EQ(cont_interf_data.oxygen_level_gradient[8].comp[1], 1.2);

  DiscontinuousInterfaceData disc_interf_data;
  disc_interf_data.ecm_density_gradient = v3Dinterf;
  EXPECT_EQ(disc_interf_data.ecm_density_gradient.comp[1], 1.2);

  BDMCubicDomain bdm_cubdom;
  bdm_cubdom.disc_fd = disc_interf_data;
  EXPECT_EQ(bdm_cubdom.is_init(), false);
  EXPECT_EQ(bdm_cubdom.disc_fd.ecm_density_gradient.comp[0], 0);
}
}
