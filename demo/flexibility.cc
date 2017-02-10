// This example demonstrates how classes can be extended / modified using
// mixins and templates
// Contruction problem is solved using variadic templates and perfect forwarding

#include <array>
#include <iostream>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>
#include <stdexcept>

#include "cpp_magic.h"
#include "timing.h"

using std::ostream;

/// default data member selector which does not remove any data members
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

  template <typename T>
  Nulltype(std::initializer_list<T>) {}

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

#define BDM_DEFAULT_TEMPLATE(template_param_name)                          \
  template <template <typename, typename, int> class template_param_name = \
                SelectAllMembers>

/// Macro to define data member for a simulation object
/// Hides complexity needed to conditionally remove the data member
#define BDM_DATA_MEMBER(access_modifier, type_name, var_name)               \
 public:                                                                    \
  static constexpr int getDataMemberUid##var_name() { return __COUNTER__; } \
  access_modifier:                                                          \
  typename TMemberSelector<type_name, self,                                 \
                           getDataMemberUid##var_name()>::type var_name

#define BDM_PUBLIC_MEMBER(type_name, var_name) \
  BDM_DATA_MEMBER(public, REMOVE_TRAILING_COMMAS(type_name), var_name)

#define BDM_PROTECTED_MEMBER(type_name, var_name) \
  BDM_DATA_MEMBER(protected, REMOVE_TRAILING_COMMAS(type_name), var_name)

#define BDM_PRIVATE_MEMBER(type_name, var_name) \
  BDM_DATA_MEMBER(private, REMOVE_TRAILING_COMMAS(type_name), var_name)

/// Macro to insert required boilerplate code into classes
/// @param: self_specifier: used to point to static members / functions of this
///         class - use PlaceholderType for template parameters without default
///         parameter - e.g.
///         `class A {};`
///           -> self_specifier: A
///         `template<typename T=DefaultValue> class B {};`
///           -> self_specifier: B<>
///         `template<typename T, typename U> class C {};`
///           -> self_specifier: C<PlaceholderType COMMA() PlaceholderType>
#define BDM_CLASS_HEADER(self_specifier)                                 \
 public:                                                                 \
  using self = self_specifier;                                           \
  template <typename Type, typename EnclosingClass, int id>              \
  using TMemberSelector =                                                \
      typename Base::template TMemberSelector<Type, EnclosingClass, id>; \
                                                                         \
 private:

// -----------------------------------------------------------------------------
// core library
BDM_DEFAULT_TEMPLATE(TTMemberSelector)
struct BdmSimObject {
  template <typename Type, typename EnclosingClass, int id>
  using TMemberSelector = TTMemberSelector<Type, EnclosingClass, id>;
};

/// This struct is used to access static functions / members of a class that
/// has template parameter(s) without a default value.
/// e.g.
/// ```
/// template <typename T> class Foo { static const int kBar = 3; };
/// Foo<PlaceholderType>::kBar
/// ```
/// The definition of the static function / member must be invariant of the
/// template parameter(s)
/// e.g. `Foo<PlaceholderType>::kBar == Foo<AnyOtherType>::kBar`
/// Usage solely to create a valid id for the scope operator. It is not allowed
/// to instantiate an object with template parameter PlaceholderType. The
/// following statement is invalid and will throw an exception at runtime:
/// `Foo<PlaceholderType> foo;`
struct PlaceholderType : public BdmSimObject<> {
  PlaceholderType() {
    throw std::logic_error(
        "Creating an instance of type PlaceholderType is not allowed. "
        "PlaceholderType should solely be used for creating a valid id "
        "for the scope operator");
  }
  template <class... A>
  PlaceholderType(const A&... a)
      : PlaceholderType() {}
};

template <typename Base = BdmSimObject<> >
class BaseCell : public Base {
  BDM_CLASS_HEADER(BaseCell<>);

 public:
  explicit BaseCell(const std::array<double, 3>& pos) : position_(pos) {}
  BaseCell() : position_{0, 0, 0} {}
  const std::array<double, 3>& GetPosition() const { return position_; }

 protected:
  BDM_PROTECTED_MEMBER(std::array<double COMMA() 3>, position_);
  BDM_PROTECTED_MEMBER(double, unused_) = 6.28;
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
template <typename Base = BaseCell<> >
class Neuron : public Base {
  BDM_CLASS_HEADER(Neuron<>);

 public:
  template <class... A>
  explicit Neuron(const std::vector<Neurite>& neurites, const A&... a)
      : Base(a...), neurites_{neurites} {}
  Neuron() = default;
  const std::vector<Neurite>& GetNeurites() const { return neurites_; }

 private:
  BDM_PRIVATE_MEMBER(std::vector<Neurite>, neurites_);
};

// define easy to use templated type alias
BDM_DEFAULT_TEMPLATE(MemberSelector)
using BdmNeuron = Neuron<BaseCell<BdmSimObject<MemberSelector> > >;

// -----------------------------------------------------------------------------
// code written by life scientists using package core and Neuroscience extension
// extend Neuron definition provided by extension
template <typename Base>
class NeuronExtension : public Base {
  BDM_CLASS_HEADER(NeuronExtension<PlaceholderType>);

 public:
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

// define easy to use templated type alias
BDM_DEFAULT_TEMPLATE(MemberSelector)
using MyExtendedNeuron =
    NeuronExtension<Neuron<BaseCell<BdmSimObject<MemberSelector> > > >;

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

int main() {
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
  NeuronExtension<Neuron<> > extended_neuron;
  CoreOp(&extended_neuron);
  CustomOp(&extended_neuron);

  // equivalent but with easier interface
  MyExtendedNeuron<> extended_neuron1(1.2, std::vector<Neurite>{Neurite()},
                                      std::array<double, 3>{1, 2, 3});
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

  return 0;
}
