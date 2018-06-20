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
struct BdmSimTest;

/// This is the central BioDynaMo object. It containes pointers to e.g. the
/// ResourceManager, the scheduler, parameters, ... \n
/// It is possible to create multiple objects, but only one can be active at
/// the same time. Creating a new simulation object automatically activates it.
template <typename TCTParam = CompileTimeParam<>>
struct BdmSim {
  using ResourceManager_t = ResourceManager<TCTParam>;  // NOLINT

  /// This function returns the currently active BdmSim simulation.
  static BdmSim<TCTParam>* GetActive();

  BdmSim(TRootIOCtor*);
  /// Constructor that takes the arguments from `main` to parse command line
  ///  options. The simulation name is extracted from the executable name.
  /// Creation of a new simulation automatically activates it.
  BdmSim(int argc, const char** argv);
  /// Alternative constructor, if the arguments from function `main` are not
  /// available, or if a different simulation name should be chosen. \n
  /// Command line options are not parsed.\n
  /// Creation of a new simulation automatically activates it.
  BdmSim(const std::string& simulation_name);
  ~BdmSim();

  /// Copies / moves values from a restored simulation into this object.
  /// Thus, pointers to `rm_`, `param_`, ... are not invalidated.
  void Restore(BdmSim&& restored);

  /// Activates this simulation.
  void Activate();

  ResourceManager<TCTParam>* GetRm();
  Param* GetParam();
  Grid<BdmSim>* GetGrid();
  Scheduler<BdmSim>* GetScheduler();

  /// Returns a thread local random number generator
  Random* GetRandom();

  /// @see `simulation_id_`
  std::string GetSimulationId() const;

  /// Replaces the scheduler for this simulation.
  /// Existing scheduler will be deleted! Therefore, pointers to the old
  /// scheduler (obtained with `GetScheduler()`) will be invalidated.
  void ReplaceScheduler(Scheduler<BdmSim>*);


 private:
  /// Currently active simulation
  static BdmSim<TCTParam>* active_;
  /// Number of simulations in this process
  static std::atomic<uint64_t> sim_counter_;

  /// random number generator for each thread
  std::vector<Random*> random_;

  ResourceManager<TCTParam>* rm_ = nullptr;
  Param* param_ = nullptr;
  std::string simulation_name_;
  Grid<BdmSim>* grid_ = nullptr; //!
  Scheduler<BdmSim>* scheduler_ = nullptr;  //!
  /// This id is unique for each simulation within the same process
  uint64_t id_ = 0; //!
  /// cached value where `id_` is appended to `simulation_name_` if `id_` is
  /// not zero.
  /// e.g. `simulation_name_ = "my-sim"` and `id_ = 0` -> "my-sim"
  /// e.g. `simulation_name_ = "my-sim"` and `id_ = 4` -> "my-sim4"
  std::string simulation_id_; //!

  /// Initialize BdmSim
  void Initialize(int argc, const char** argv);

  /// Initialize data members that have a dependency on BdmSim
  template <typename TResourceManager = ResourceManager<TCTParam>,
            typename TGrid = Grid<BdmSim>,
            typename TScheduler = Scheduler<BdmSim>>
  void InitializeMembers();

  /// Return only the executable name given the path
  /// @param path path and filename of the executable
  /// e.g. `executable`, `./executable`, './build/executable'
  /// @return `executable`
  std::string ExtractSimulationName(const char* path);

  /// This function parses command line parameters and the configuration file.
  /// @param argc argument count from main function
  /// @param argv argument vector from main function
  void InitializeRuntimeParams(int argc, const char** argv);

  /// This function initialzes `simulation_name_` and `simulatio_id_`
  void InitializeSimulationId(const std::string& simulation_name);

  friend BdmSimTest;

  ClassDefNV(BdmSim, 1);
};

}  // namespace bdm

#endif // FOO
