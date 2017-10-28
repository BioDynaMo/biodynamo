#include "biodynamo.h"
#include "command_line_options.h"
#include "cpptoml/cpptoml.h"

namespace bdm {

void InitializeBioDynamo(int argc, const char** argv) {
  auto options = bdm::DefaultSimulationOptionParser(argc, argv);
  constexpr auto kConfigFile = "bdm.toml";
  constexpr auto kConfigFileParentDir = "../bdm.toml";
  if (FileExists(kConfigFile)) {
    auto config = cpptoml::parse_file(kConfigFile);
    Param::AssignFromConfig(config);
  } else if (FileExists(kConfigFileParentDir)) {
    auto config = cpptoml::parse_file(kConfigFileParentDir);
    Param::AssignFromConfig(config);
  } else {
    Warning("InitializeBioDynamo",
            "Config file %s not found in `.` or `../` directory.", kConfigFile);
  }
  if (options.backup_file_ != "") {
    Param::backup_file_ = options.backup_file_;
    Param::restore_file_ = options.restore_file_;
  }
}

}  // namespace bdm
