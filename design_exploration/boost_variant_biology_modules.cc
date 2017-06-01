#include <array>
#include <cmath>
#include <iostream>
#include <vector>

#include "boost/variant.hpp"
#include "timing.h"

namespace bdm {

using boost::variant;
using boost::apply_visitor;

enum Event { kCellDivision, kNeuriteBranching };

template <typename TSimulationObject>
struct RunVisitor : public boost::static_visitor<void> {
  explicit RunVisitor(TSimulationObject* const so) : kSimulationObject(so) {}

  template <typename T>
  void operator()(T& t) const {
    t.Run(kSimulationObject);
  }

 private:
  TSimulationObject* const kSimulationObject;
};

template <typename TVector>
struct CopyVisitor : public boost::static_visitor<void> {
  CopyVisitor(Event event, TVector* vector) : kEvent(event), vector_(vector) {}

  template <typename T>
  void operator()(const T& from) const {
    if (from.IsCopied(kEvent)) {
      T copy(from);
      vector_->emplace_back(std::move(copy));
    }
  }

  const Event kEvent;
  TVector* vector_;
};

template <typename LBMVariant>
struct Cell {
  Cell() : position_{1, 2, 3} {}

  Cell(Cell<LBMVariant>&& other)
      : diameter_(other.diameter_),
        position_(std::move(other.position_)),
        biology_modules_(std::move(other.biology_modules_)) {}

  Cell(const Cell<LBMVariant>& other)
      : diameter_(other.diameter_), position_(other.position_) {
    CopyVisitor<std::vector<LBMVariant> > visitor(Event::kCellDivision,
                                                  &biology_modules_);
    for (auto& module : other.biology_modules_) {
      apply_visitor(visitor, module);
    }
  }

  template <typename TBiologyModule>
  void AddBiologyModule(TBiologyModule&& module) {
    biology_modules_.emplace_back(module);
  }

  void RunBiologyModules() {
    RunVisitor<Cell<LBMVariant> > visitor(this);
    for (auto& module : biology_modules_) {
      apply_visitor(visitor, module);
    }
  }

  double diameter_ = 42;
  std::array<double, 3> position_;
  std::vector<LBMVariant> biology_modules_;
};

struct GrowthModule {
  double growth_rate_ = 0.5;

  template <typename T>
  void Run(T* t) {
    t->diameter_ += growth_rate_;
  }

  bool IsCopied(Event event) const { return event == Event::kCellDivision; }
};

struct MovementModule {
  std::array<double, 3> velocity_;

  explicit MovementModule(const std::array<double, 3>& velocity)
      : velocity_(velocity) {}

  template <typename T>
  void Run(T* t) {
    t->position_[0] += velocity_[0];
    t->position_[1] += velocity_[1];
    t->position_[2] += velocity_[2];
  }

  bool IsCopied(Event event) const { return false; }
};

void Benchmark() {
  const size_t n = 1e6;
  typedef variant<GrowthModule, MovementModule> BiologyModules;
  typedef Cell<BiologyModules> MyCell;

  std::vector<MyCell> cells;
  cells.reserve(n);

  // create cells
  auto create_timer = new Timing("create");
  for (size_t i = 0; i < n; i++) {
    MyCell cell;
    cell.AddBiologyModule(GrowthModule());
    cell.AddBiologyModule(MovementModule({2, 3, 4}));
    cells.emplace_back(std::move(cell));
  }
  delete create_timer;

  // run biology modules
  auto run_timer = new Timing("run   ");
  for (size_t i = 0; i < 100; i++) {
    for (auto& cell : cells) {
      cell.RunBiologyModules();
    }
  }
  delete run_timer;

  // copy cells
  std::vector<MyCell> other_cells;
  other_cells.reserve(n);

  auto copy_timer = new Timing("copy  ");
  for (auto& cell : cells) {
    MyCell copy(cell);
    other_cells.emplace_back(std::move(copy));
  }
  delete copy_timer;

  // Validate sample
  bool error = false;

  error |= cells[3].biology_modules_.size() != 2;
  error |= std::abs(cells[3].diameter_ - 92) > 1e-5;
  error |= std::abs(cells[3].position_[0] - 201) > 1e-5;
  error |= std::abs(cells[3].position_[1] - 302) > 1e-5;
  error |= std::abs(cells[3].position_[2] - 403) > 1e-5;
  error |= other_cells[3].biology_modules_.size() != 1;

  if (error) {
    std::cerr << "ERROR: result validation of sample failed" << std::endl;
    std::cout << cells[3].biology_modules_.size() << std::endl;
    std::cout << cells[3].diameter_ << std::endl;
    std::cout << cells[3].position_[0] << std::endl;
    std::cout << cells[3].position_[1] << std::endl;
    std::cout << cells[3].position_[2] << std::endl;
    std::cout << other_cells[3].biology_modules_.size() << std::endl;
  }
}

}  // namespace bdm

int main() {
  {
    bdm::Timing timing("total ");
    bdm::Benchmark();
  }

  return 0;
}
