#ifndef FOO
#define FOO

namespace bdm {

template <typename> struct ResourceManager;

// // TODO move
// template <typename TCTParam = CompileTimeParam<>>
// struct BdmSim;
//
// static BdmSim<>* gBdm = nullptr;

template <typename TCTParam = CompileTimeParam<>>
struct BdmSim {

  ~BdmSim() {
    delete rm_;
  }

  ResourceManager<TCTParam>* GetRm() { return rm_; }

  ResourceManager<TCTParam>* rm_ = new ResourceManager<TCTParam>();

  // thread_local int random_;
  void Activate() const {
    active_ = this;
    // TODO reset certain components
  }

  static BdmSim<TCTParam>* active_;

  /// This function returns the currently active BdmSim simulation.
  static BdmSim<TCTParam>* GetBdm() {
    return active_;
  }

  // parameter
  // random numbers
  // Grid
  // scheduler
  // visualization
};

template <typename T>
BdmSim<T>* BdmSim<T>::active_ = nullptr;
// template <typename TBdm = BdmSim<>>
// TBdm* Bdm() {
//   return gBdm;
// }

}  // namespace bdm

#endif // FOO
