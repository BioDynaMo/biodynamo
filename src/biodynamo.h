#ifndef BIODYNAMO_H_
#define BIODYNAMO_H_

#include "biology_module_util.h"
#include "cell.h"
#include "command_line_options.h"
#include "compile_time_param.h"
#include "cpptoml/cpptoml.h"
#include "model_initializer.h"
#include "param.h"
#include "resource_manager.h"
#include "scheduler.h"
#include "simulation_object_util.h"
#include "variadic_template_parameter_util.h"
#include "vtune.h"

namespace bdm {

inline void InitializeBioDynamo(int argc, const char** argv) {
  auto options = bdm::DefaultSimulationOptionParser(argc, argv);
  constexpr auto kConfigFile = "bdm.toml";
  if (FileExists(kConfigFile)) {
    auto config = cpptoml::parse_file(kConfigFile);
    Param::AssignFromConfig(config);
  } else {
    Warning("InitializeBioDynamo", "Config file %s not found.", kConfigFile);
  }
  if (options.backup_file_ != "") {
    Param::backup_file_ = options.backup_file_;
    Param::restore_file_ = options.restore_file_;
  }
}

}  // namespace bdm

#endif  // BIODYNAMO_H_
