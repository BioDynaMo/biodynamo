#ifndef FOO
#define FOO

namespace bdm {

// forward declarations
template <typename > struct ResourceManager;
template <typename> struct Grid;


template <typename TCTParam = CompileTimeParam<>>
struct BdmSim {
  using Self = BdmSim<TCTParam>;
  using ResourceManager_t = ResourceManager<>;  // NOLINT

  BdmSim();

  ~BdmSim();

  ResourceManager<TCTParam>* GetRm();

  Grid<Self>* GetGrid();

  // thread_local int random_;
  void Activate();

  static BdmSim<TCTParam>* active_;

  /// This function returns the currently active BdmSim simulation.
  static BdmSim<TCTParam>* GetBdm();

  // parameter
  // random numbers
  // Grid
  // scheduler
  // visualization

 private:
  ResourceManager<>* rm_ = nullptr;
  Grid<Self>* grid_ = nullptr;
  // Scheduler<>* scheduler_ = new Scheduler<>();

  template <typename TResourceManager = ResourceManager<>,
            typename TGrid = Grid<Self>>
  void Init();
};

}  // namespace bdm

#endif // FOO
