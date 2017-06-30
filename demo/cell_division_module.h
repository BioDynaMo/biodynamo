#ifndef DEMO_CELL_DIVISION_MODULE_H_
#define DEMO_CELL_DIVISION_MODULE_H_

#include "biology_module_util.h"
#include "resource_manager.h"

namespace bdm {

struct GrowthModule {
  template <typename T>
  void Run(T* cell) {
    if (cell->GetDiameter() <= 40) {
      cell->ChangeVolume(300);
    } else {
      Divide(
          *cell,
          ResourceManager<Cell<Soa, variant<GrowthModule>>>::Get()->GetCells());
    }
  }

  bool IsCopied(Event event) const { return true; }
};

}  // namespace bdm

#endif  // DEMO_CELL_DIVISION_MODULE_H_
