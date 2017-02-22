// This example demonstrates how classes can be extended / modified using
// mixins and templates
// Furthermore, it shows how to remove certain data members and how to select
// different vectorization backends. These backends can be used to select
// SOA or AOSOA memory layout without code changes inside the class or client
// side code

#include <array>
#include <iostream>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>
#include <stdexcept>

#include <Vc/Vc>

#include "cpp_magic.h"
#include "timing.h"

using std::ostream;
using std::enable_if;
using std::is_same;

// -----------------------------------------------------------------------------
// infrastructure required for multiform objects

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
/// e.g. LOOP_3_1(OP, a, b, c, d, e) will lead to:
/// OP(a, b, c)
/// OP(a, d, e)
/// For a more detailed explanation see `MAP` macro in `third_party/cpp_magic.h`
// clang-format off
#define LOOP_3_1(operation, first, second, third, ...)       \
  operation(first, second, third)                            \
  IF(HAS_ARGS(__VA_ARGS__))(                                 \
    DEFER2(_LOOP_3_1)()(operation, first, __VA_ARGS__))
#define _LOOP_3_1() LOOP_3_1
// clang-format on

/// adds the partial template specialization to select one clazz-member pair
/// only for internal usage - will be called inside LOOP_3_1
/// @param name:    selector name
/// @param clazz:   clazz of the data member
/// @param member:  data member name
#define INTERNAL_SELECT_MEMBER(name, clazz, member)         \
  template <typename Type>                                  \
  struct name<Type, clazz, clazz::kDataMemberUid##member> { \
    typedef Type type;                                      \
  };

/// creates a new selector type
/// it only enables the specified data members if applied to a simulation object
/// others will be removed
#define NEW_MEMBER_SELECTOR(name, ...)                      \
  template <typename Type, typename EnclosingClass, int id> \
  struct name {                                             \
    typedef Nulltype type;                                  \
  };                                                        \
  EVAL(LOOP_3_1(INTERNAL_SELECT_MEMBER, name, __VA_ARGS__))

/// adds the partial template specialization to remove one clazz-member pair
/// only for internal usage - will be called inside LOOP_3_1
/// @param name:    selector name
/// @param clazz:   clazz of the data member
/// @param member:  data member name
#define INTERNAL_MEMBER_REMOVER(name, clazz, member)        \
  template <typename Type>                                  \
  struct name<Type, clazz, clazz::kDataMemberUid##member> { \
    typedef Nulltype type;                                  \
  };

/// creates a new selector type
/// it removes the specified data members if applied to a simulation object
/// others will be kept -> inverse of NEW_MEMBER_SELECTOR
#define NEW_MEMBER_REMOVER(name, ...)                       \
  template <typename Type, typename EnclosingClass, int id> \
  struct name {                                             \
    typedef Type type;                                      \
  };                                                        \
  EVAL(LOOP_3_1(INTERNAL_MEMBER_REMOVER, name, __VA_ARGS__))

#define BDM_DEFAULT_TEMPLATE(selector_name, backend_name)            \
  template <template <typename, typename, int> class selector_name = \
                SelectAllMembers,                                    \
            typename backend_name = VcBackend>

/// Macro to define data member for a simulation object
/// Hides complexity needed to conditionally remove the data member
#define BDM_DATA_MEMBER(access_modifier, type_name, var_name) \
 public:                                                      \
  static const int kDataMemberUid##var_name = __COUNTER__;    \
  access_modifier:                                            \
  typename TMemberSelector<type_name, SelfUnique,             \
                           kDataMemberUid##var_name>::type var_name

#define BDM_PUBLIC_MEMBER(type_name, var_name) \
  BDM_DATA_MEMBER(public, REMOVE_TRAILING_COMMAS(type_name), var_name)

#define BDM_PROTECTED_MEMBER(type_name, var_name) \
  BDM_DATA_MEMBER(protected, REMOVE_TRAILING_COMMAS(type_name), var_name)

#define BDM_PRIVATE_MEMBER(type_name, var_name) \
  BDM_DATA_MEMBER(private, REMOVE_TRAILING_COMMAS(type_name), var_name)

