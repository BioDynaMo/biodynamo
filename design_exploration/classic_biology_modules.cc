#include <array>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include "timing.h"

namespace bdm {

enum Event { kCellDivision, kNeuriteBranching };

template <typename TSimulationObject>
struct BiologyModule {
  virtual ~BiologyModule() {}

  virtual void Run(TSimulationObject* simulation_object) = 0;

  virtual bool IsCopied(Event event) const { return false; }

  virtual BiologyModule<TSimulationObject>* Copy() const = 0;
};

struct Cell {
  Cell() : position_{1, 2, 3} {}

  Cell(Cell&& other)
      : diameter_(other.diameter_),
        position_(std::move(other.position_)),
        biology_modules_(std::move(other.biology_modules_)) {}

  Cell(const Cell& other)
      : diameter_(other.diameter_), position_(other.position_) {
    for (auto module : other.biology_modules_) {
      if (module->IsCopied(Event::kCellDivision)) {
        biology_modules_.push_back(module->Copy());
      }
    }
  }

  ~Cell() {
    for (auto module : biology_modules_) {
      delete module;
    }
  }

  void AddBiologyModule(BiologyModule<Cell>* module) {
    biology_modules_.push_back(module);
  }

  void RunBiologyModules() {
    for (auto module : biology_modules_) {
      module->Run(this);
    }
  }

  double diameter_ = 42;
  std::array<double, 3> position_;
  std::vector<BiologyModule<Cell>*> biology_modules_;
};

template <typename TSimulationObject>
struct GrowthModule : public BiologyModule<TSimulationObject> {
  double growth_rate_ = 0.5;

  void Run(TSimulationObject* t) override { t->diameter_ += growth_rate_; }

  bool IsCopied(Event event) const override {
    return event == Event::kCellDivision;
  }

  GrowthModule<TSimulationObject>* Copy() const override {
    return new GrowthModule<TSimulationObject>(*this);
  }
};

template <typename TSimulationObject>
struct MovementModule : public BiologyModule<TSimulationObject> {
  std::array<double, 3> velocity_;

  explicit MovementModule(const std::array<double, 3>& velocity)
      : velocity_(velocity) {}

  void Run(TSimulationObject* t) override {
    t->position_[0] += velocity_[0];
    t->position_[1] += velocity_[1];
    t->position_[2] += velocity_[2];
  }

  bool IsCopied(Event event) const override { return false; }

  MovementModule<TSimulationObject>* Copy() const override {
    return new MovementModule<TSimulationObject>(*this);
  }
};

void Benchmark() {
  const size_t n = 1e6;

  std::vector<Cell> cells;
  cells.reserve(n);

  // create cells
  auto create_timer = new Timing("create");
  for (size_t i = 0; i < n; i++) {
    Cell cell;
    cell.AddBiologyModule(new GrowthModule<Cell>());
    cell.AddBiologyModule(new MovementModule<Cell>({2, 3, 4}));
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
  std::vector<Cell> other_cells;
  other_cells.reserve(n);

  auto copy_timer = new Timing("copy  ");
  for (auto& cell : cells) {
    Cell copy(cell);
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
