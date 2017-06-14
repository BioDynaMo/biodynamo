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

  TCells* GetCells() { return &cells_; }

 private:
  ResourceManager() { cells_.clear(); }

  TCells cells_;
};

}  // namespace bdm

#endif  // RESOURCE_MANAGER_H_