// -----------------------------------------------------------------------------
// infrastructure required for vector backend solution
template <typename T>
class SoaRefWrapper {
 public:
  SoaRefWrapper(T& data) : data_(data) {}

  // TODO add all operators

  Vc_ALWAYS_INLINE typename T::value_type& operator[](std::size_t index) {
    return data_[index];
  }

  Vc_ALWAYS_INLINE const typename T::value_type& operator[](
      std::size_t index) const {
    return data_[index];
  }

  template <typename U>
  Vc_ALWAYS_INLINE auto operator<=(const U& u) const
      -> decltype(std::declval<typename T::value_type>() <= u) {
    return data_ <= u;
  }

  template <typename U>
  Vc_ALWAYS_INLINE auto operator<(const U& u) const
      -> decltype(std::declval<typename T::value_type>() < u) {
    return data_ < u;
  }

  template <typename U>
  Vc_ALWAYS_INLINE SoaRefWrapper<T>& operator+=(const U& u) {
    data_ += u;
    return *this;
  }

  Vc_ALWAYS_INLINE SoaRefWrapper<T>& operator=(const SoaRefWrapper<T>& other) {
    if (this != &other) {
      data_ = other.data_;
    }
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& out,
                                  const SoaRefWrapper<T>& wrapper) {
    out << wrapper.data_;
    return out;
  }

  typename T::iterator begin() { return data_.begin(); }
  typename T::iterator end() { return data_.end(); }

  typename T::const_iterator begin() const { return data_.cbegin(); }
  typename T::const_iterator end() const { return data_.cend(); }

 private:
  T& data_;
};

/// This class represents an array with exactly one element
/// Needed for AOSOA: Objects will store a single e.g. real_v instead of N
/// instances. However code was written for SOA and expects an array interface
/// which is exposed with this class.
/// Makes it easy for the compiler to optimize out the extra call to operator[]
/// Didn't work with std::array<T, 1>
template <typename T>
class OneElementArray {
 public:
  OneElementArray() : data_() {}
  OneElementArray(const T& data) : data_(data) {}
  OneElementArray(T&& data) : data_(data) {}
  OneElementArray(std::initializer_list<T> list) : data_(*list.begin()) {}

  Vc_ALWAYS_INLINE T& operator[](const size_t idx) { return data_; }

  Vc_ALWAYS_INLINE const T& operator[](const size_t idx) const { return data_; }

  T* begin() { return &data_; }
  T* end() { return &data_ + 1; }

  const T* begin() const { return &data_; }
  const T* end() const { return &data_ + 1; }

 private:
  T data_;
};

template <bool condition, typename T, typename U>
struct type_ternary_operator {};

template <typename T, typename U>
struct type_ternary_operator<true, T, U> {
  typedef T type;
};

template <typename T, typename U>
struct type_ternary_operator<false, T, U> {
  typedef U type;
};

struct VcBackend {
  typedef const std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = Vc::double_v::Size;
  typedef Vc::double_v real_v;
  template <typename T>
  using SimdArray = std::array<T, kVecLen>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container = OneElementArray<T>;
};

struct VcSoaBackend {
  typedef std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = VcBackend::kVecLen;
  typedef VcBackend::real_v real_v;
  template <typename T>
  using SimdArray = typename VcBackend::template SimdArray<T>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container = std::vector<T, Allocator>;
};

struct VcSoaRefBackend {
  typedef std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = VcBackend::kVecLen;
  typedef VcBackend::real_v real_v;
  template <typename T>
  using SimdArray = typename VcSoaBackend::template SimdArray<T>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container =
      SoaRefWrapper<typename VcSoaBackend::template Container<T, Allocator>>;
};

struct ScalarBackend {
  typedef const std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = 1;
  // TODO change to OneElementArray?
  typedef Vc::SimdArray<double, kVecLen> real_v;
  template <typename T>
  using SimdArray = OneElementArray<T>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container = OneElementArray<T>;
};

typename VcBackend::real_v iif(
    const decltype(std::declval<typename VcBackend::real_v>() <
                   std::declval<typename VcBackend::real_v>())& condition,
    const typename VcBackend::real_v& true_value,
    const typename VcBackend::real_v& false_value) {
  return Vc::iif(condition, true_value, false_value);
}

