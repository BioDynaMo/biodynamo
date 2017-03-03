// This example demonstrates how classes can be extended / modified using
// mixins and templates
// Furthermore, it shows how to remove certain data members and how to select
// different vectorization backends. These backends can be used to select
// SOA or AOSOA memory layout without code changes inside the class or client
// side code

#include <array>
#include <iostream>
#include <string>
// #include <typeinfo>
// #include <utility>
#include <vector>
// #include <stdexcept>

// #include <Vc/Vc>
//
// #include "cpp_magic.h"
#include "timing.h"

// using std::ostream;
// using std::enable_if;
// using std::is_same;

#include "multiform_object.h"
#include "simulation_object.h"
#include "simulation_object_util.h"

//TODO remove
using namespace bdm;

template <typename Base = BdmSimObject<>>
class BaseCell : public Base {
  BDM_CLASS_HEADER(BaseCell, BaseCell<>,
                   BaseCell<typename Base::template Self<Backend>>, position_,
                   unused_);

 public:
  explicit BaseCell(const std::array<real_v, 3>& pos) : position_{{pos}} {}

  BaseCell() : position_{{0, 0, 0}} {}

  const std::array<real_v, 3>& GetPosition() const { return position_[idx_]; }

 protected:
  BDM_PROTECTED_MEMBER(Container<std::array<real_v COMMA() 3>>, position_);
  BDM_PROTECTED_MEMBER(Container<real_v>, unused_) = {real_v(6.28)};
};

template <typename Cell>
void CoreOp(Cell* cell) {
  std::cout << "[CoreOp] cell z-position: " << cell->GetPosition()[2]
            << std::endl;
}

// -----------------------------------------------------------------------------
// libraries for specific specialities add functionality - e.g. Neuroscience
class Neurite {};

// add Neurites to BaseCell
template <typename Base = BaseCell<>>
class Neuron : public Base {
  BDM_CLASS_HEADER(Neuron, Neuron<>,
                   Neuron<typename Base::template Self<Backend>>, neurites_);

 public:
  template <class... A>
  explicit Neuron(const SimdArray<std::vector<Neurite>>& neurites,
                  const A&... a)
      : Base(a...) {
    neurites_[idx_] = neurites;
  }

  Neuron() = default;
  const SimdArray<std::vector<Neurite>>& GetNeurites() const {
    return neurites_[idx_];
  }

 private:
  BDM_PRIVATE_MEMBER(Container<SimdArray<std::vector<Neurite>>>, neurites_);
};

// define easy to use templated type alias
BDM_DEFAULT_TEMPLATE(MemberSelector, Backend)
using BdmNeuron = Neuron<BaseCell<BdmSimObject<MemberSelector, Backend>>>;

// -----------------------------------------------------------------------------
// code written by life scientists using package core and Neuroscience extension
// extend Neuron definition provided by extension
template <typename Base>
class NeuronExtension : public Base {
  BDM_CLASS_HEADER(NeuronExtension, NeuronExtension<PlaceholderType>,
                   NeuronExtension<typename Base::template Self<Backend>>,
                   foo_);

 public:
  template <class... A>
  explicit NeuronExtension(const real_v& foo, const A&... a)
      : Base(a...), foo_{foo} {}

  NeuronExtension() = default;

  const real_v& GetFoo() const { return foo_[idx_]; }

  void SetFoo(const real_v& foo) { foo_[idx_] = foo; }

 private:
  BDM_PRIVATE_MEMBER(Container<real_v COMMA() Vc::Allocator<real_v>>,
                     foo_) = {real_v(3.14)};
};

// define easy to use templated type alias
BDM_DEFAULT_TEMPLATE(MemberSelector, Backend)
using MyExtendedNeuron =
    NeuronExtension<Neuron<BaseCell<BdmSimObject<MemberSelector, Backend>>>>;

