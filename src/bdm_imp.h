#ifndef FOO_1_
#define FOO_1_

#include "resource_manager.h"
#include "grid.h"
#include "scheduler.h"
#include "version.h"
#include "param.h"
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
  param_ = new Param();
  InitializeRuntimeParams(argc, argv);
  InitializeMembers();
}

template <typename T>
BdmSim<T>::BdmSim(const std::string& executable_name) {
  Activate();
  param_ = new Param();
  const char* argv[1] = {executable_name.c_str()};
  InitializeRuntimeParams(1, argv);
  InitializeMembers();
}

template <typename T>
BdmSim<T>& BdmSim<T>::operator=(BdmSim<T>&& other) {
  // delete rm_;
  // delete grid_;
  // delete scheduler_;
  //
  // rm_ = other.rm_;
  // grid_ = other.grid_;
  // scheduler_ = other.scheduler_;
  //
  // other.rm_ = nullptr;
  // other.grid_ = nullptr;
  // other.scheduler_ = nullptr;
  *param_ = *other.param_;
  *rm_ = std::move(*other.rm_);
  // *grid_ = std::move(*other.grid_);
  // delete grid_;
  // grid_ = other.grid_;
  return *this;
}

template <typename T>
BdmSim<T>::~BdmSim() {
  delete rm_;
  delete grid_;
  delete scheduler_;
  delete param_;
  if (active_ == this) {
    active_ = nullptr;
    // FIXME anything else; catalyst adaptor?
  }
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
Param* BdmSim<T>::GetParam() { return param_; }

template <typename T>
Grid<BdmSim<T>>* BdmSim<T>::GetGrid() { return grid_; }

template <typename T>
Scheduler<BdmSim<T>>* BdmSim<T>::GetScheduler() { return scheduler_; }

template <typename T>
void BdmSim<T>::ReplaceScheduler(Scheduler<BdmSim<T>>* scheduler) {  // TODO use unique_ptr to make ownership transformation explicit
  delete scheduler_;
  scheduler_ = scheduler;
}

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
template <typename TGrid,
          typename TScheduler>
void BdmSim<T>::TRootIoCtorInitializeMembers() {
  grid_ = new TGrid();
}

template <typename T>
void BdmSim<T>::InitializeRuntimeParams(int argc, const char** argv) {
  // Removing this line causes an unexplainable segfault due to setting the
  // gErrorIngoreLevel global parameter of ROOT. We need to log at least one
  // thing before setting that parameter.
  Log::Info("", "Initialize new simulation using BioDynaMo ", Version::String());

  // detect if the biodynamo environment has been sourced
  if (std::getenv("BDM_CMAKE_DIR") == nullptr) {
    Log::Fatal("BdmSim::InitializeRuntimeParams",
               "The BioDynaMo environment is not set up correctly. Please call "
               "$use_biodynamo "
               "and retry this command.");
  }

  param_->executable_name_ = ExtractExecutableName(argv[0]);
  auto options = bdm::DefaultSimulationOptionParser(argc, argv);
  constexpr auto kConfigFile = "bdm.toml";
  constexpr auto kConfigFileParentDir = "../bdm.toml";
  if (FileExists(kConfigFile)) {
    auto config = cpptoml::parse_file(kConfigFile);
    param_->AssignFromConfig(config);
  } else if (FileExists(kConfigFileParentDir)) {
    auto config = cpptoml::parse_file(kConfigFileParentDir);
    param_->AssignFromConfig(config);
  } else {
    Log::Warning("BdmSim::InitializeRuntimeParams",
                 "Config file %s not found in `.` or `../` directory.",
                 kConfigFile);
  }
  if (options.backup_file_ != "") {
    param_->backup_file_ = options.backup_file_;
    param_->restore_file_ = options.restore_file_;
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
