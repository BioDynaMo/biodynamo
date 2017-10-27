#include "biodynamo.h"
#include <fstream>
#include "gtest/gtest.h"

namespace bdm {

const char* gConfigFileName = "bdm.toml";
const char* gConfigContent =
    "[simulation]\n"
    "backup_file = \"backup.root\"\n"
    "restore_file = \"restore.root\"\n"
    "backup_interval = 3600\n"
    "time_step = 0.0125\n"
    "max_displacement = 2.0\n"
    "run_mechanical_interactions = false\n"
    "bound_space = true\n"
    "min_bound = -100\n"
    "max_bound =  200\n"
    "\n"
    "[visualization]\n"
    "live = true\n"
    "export = true\n"
    "export_interval = 100\n"
    "\n"
    "  [[visualize_sim_object]]\n"
    "  name = \"Cell\"\n"
    "\n"
    "  [[visualize_sim_object]]\n"
    "  name = \"Neurite\"\n"
    "  additional_data_members = [ \"spring_axis_\", \"tension_\" ]\n"
    "\n"
    "\n"
    "  [[visualize_diffusion]]\n"
    "  name = \"Na\"\n"
    "  concentration = false\n"
    "  gradient = true\n"
    "\n"
    "  [[visualize_diffusion]]\n"
    "  name = \"K\"\n"
    "\n"
    "[development]\n"
    "# this is a comment\n"
    "output_op_runtime = true\n";

void ValidateNonCLIParameter() {
  EXPECT_EQ(3600u, Param::backup_interval_);
  EXPECT_EQ(0.0125, Param::simulation_time_step_);
  EXPECT_EQ(2.0, Param::simulation_max_displacement_);
  EXPECT_FALSE(Param::run_mechanical_interactions_);
  EXPECT_TRUE(Param::bound_space_);
  EXPECT_EQ(-100, Param::min_bound_);
  EXPECT_EQ(200, Param::max_bound_);
  EXPECT_TRUE(Param::live_visualization_);
  EXPECT_TRUE(Param::export_visualization_);
  EXPECT_EQ(100u, Param::visualization_export_interval_);

  // visualize_sim_object
  EXPECT_EQ(2u, Param::visualize_sim_objects_.size());
  auto it = Param::visualize_sim_objects_.cbegin();
  uint64_t counter = 0;
  while (it != Param::visualize_sim_objects_.cend()) {
    if (counter == 1) {
      EXPECT_EQ("Cell", (*it).first);
      EXPECT_EQ(0u, (*it).second.size());
    } else if (counter == 0) {
      EXPECT_EQ("Neurite", (*it).first);
      auto additional_dm = (*it).second;
      EXPECT_EQ(2u, additional_dm.size());
      EXPECT_TRUE(additional_dm.find("spring_axis_") != additional_dm.end());
      EXPECT_TRUE(additional_dm.find("tension_") != additional_dm.end());
    }
    counter++;
    it++;
  }

  // visualize_diffusion
  EXPECT_EQ(2u, Param::visualize_diffusion_.size());
  for (uint64_t i = 0; i < 2; i++) {
    auto vd = Param::visualize_diffusion_[i];
    if (i == 0) {
      EXPECT_EQ("Na", vd.name_);
      EXPECT_FALSE(vd.concentration_);
      EXPECT_TRUE(vd.gradient_);
    } else if (i == 1) {
      EXPECT_EQ("K", vd.name_);
      EXPECT_TRUE(vd.concentration_);
      EXPECT_FALSE(vd.gradient_);
    }
  }

  EXPECT_TRUE(Param::output_op_runtime_);
}

TEST(BiodynamoTest, InitializeBioDynamo) {
  remove(gConfigFileName);

  std::ofstream config_file(gConfigFileName);
  config_file << gConfigContent;
  config_file.close();

  const char* argv[1] = {"binary_name"};
  InitializeBioDynamo(1, argv);

  EXPECT_EQ("backup.root", Param::backup_file_);
  EXPECT_EQ("restore.root", Param::restore_file_);
  ValidateNonCLIParameter();

  Param::Reset();
  remove(gConfigFileName);
}

TEST(BiodynamoTest, InitializeBioDynamoWithCLIArguments) {
  remove(gConfigFileName);

  std::ofstream config_file(gConfigFileName);
  config_file << gConfigContent;
  config_file.close();

  const char* argv[5] = {"binary_name", "-b", "mybackup.root", "-r",
                         "myrestore.root"};
  InitializeBioDynamo(5, argv);

  // the following two parameters should contain the values from the command
  // line arguments.
  EXPECT_EQ("mybackup.root", Param::backup_file_);
  EXPECT_EQ("myrestore.root", Param::restore_file_);
  ValidateNonCLIParameter();

  Param::Reset();
  remove(gConfigFileName);
}

}  // namespace bdm
