#ifndef CELL_TEST_H_
#define CELL_TEST_H_

#include "biology_module_util.h"

namespace bdm {
namespace cell_test_internal {

struct GrowthModule {
  double growth_rate_ = 0.5;

  template <typename T>
  void Run(T* t) {
    t->SetDiameter(t->GetDiameter() + growth_rate_);
  }

  bool IsCopied(Event event) const { return event == Event::kCellDivision; }
};

struct MovementModule {
  std::array<double, 3> velocity_;

  MovementModule() : velocity_({{0, 0, 0}}) {}
  explicit MovementModule(const std::array<double, 3>& velocity)
      : velocity_(velocity) {}

  template <typename T>
  void Run(T* t) {
    const auto& position = t->GetPosition();
    t->SetPosition(Matrix::Add(position, velocity_));
  }

  bool IsCopied(Event event) const { return false; }
};

}  // namespace cell_test_internal
}  // namespace bdm

#endif  // CELL_TEST_H_
