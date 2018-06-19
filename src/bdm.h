#ifndef FOO
#define FOO

#include <string>
#include <Rtypes.h>
#include "random.h"

namespace bdm {

// forward declarations
template <typename > struct ResourceManager;
template <typename> struct Grid;
template <typename> struct Scheduler;
struct Param;

struct Soa;
template <typename TBackend = Soa>
struct CompileTimeParam;

/// only one active simulation at a time...
template <typename TCTParam = CompileTimeParam<>>
struct BdmSim {
  using Self = BdmSim<TCTParam>; // TODO remove not needed
  using ResourceManager_t = ResourceManager<TCTParam>;  // NOLINT

  /// This function returns the currently active BdmSim simulation.
  static BdmSim<TCTParam>* GetBdm();

  /// Creation of a new simulation automatically activates it!
  // FIXME move to bdm_imp.h
  BdmSim(TRootIOCtor*) {
    //TRootIoCtorInitializeMembers(); FIXME can it be removed?
  }
  BdmSim(int argc, const char** argv);
  BdmSim(const std::string& simulation_name);
  ~BdmSim();

  BdmSim& operator=(BdmSim&& other);

  // thread_local int random_;
  void Activate();

  ResourceManager<TCTParam>* GetRm();
  Param* GetParam();
  Grid<Self>* GetGrid();
  Scheduler<Self>* GetScheduler();

  /// Returns a thread local random number generator
  NewRandom* GetRandom();

  std::string GetSimulationId() const;

  /// Replaces the scheduler for this simulation.
  /// Existing scheduler will be deleted! Therefore, pointers to the old
  /// scheduler (obtained with `GetScheduler()`) will be invalidated.
  void ReplaceScheduler(Scheduler<Self>*);


 private:
  static BdmSim<TCTParam>* active_;
  static std::atomic<uint64_t> sim_counter_;

  /// random number generator for each thread
  std::vector<NewRandom*> random_;

  ResourceManager<TCTParam>* rm_ = nullptr;
  Param* param_ = nullptr;
  Grid<Self>* grid_ = nullptr; //!
  Scheduler<Self>* scheduler_ = nullptr;  //!
  std::string simulation_name_;
  /// This id is unique for each simulation within the same process
  uint64_t id_ = 0; //!
  /// cached value where `id_` is appended to `simulation_name_` if `id_` is not zero.
  /// e.g. `simulation_name_ = "my-sim"` and `id_ = 0` -> "my-sim"
  /// e.g. `simulation_name_ = "my-sim"` and `id_ = 4` -> "my-sim4"
  std::string simulation_id_; //!

  void Initialize(int argc, const char** argv);

  // TODO
  template <typename TResourceManager = ResourceManager<TCTParam>,
            typename TGrid = Grid<Self>,
            typename TScheduler = Scheduler<Self>>
  void InitializeMembers();

  template <typename TGrid = Grid<Self>,
            typename TScheduler = Scheduler<Self>>
  void TRootIoCtorInitializeMembers();

  /// Return only the executable name given the path
  /// @param path path and filename of the executable
  /// e.g. `executable`, `./executable`, './build/executable'
  /// @return `executable`
  std::string ExtractSimulationName(const char* path);

  /// This function parses command line parameters and the configuration file.
  /// @param argc argument count from main function
  /// @param argv argument vector from main function
  void InitializeRuntimeParams(int argc, const char** argv);

  void InitializeSimulationId(int argc, const char** argv);

  ClassDefNV(BdmSim, 1);
};

}  // namespace bdm

#endif // FOO
