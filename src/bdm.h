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

  BdmSim();
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

  template <typename TResourceManager = ResourceManager<>,
            typename TGrid = Grid<Self>,
            typename TScheduler = Scheduler<Self>>
  void Init();
};

}  // namespace bdm

#endif // FOO