// define some client code that processes extended neurons
template <typename Cell>
void CustomOp(const Cell& cell) {
  std::cout << "[CustomOp] cell #neurites " << cell->GetNeurites().size()
            << std::endl
            << "           cell.foo_      " << cell->GetFoo() << std::endl;
}

// define member selectors to remove data members that won't be used in the
// simulation
NEW_MEMBER_REMOVER(RemoveUnused, BaseCell<>, unused_);
NEW_MEMBER_SELECTOR(FooSelector, NeuronExtension<PlaceholderType>, foo_);

void TestDataMemberSelectors() {
  // --------------------------
  // use only classes from core
  BaseCell<> base;
  CoreOp(&base);

  // -------------------------------------
  // use class from neuroscience extension
  Neuron<> neuron;
  CoreOp(&neuron);

  BdmNeuron<> neuron1;
  // following statement is equivalent to:
  // Neuron<BaseCell<BdmSimObject<RemoveUnused> > >
  BdmNeuron<RemoveUnused> neuron_wo_unused;

  std::cout << "sizeof(neuron1)          " << sizeof(neuron1) << std::endl
            << "sizeof(neuron_wo_unused) " << sizeof(neuron_wo_unused)
            << std::endl;

  // ---------------------
  // use customized neuron
  NeuronExtension<Neuron<>> extended_neuron;
  CoreOp(&extended_neuron);
  CustomOp(&extended_neuron);

  // equivalent but with easier interface
  MyExtendedNeuron<> extended_neuron1(
      MyExtendedNeuron<>::real_v(1.2),
      MyExtendedNeuron<>::SimdArray<std::vector<Neurite>>{},
      std::array<MyExtendedNeuron<>::real_v, 3>{1, 2, 3});
  CoreOp(&extended_neuron1);
  CustomOp(&extended_neuron1);

  // easier customizeble interface to
  MyExtendedNeuron<RemoveUnused> extended_neuron_wo_unused;
  MyExtendedNeuron<FooSelector> extended_neuron_only_foo;
  std::cout << "sizeof(extended_neuron1)          " << sizeof(extended_neuron1)
            << std::endl
            << "sizeof(extended_neuron_wo_unused) "
            << sizeof(extended_neuron_wo_unused) << std::endl
            << "sizeof(extended_neuron_only_foo)  "
            << sizeof(extended_neuron_only_foo) << std::endl;
}

void TestDifferentBackends() {
  MyExtendedNeuron<SelectAllMembers, VcBackend> vc_simd_neuron;
  std::cout << "simd   foo " << vc_simd_neuron.GetFoo() << std::endl;
  VcBackend::real_v foo;
  foo[0] = 1.1;
  foo[1] = 2.2;
  vc_simd_neuron.SetFoo(foo);
  MyExtendedNeuron<SelectAllMembers, VcBackend> vc_simd_neuron_2 =
      vc_simd_neuron;
  std::cout << "simd   foo " << vc_simd_neuron.GetFoo() << std::endl;

  MyExtendedNeuron<SelectAllMembers, VcSoaBackend> vc_soa_neuron;
  vc_soa_neuron.clear();
  // alternative to the last two lines:
  auto vc_soa_neuron_1 =
      MyExtendedNeuron<SelectAllMembers, VcSoaBackend>::NewEmptySoa();
  // vc_soa_neuron[0] = vc_simd_neuron; // FIXME

  vc_soa_neuron.push_back(vc_simd_neuron);
  std::cout << "soa[0] foo " << vc_soa_neuron[0].GetFoo() << std::endl;

  auto vc_soa_ref_neuron = vc_soa_neuron.GetSoaRef();
  foo[0] = 3.3;
  foo[1] = 4.4;
  vc_soa_ref_neuron[0].SetFoo(foo);
  std::cout << "soa[0] foo " << vc_soa_neuron[0].GetFoo() << std::endl;
}

int main() {
  TestDataMemberSelectors();
  TestDifferentBackends();

  return 0;
}
