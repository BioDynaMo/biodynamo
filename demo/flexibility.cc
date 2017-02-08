// This example demonstrates how classes can be extended / modified using
// mixins and templates
// Contruction problem is solved using variadic templates and perfect forwarding

#include <array>
#include <vector>
#include <utility>
#include <iostream>
#include "timing.h"

// -----------------------------------------------------------------------------
// core library only defines minimal cell
class BaseCell {
  protected: std::array<double, 3> position_;

 public:
  explicit BaseCell(const std::array<double, 3>& pos) : position_(pos) {}
  BaseCell() : position_{0, 0, 0} {}
  const std::array<double, 3>& GetPosition() const { return position_; }
};

template <typename Cell>
void CoreOp(Cell* cell) {
  std::cout << cell->GetPosition()[2] << std::endl;
}

// -----------------------------------------------------------------------------
// libraries for specific specialities add functionality - e.g. Neuroscience
class Neurite {};

// adds Neurites to BaseCell
template <typename Base, typename TNeurite = Neurite>
class Neuron : public Base {
  std::vector<TNeurite> neurites_;

 public:
  template <class... A>
  explicit Neuron(const std::vector<TNeurite>& neurites, const A&... a)
      : Base(a...), neurites_{neurites} {}
  Neuron() = default;
  const std::vector<TNeurite>& GetNeurites() const { return neurites_; }
};

// -----------------------------------------------------------------------------
// code written by life scientists using package core and Neuroscience extension
template <typename Base>
class NeuronExtension : public Base {
  double foo_ = 3.14;

 public:
  template <class... A>
  explicit NeuronExtension(double foo, const A&... a) : Base(a...), foo_{foo} {}
  NeuronExtension() = default;
  double GetFoo() const {
    volatile auto& foo = Base::position_;
    return foo_;
  }
};

template <typename Cell>
void CustomOp(const Cell& cell) {
  std::cout << cell->GetNeurites().size() << "-" << cell->GetFoo() << std::endl;
}

int main() {
  typedef NeuronExtension<Neuron<BaseCell>> CustomNeuron;
  // note: also Neurites can be modified and then inserted into Neuron
  // typedef NeuriteExtension<Neurite> CustomNeurite;
  // typedef NeuronExtension<Neuron<BaseCell, CustomNeurite>> CustomNeuron;

  // default constructor
  CustomNeuron cell;
  CoreOp(&cell);
  CustomOp(&cell);

  // pass initialization values to NeuronExtension, Neuron and BaseCell
  CustomNeuron cell2(1.2, std::vector<Neurite>{Neurite()},
                     std::array<double, 3>{1, 2, 3});
  CoreOp(&cell2);    // prints: 3
  CustomOp(&cell2);  // prints: 1-1.2

  // small benchmark
  double sum = 0;
  {
    bdm::Timing timing;
    for (int i = 0; i < 10e6; i++) {
      sum += cell.GetFoo();
    }
  }
  std::cout << "Sum " << sum << std::endl;

  return 0;
}
