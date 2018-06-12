#ifndef FOO_1_
#define FOO_1_

#include "resource_manager.h"
#include "grid.h"
#include "scheduler.h"
#include "version.h"
#include "command_line_options.h"
#include "cpptoml/cpptoml.h"

namespace bdm {

template <typename T>
BdmSim<T>* BdmSim<T>::active_ = nullptr;

template <typename T>
BdmSim<T>* BdmSim<T>::GetBdm() {
  return active_;
}

template <typename T>
BdmSim<T>::BdmSim(int argc, const char** argv) {
  Activate();
  InitializeMembers();
  InitializeSimulation(argc, argv);
}

template <typename T>
BdmSim<T>::BdmSim(const std::string& executable_name) {
  Activate();
  InitializeMembers();
  const char* argv[1] = {executable_name.c_str()};
  InitializeSimulation(1, argv);
}

template <typename T>
BdmSim<T>::~BdmSim() {
  if (active_ == this) {
    active_ = nullptr;
    // FIXME anything else; catalyst adaptor?
  }
  delete rm_;
  delete grid_;
  delete scheduler_;
}

template <typename T>
void BdmSim<T>::Activate() {
  active_ = this;
  // TODO reset certain components
  // e.g. CatalystAdaptor
}

template <typename T>
ResourceManager<T>* BdmSim<T>::GetRm() { return rm_; }

template <typename T>
Grid<BdmSim<T>>* BdmSim<T>::GetGrid() { return grid_; }

template <typename T>
Scheduler<BdmSim<T>>* BdmSim<T>::GetScheduler() { return scheduler_; }

template <typename T>
template <typename TResourceManager,
          typename TGrid,
          typename TScheduler>
void BdmSim<T>::InitializeMembers() {
  rm_ = new TResourceManager();
  grid_ = new TGrid();
  scheduler_ = new TScheduler();
}

template <typename T>
void BdmSim<T>::InitializeSimulation(int argc, const char** argv) {
  // Removing this line causes an unexplainable segfault due to setting the
  // gErrorIngoreLevel global parameter of ROOT. We need to log at least one
  // thing before setting that parameter.
  Log::Info("", "Initialize new simulation using BioDynaMo ", Version::String());

  // detect if the biodynamo environment has been sourced
  if (std::getenv("BDM_CMAKE_DIR") == nullptr) {
    Log::Fatal("BdmSim::InitializeSimulation",
               "The BioDynaMo environment is not set up correctly. Please call "
               "$use_biodynamo "
               "and retry this command.");
  }

  Param::executable_name_ = ExtractExecutableName(argv[0]);
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
    Log::Warning("BdmSim::InitializeSimulation",
                 "Config file %s not found in `.` or `../` directory.",
                 kConfigFile);
  }
  if (options.backup_file_ != "") {
    Param::backup_file_ = options.backup_file_;
    Param::restore_file_ = options.restore_file_;
  }
}


template <typename T>
std::string BdmSim<T>::ExtractExecutableName(const char* path) {
  std::string s(path);
  auto pos = s.find_last_of("/");
  if (pos == std::string::npos) {
    return s;
  } else {
    return s.substr(pos + 1, s.length() - 1);
  }
}

}  // namespace bdm

#endif // FOO_1_
