#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include "cell.h"
#include "daosoa.h"

namespace bdm {

// TODO: cell type should be templated; should also be possible to
// hold different cell types
template <typename Backend>
class ResourceManager {
 public:
  virtual ~ResourceManager() {}

  static ResourceManager<Backend>* Get() {
    static ResourceManager<Backend> instance;
    return &instance;
  }

  // todo change to return const and const method
  const daosoa<Cell, Backend>& GetCells() const { return cells_; }

  void SetCells(const daosoa<Cell, Backend>& cells) { cells_ = cells; }

 private:
  ResourceManager() {}

  daosoa<Cell, Backend> cells_;
};

}  // namespace bdm

#endif  // RESOURCE_MANAGER_H_
