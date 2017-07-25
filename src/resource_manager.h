#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include "cell.h"

namespace bdm {

// TODO(lukas) rework and prepare for multi cell simulations
template <typename TCells>
class ResourceManager {
 public:
  static ResourceManager<TCells>* Get() {
    static ResourceManager<TCells> kInstance;
    return &kInstance;
  }

  TCells* GetCells() { return cells_; }
  void SetCells(TCells* cells) {
    delete cells_;
    cells_ = cells;
  }

 private:
  ResourceManager() {
    cells_ = new TCells();
    cells_->clear();
  }

  TCells* cells_;
};

}  // namespace bdm

#endif  // RESOURCE_MANAGER_H_
