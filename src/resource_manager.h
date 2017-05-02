#ifndef RESOURCE_MANAGER_H_
#define RESOURCE_MANAGER_H_

#include "cell.h"

namespace bdm {

// TODO(lukas): cell type should be templated; should also be possible to
// hold different cell types
template <typename Backend>
class ResourceManager {
 public:
  template <typename T>
  using Container = typename Backend::template Container<T>;

  virtual ~ResourceManager() {}

  static ResourceManager<Backend>* Get() {
    static ResourceManager<Backend> instance;
    return &instance;
  }

  // todo change to return const and const method
  const Container<Cell<Backend>>& GetCells() const { return cells_; }

  void SetCells(const Container<Cell<Backend>>& cells) { cells_ = cells; }

 private:
  ResourceManager() {}

  Container<Cell<Backend>> cells_;
};

}  // namespace bdm

#endif  // RESOURCE_MANAGER_H_
