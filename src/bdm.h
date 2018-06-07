#ifndef FOO
#define FOO

namespace bdm {

// forward declarations
template <typename > struct ResourceManager;
template <typename> struct Grid;
template <typename> struct Scheduler;


template <typename TCTParam = CompileTimeParam<>>
struct BdmSim {
  using Self = BdmSim<TCTParam>;
  using ResourceManager_t = ResourceManager<>;  // NOLINT


  /// This function returns the currently active BdmSim simulation.
  static BdmSim<TCTParam>* GetBdm();

  /// Creation of a new simulation automatically activates it!
  BdmSim(int argc, const char** argv);
  BdmSim(const std::string& executable_name);
  ~BdmSim();

  // thread_local int random_;
  void Activate();

  ResourceManager<TCTParam>* GetRm();
  Grid<Self>* GetGrid();
  Scheduler<Self>* GetScheduler();

  // parameter
  // random numbers
  // visualization

 private:
  static BdmSim<TCTParam>* active_;

  ResourceManager<>* rm_ = nullptr;
  Grid<Self>* grid_ = nullptr;
  Scheduler<Self>* scheduler_ = nullptr;

  // TODO
  template <typename TResourceManager = ResourceManager<>,
            typename TGrid = Grid<Self>,
            typename TScheduler = Scheduler<Self>>
  void InitializeMembers();

  /// Return only the executable name given the path
  /// @param path path and filename of the executable
  /// e.g. `executable`, `./executable`, './build/executable'
  /// @return `executable`
  std::string ExtractExecutableName(const char* path);

  /// This function parses command line parameters and the configuration file.
  /// @param argc argument count from main function
  /// @param argv argument vector from main function
  void InitializeSimulation(int argc, const char** argv);
};

}  // namespace bdm

#endif // FOO
