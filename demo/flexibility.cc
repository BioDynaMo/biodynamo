// This example demonstrates how classes can be extended / modified using
// mixins and templates
// Contruction problem is solved using variadic templates and perfect forwarding

#include <array>
#include <iostream>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include "cpp_magic.h"
#include "timing.h"

using std::ostream;

template <typename Type, typename EnclosingClass, int id>
struct SelectAllMembers {
  typedef Type type;
};

/// Type for removed data members - which can be optimized out by the compiler
struct Nulltype {
  int empty[0] = {};
  Nulltype() {}
  template <typename T>
  Nulltype(T&& d) {}  // NOLINT(runtime/explicit)
  template <typename T>
  Nulltype& operator=(const T& other) {
    return *this;
  }
  friend ostream& operator<<(ostream& out, const Nulltype& value) {
    return out;
  }
};

/// loops over variadic macro arguments and calls the specified operation
/// removes the first three parameters in each iteration, but adds the first one
/// again for the next call
/// e.g. LOOP(OP, a, b, c, d, e) will lead to:
/// OP(a, b, c)
/// OP(a, d, e)
/// For a more detailed explanation see `MAP` macro in `third_party/cpp_magic.h`
// clang-format off
#define LOOP(operation, first, second, third, ...)           \
  operation(first, second, third)                            \
  IF(HAS_ARGS(__VA_ARGS__))(                                 \
    DEFER2(_LOOP)()(operation, first, __VA_ARGS__))
#define _LOOP() LOOP
// clang-format on

/// adds the partial template specialization to select one clazz-member pair
/// only for internal usage - will be called inside LOOP
/// @param name:    selector name
/// @param clazz:   clazz of the data member
/// @param member:  data member name
#define INTERNAL_SELECT_MEMBER(name, clazz, member)             \
  template <typename Type>                                      \
  struct name<Type, clazz, clazz::getDataMemberUid##member()> { \
    typedef Type type;                                          \
  };

/// creates a new selector type
/// it only enables the specified data members if applied to a simulation object
/// others will be removed
#define NEW_MEMBER_SELECTOR(name, ...)                      \
  template <typename Type, typename EnclosingClass, int id> \
  struct name {                                             \
    typedef Nulltype type;                                  \
  };                                                        \
  EVAL(LOOP(INTERNAL_SELECT_MEMBER, name, __VA_ARGS__))

/// adds the partial template specialization to remove one clazz-member pair
/// only for internal usage - will be called inside LOOP
/// @param name:    selector name
/// @param clazz:   clazz of the data member
/// @param member:  data member name
#define INTERNAL_MEMBER_REMOVER(name, clazz, member)            \
  template <typename Type>                                      \
  struct name<Type, clazz, clazz::getDataMemberUid##member()> { \
    typedef Nulltype type;                                      \
  };

/// creates a new selector type
/// it removes the specified data members if applied to a simulation object
/// others will be kept -> inverse of NEW_MEMBER_SELECTOR
#define NEW_MEMBER_REMOVER(name, ...)                       \
  template <typename Type, typename EnclosingClass, int id> \
  struct name {                                             \
    typedef Type type;                                      \
  };                                                        \
  EVAL(LOOP(INTERNAL_MEMBER_REMOVER, name, __VA_ARGS__))

#define BDM_DEFAULT_TEMPLATE                                           \
  template <template <typename, typename, int> class TMemberSelector = \
                SelectAllMembers>

/// Macro to define data member for a simulation object
/// Hides complexity needed to conditionally remove the data member
#define BDM_DATA_MEMBER(access_modifier, type_name, var_name)               \
 public:                                                                    \
  static constexpr int getDataMemberUid##var_name() { return __COUNTER__; } \
 access_modifier:                                                           \
  typename TMemberSelector<type_name, self,                                 \
                           getDataMemberUid##var_name()>::type var_name

#define BDM_PUBLIC_MEMBER(type_name, var_name) \
 BDM_DATA_MEMBER(public, REMOVE_TRAILING_COMMAS(type_name), var_name)

