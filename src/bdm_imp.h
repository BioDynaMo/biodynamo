#ifndef FOO_1_
#define FOO_1_

#include "resource_manager.h"
#include "grid.h"
// #include "scheduler.h"

namespace bdm {


template <typename T>
BdmSim<T>::BdmSim() { Init(); }

template <typename T>
BdmSim<T>::~BdmSim() {
  delete rm_;
  delete grid_;
  // delete scheduler_;
}

template <typename T>
ResourceManager<T>* BdmSim<T>::GetRm() { return rm_; }

template <typename T>
Grid<typename BdmSim<T>::Self>* BdmSim<T>::GetGrid() { return grid_; }

template <typename T>
void BdmSim<T>::Activate() {
  active_ = this;
  // TODO reset certain components
  // e.g. CatalystAdaptor
}

template <typename T>
BdmSim<T>* BdmSim<T>::GetBdm() {
  return active_;
}

template <typename T>
template <typename TResourceManager,
          typename TGrid>
void BdmSim<T>::Init() {
  rm_ = new TResourceManager();
  grid_ = new TGrid();
}

template <typename T>
BdmSim<T>* BdmSim<T>::active_ = nullptr;

}  // namespace bdm

#endif // FOO_1_