/// loops over variadic macro arguments and calls the specified operation
/// processes one argument in each iteration
/// e.g. LOOP(OP, a, b) will lead to:
/// OP(a)
/// OP(b)
/// For a more detailed explanation see `MAP` macro in `third_party/cpp_magic.h`
// clang-format off
#define LOOP(operation, first, ...)                          \
  operation(first)                                           \
  IF(HAS_ARGS(__VA_ARGS__))(                                 \
    DEFER2(_LOOP)()(operation, __VA_ARGS__))
#define _LOOP() LOOP
// clang-format on

/// loops over variadic macro arguments and calls the specified operation
/// removes the first two parameters in each iteration, but adds the first one
/// again for the next call
/// e.g. LOOP_3_1(OP, a, b, c) will lead to:
/// OP(a, b)
/// OP(a, c)
/// For a more detailed explanation see `MAP` macro in `third_party/cpp_magic.h`
// clang-format off
#define LOOP_2_1(operation, first, second, ...)           \
  operation(first, second)                                \
  IF(HAS_ARGS(__VA_ARGS__))(                              \
    DEFER2(_LOOP_2_1)()(operation, first, __VA_ARGS__))
#define _LOOP_2_1() LOOP_2_1
// clang-format on

#define BDM_CLASS_HEADER_PUSH_BACK_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_PUSH_BACK_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_PUSH_BACK_BODY_ITERATOR(data_member) \
  data_member.push_back(other.data_member[0]);

#define BDM_CLASS_HEADER_CPY_CTOR_INIT(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_CPY_CTOR_INIT_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_CPY_CTOR_INIT_ITERATOR(data_member) \
  data_member(other.data_member),

#define BDM_CLASS_HEADER_CLEAR_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_CLEAR_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_CLEAR_BODY_ITERATOR(data_member) data_member.clear();