#define BDM_PROTECTED_MEMBER(type_name, var_name) \
 BDM_DATA_MEMBER(protected, REMOVE_TRAILING_COMMAS(type_name), var_name)

#define BDM_PRIVATE_MEMBER(type_name, var_name) \
 BDM_DATA_MEMBER(private, REMOVE_TRAILING_COMMAS(type_name), var_name)

class NullBaseClass {};

// -----------------------------------------------------------------------------
// core library only defines minimal cell
BDM_DEFAULT_TEMPLATE
class BaseCell {
 public:
  using self = BaseCell<>;
  explicit BaseCell(const std::array<double, 3>& pos) : position_(pos) {}
  BaseCell() : position_{0, 0, 0} {}
  const std::array<double, 3>& GetPosition() const { return position_; }

 protected:
  BDM_PROTECTED_MEMBER(std::array<double COMMA() 3>, position_);
  BDM_PROTECTED_MEMBER(double, unused_) = 6.28;
};

template <typename Cell>
void CoreOp(Cell* cell) {
  std::cout << cell->GetPosition()[2] << std::endl;
}

// -----------------------------------------------------------------------------
// libraries for specific specialities add functionality - e.g. Neuroscience
class Neurite {};

// adds Neurites to BaseCell
// typename TNeurite = Neurite,
template <typename Base, template <typename, typename, int>
                         class TMemberSelector = SelectAllMembers>
class Neuron : public Base {
 public:
  using self = Neuron<Base>;
  template <class... A>
  explicit Neuron(const std::vector<Neurite>& neurites, const A&... a)
      : Base(a...), neurites_{neurites} {}
  Neuron() = default;
  const std::vector<Neurite>& GetNeurites() const { return neurites_; }

 private:
  BDM_PRIVATE_MEMBER(std::vector<Neurite>, neurites_);
};

// -----------------------------------------------------------------------------
// code written by life scientists using package core and Neuroscience extension
template <typename Base, template <typename, typename, int>
                         class TMemberSelector = SelectAllMembers>
class NeuronExtension : public Base {
 public:
  using self = NeuronExtension<Base>;
  template <class... A>
  explicit NeuronExtension(double foo, const A&... a)
      : Base(a...), foo_{foo} {}
  NeuronExtension() = default;
  double GetFoo() const {
    volatile auto& foo = Base::position_;
    return foo_;
  }

 private:
  BDM_PRIVATE_MEMBER(double, foo_) = 3.14;
};

template <typename Cell>
void CustomOp(const Cell& cell) {
  std::cout << cell->GetNeurites().size() << " - " << cell->GetFoo()
            << std::endl;
}

NEW_MEMBER_REMOVER(RemoveUnused, BaseCell<>, unused_);

BDM_DEFAULT_TEMPLATE
using MyNeuron = Neuron<BaseCell<TMemberSelector>, TMemberSelector>;

BDM_DEFAULT_TEMPLATE
using MyExtendedNeuron =
    NeuronExtension<Neuron<BaseCell<TMemberSelector>, TMemberSelector>,
                    TMemberSelector>;

int main() {
  BaseCell<> base;
  CoreOp(&base);

  Neuron<BaseCell<> > neuron;
  CoreOp(&base);
  std::cout << neuron.GetNeurites().size() << std::endl;

  MyNeuron<> my_neuron;
  MyNeuron<RemoveUnused> my_neuron_wo_unused;

  std::cout << sizeof(my_neuron) << " - " << sizeof(my_neuron_wo_unused)
            << std::endl;

  NeuronExtension<Neuron<BaseCell<> > > extended_neuron;
  CustomOp(&extended_neuron);

  MyExtendedNeuron<> my_extended_neuron(1.2, std::vector<Neurite>{Neurite()},
                                        std::array<double, 3>{1, 2, 3});
  MyExtendedNeuron<RemoveUnused> my_extended_neuron_wo_unused;
  std::cout << sizeof(my_extended_neuron) << " - "
            << sizeof(my_extended_neuron_wo_unused) << std::endl;
  CoreOp(&my_extended_neuron);
  CustomOp(&my_extended_neuron);

  return 0;
}
