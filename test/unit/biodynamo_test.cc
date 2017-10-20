#include "biodynamo.h"
#include <fstream>
#include "gtest/gtest.h"

namespace bdm {

const char* kConfigFileName = "bdm.toml";
const char* kConfigContent =
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
    "[development]\n"
    "# this is a comment\n"
    "output_op_runtime = true\n";

TEST(BiodynamoTest, InitializeBioDynamo) {
  remove(kConfigFileName);

  std::ofstream config_file(kConfigFileName);
  config_file << kConfigContent;
  config_file.close();

  const char* argv[1] = {"binary_name"};
  InitializeBioDynamo(1, argv);

  EXPECT_EQ("backup.root", Param::backup_file_);
  EXPECT_EQ("restore.root", Param::restore_file_);
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
  EXPECT_TRUE(Param::output_op_runtime_);

  Param::Reset();
  remove(kConfigFileName);
}

TEST(BiodynamoTest, InitializeBioDynamoWithCLIArguments) {
  remove(kConfigFileName);

  std::ofstream config_file(kConfigFileName);
  config_file << kConfigContent;
  config_file.close();

  const char* argv[5] = {"binary_name", "-b", "mybackup.root", "-r",
                         "myrestore.root"};
  InitializeBioDynamo(5, argv);

  // the following two parameters should contain the values from the command
  // line arguments.
  EXPECT_EQ("mybackup.root", Param::backup_file_);
  EXPECT_EQ("myrestore.root", Param::restore_file_);
  EXPECT_EQ(3600u, Param::backup_interval_);
  // remaining ones should contain the values from the config file
  EXPECT_EQ(0.0125, Param::simulation_time_step_);
  EXPECT_EQ(2.0, Param::simulation_max_displacement_);
  EXPECT_FALSE(Param::run_mechanical_interactions_);
  EXPECT_TRUE(Param::bound_space_);
  EXPECT_EQ(-100, Param::min_bound_);
  EXPECT_EQ(200, Param::max_bound_);
  EXPECT_TRUE(Param::live_visualization_);
  EXPECT_TRUE(Param::export_visualization_);
  EXPECT_EQ(100u, Param::visualization_export_interval_);
  EXPECT_TRUE(Param::output_op_runtime_);

  Param::Reset();
  remove(kConfigFileName);
}

}  // namespace bdm