#define BDM_CLASS_HEADER_RESERVE_BODY(...) \
  EVAL(LOOP_2_1(BDM_CLASS_HEADER_RESERVE_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_RESERVE_BODY_ITERATOR(new_cap, data_member) \
  data_member.reserve(new_cap);

#define BDM_CLASS_HEADER_ASSIGNMENT_OP_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_ASSIGNMENT_OP_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_ASSIGNMENT_OP_BODY_ITERATOR(data_member) \
  data_member = other.data_member;

/// Macro to insert required boilerplate code into class
/// @param: class_name: class name witout template specifier e.g.
///         class Foo {};
///          -> class_name: Foo
///         template <typename T> class Foo {};
///          -> class_name: Foo
/// @param: self_unique_specifier: used to point to static members / functions
///         of this class - use PlaceholderType for template parameters without
///         default parameter - e.g.
///         `class A {};`
///           -> self_specifier: A
///         `template<typename T=DefaultValue> class B {};`
///           -> self_specifier: B<>
///         `template<typename T, typename U> class C {};`
///           -> self_specifier: C<PlaceholderType COMMA() PlaceholderType>
/// @param   self_specifier: used internally to create the same object, but with
///          different backend - required since inheritance chain is not known
///          inside a mixin.
///          Value: type Id, but template parameter Base must be replaced with:
///          typename Base::template Self<Backend>
///          Example: original class:
///          template<class Base, class Neurite> class Neuron : public Base {};
///          type Id:
///          Neuron<Base, Neurite>
///          replace Base:
///          Neuron<typename Base::template Self<Backend>, Neurite>
///          "," are not allowed as part of preprocessor parameter -> replace
///          with COMMA()
///           -> self_specifier: Neuron<typename Base::template Self<Backend>
///           COMMA() Neurite>
#define BDM_CLASS_HEADER(class_name, self_unique_specifier, self_specifier,    \
                         ...)                                                  \
 public:                                                                       \
  /* reduce verbosity of some types and variables by defining a local alias */ \
  using Base::idx_;                                                            \
                                                                               \
  template <typename Type, typename EnclosingClass, int id>                    \
  using TMemberSelector =                                                      \
      typename Base::template TMemberSelector<Type, EnclosingClass, id>;       \
                                                                               \
  using Backend = typename Base::Backend;                                      \
  using real_v = typename Backend::real_v;                                     \
                                                                               \
  template <typename T>                                                        \
  using SimdArray = typename Backend::template SimdArray<T>;                   \
                                                                               \
  template <typename T, typename Allocator = std::allocator<T>>                \
  using Container = typename Backend::template Container<T, Allocator>;        \
                                                                               \
  using SelfUnique = self_unique_specifier;                                    \
                                                                               \
  template <typename Backend>                                                  \
  using Self = self_specifier;                                                 \
                                                                               \
  /* all template versions of this class are friends of each other */          \
  /* so they can access each others data members */                            \
  template <typename T>                                                        \
  friend class class_name;                                                     \
                                                                               \
  template <class... A>                                                        \
  static Self<ScalarBackend> NewScalar(const A&... a) {                        \
    return Self<ScalarBackend>(a...);                                          \
  }                                                                            \
                                                                               \
  /* Creates new empty object with VcSoaBackend. */                            \
  /* Calling Self<VcSoaBackend> soa; will have already one instance inside */  \
  /* the one with default parameters */                                        \
  /* Therefore that one has to be removed */                                   \
  static Self<VcSoaBackend> NewEmptySoa(std::size_t reserve_capacity = 0) {    \
    Self<VcSoaBackend> ret_value;                                              \
    ret_value.clear();                                                         \
    if (reserve_capacity != 0) {                                               \
      ret_value.reserve(reserve_capacity);                                     \
    }                                                                          \
    return ret_value;                                                          \
  }                                                                            \
                                                                               \
 protected:                                                                    \
  /* Ctor to create SoaRefBackend */                                           \
  /* only compiled if T == VcSoaRefBackend */                                  \
  /* template parameter required for enable_if - otherwise compile error */    \
  template <typename T = Backend>                                              \
  class_name(                                                                  \
      Self<VcSoaBackend>& other,                                               \
      typename enable_if<is_same<T, VcSoaRefBackend>::value>::type* = 0)       \
      : Base(other),                                                           \
        REMOVE_TRAILING_COMMAS(BDM_CLASS_HEADER_CPY_CTOR_INIT(__VA_ARGS__)) {} \
                                                                               \
 public:                                                                       \
  /* TODO only for SoaBackends */                                              \
  /* needed because operator[] is not thread safe - index is shared among  */  \
  /* all threads */                                                            \
  Vc_ALWAYS_INLINE Self<VcSoaRefBackend> GetSoaRef() {                         \
    return Self<VcSoaRefBackend>(*this);                                       \
  }                                                                            \
                                                                               \
  /* only compiled if Backend == Soa(Ref)Backend */                            \
  /* template parameter required for enable_if - otherwise compile error */    \
  template <typename T = Backend>                                              \
  typename enable_if<is_same<T, VcSoaRefBackend>::value ||                     \
                     is_same<T, VcSoaBackend>::value>::type                    \
  push_back(const Self<VcBackend>& other) {                                    \
    Base::push_back(other);                                                    \
    BDM_CLASS_HEADER_PUSH_BACK_BODY(__VA_ARGS__);                              \
  }                                                                            \
                                                                               \
  /* only compiled if Backend == Soa(Ref)Backend */                            \
  /* template parameter required for enable_if - otherwise compile error */    \
  template <typename T = Backend>                                              \
  typename enable_if<is_same<T, ScalarBackend>::value>::type push_back(        \
      const Self<VcBackend>& other) {                                          \
    throw std::runtime_error("TODO implement: see src/cell.h:Append");         \
  }                                                                            \
                                                                               \
  /* This operator is not thread safe! all threads modify the same index. */   \
  /* For parallel execution create a reference object for each thread -- */    \
  /* see GetSoaRef */                                                          \
  /* only compiled if Backend == Soa(Ref)Backend */                            \
  /* no version if Backend == VcBackend that returns a Self<ScalarBackend> */  \
  /* since this would involves copying of elements and would therefore */      \
  /* degrade performance -> it is therefore discouraged */                     \
  template <typename T = Backend>                                              \
  typename enable_if<is_same<T, VcSoaRefBackend>::value ||                     \
                         is_same<T, VcSoaBackend>::value,                      \
                     Self<Backend>&>::type                                     \
  operator[](int index) {                                                      \
    idx_ = index;                                                              \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  /* assigment operator if two objects are of the exact same type */           \
  Self<Backend>& operator=(const Self<Backend>& other) const {                 \
    Base::operator=(other);                                                    \
    BDM_CLASS_HEADER_ASSIGNMENT_OP_BODY(__VA_ARGS__)                           \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  /* only compiled for VcSOA and VcSOARef backends */                          \
  /* equivalent to std::vector<> clear - it removes all all elements from */   \
  /* all data members */                                                       \
  template <typename T = Backend>                                              \
  typename enable_if<is_same<T, VcSoaRefBackend>::value ||                     \
                     is_same<T, VcSoaBackend>::value>::type                    \
  clear() {                                                                    \
    Base::clear();                                                             \
    BDM_CLASS_HEADER_CLEAR_BODY(__VA_ARGS__)                                   \
  }                                                                            \
                                                                               \
  /* only compiled for VcSOA and VcSOARef backends */                          \
  /* equivalent to std::vector<> reserve - it increases the capacity */        \
  /* of all data member containers */                                          \
  template <typename T = Backend>                                              \
  typename enable_if<is_same<T, VcSoaRefBackend>::value ||                     \
                     is_same<T, VcSoaBackend>::value>::type                    \
  reserve(std::size_t new_capacity) {                                          \
    Base::reserve(new_capacity);                                               \
    BDM_CLASS_HEADER_RESERVE_BODY(new_capacity, __VA_ARGS__)                   \
  }                                                                            \
                                                                               \
 private:

// -----------------------------------------------------------------------------
// core library
BDM_DEFAULT_TEMPLATE(TTMemberSelector, TBackend)
struct BdmSimObject {
 protected:
  template <typename Type, typename EnclosingClass, int id>
  using TMemberSelector = TTMemberSelector<Type, EnclosingClass, id>;

  template <typename TTBackend>
  using Self = BdmSimObject<TTMemberSelector, TTBackend>;

  using Backend = TBackend;

  // used to access the SIMD array in a soa container
  // for non Soa Backends index_t will be const so it can be optimized out
  // by the compiler
  typename Backend::index_t idx_ = 0;

  BdmSimObject() {}

  // Ctor to create SoaRefBackend
  // only compiled if T == VcSoaRefBackend
  // template parameter required for enable_if - otherwise compile error
  template <typename T = Backend>
  BdmSimObject(
      Self<VcSoaBackend>& other,
      typename enable_if<is_same<T, VcSoaRefBackend>::value>::type* = 0) {}

  // only compiled if Backend == Soa(Ref)Backend
  // template parameter required for enable_if - otherwise compile error
  template <typename T = Backend>
  typename enable_if<is_same<T, VcSoaRefBackend>::value ||
                     is_same<T, VcSoaBackend>::value>::type
  push_back(const Self<VcBackend>& other) {}

  // only compiled for VcSOA and VcSOARef backends
  // equivalent to std::vector<> clear - it removes all all elements from all
  // data members
  template <typename T = Backend>
  typename enable_if<is_same<T, VcSoaRefBackend>::value ||
                     is_same<T, VcSoaBackend>::value>::type
  clear() {}

  // only compiled for VcSOA and VcSOARef backends
  // equivalent to std::vector<> reserve - it increases the capacity
  // of all data member containers
  template <typename T = Backend>
  typename enable_if<is_same<T, VcSoaRefBackend>::value ||
                     is_same<T, VcSoaBackend>::value>::type
  reserve(std::size_t new_capacity) {}

  // assigment operator if two objects are of the exact same type
  BdmSimObject<TTMemberSelector, TBackend>& operator=(
      const BdmSimObject<TTMemberSelector, TBackend>& other) const {
    return *this;
  }
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
  explicit PlaceholderType(const A&... a)
      : PlaceholderType() {}
};

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
  BDM_PROTECTED_MEMBER(Container<real_v COMMA() Vc::Allocator<real_v>>,
                       unused_) = {real_v(6.28)};
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
