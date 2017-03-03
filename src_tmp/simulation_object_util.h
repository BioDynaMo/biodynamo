#ifndef SIMULATION_OBJECT_UTIL_H_
#define SIMULATION_OBJECT_UTIL_H_

#include <memory>

#include "backend.h"
#include "preprocessor.h"

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

#endif // SIMULATION_OBJECT_UTIL_H_
