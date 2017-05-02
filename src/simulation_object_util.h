#ifndef SIMULATION_OBJECT_UTIL_H_
#define SIMULATION_OBJECT_UTIL_H_

#include <exception>
#include <memory>
#include <type_traits>
#include "backend.h"
#include "macros.h"
#include "type_util.h"

using std::enable_if;
using std::is_same;

// -----------------------------------------------------------------------------
// Helper macros used to generate code for all data members of a class

#define BDM_CLASS_HEADER_PUSH_BACK_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_PUSH_BACK_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_PUSH_BACK_BODY_ITERATOR(data_member) \
  data_member.push_back(other.data_member[0]);

#define BDM_CLASS_HEADER_CPY_CTOR_INIT(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_CPY_CTOR_INIT_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_CPY_CTOR_INIT_ITERATOR(data_member) \
  data_member(other->data_member),

#define BDM_CLASS_HEADER_CLEAR_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_CLEAR_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_CLEAR_BODY_ITERATOR(data_member) data_member.clear();

#define BDM_CLASS_HEADER_RESERVE_BODY(...) \
  EVAL(LOOP_2_1(BDM_CLASS_HEADER_RESERVE_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_RESERVE_BODY_ITERATOR(new_cap, data_member) \
  data_member.reserve(new_cap);

/// Macro to insert required boilerplate code into simulation object
/// @param  class_name: class name witout template specifier e.g. \n
///         `class Foo {};` \n
///          -> class_name: `Foo` \n
///         `template <typename T> class Foo {};` \n
///          -> class_name: `Foo` \n
/// @param   self_specifier: Used internally to create the same object, but with
///          different backend - required since inheritance chain is not known
///          inside a mixin. \n
///          Value: Type Id, but template parameter Base must be replaced with:
///          `typename Base::template Self<TTBackend>`\n\n
///          Example: original class: \n
///          `template<class Base, class Neurite> class Neuron : public Base
///          {};` \n
///          Type Id: `Neuron<Base, Neurite>`
///          replace Base:
///          `Neuron<typename Base::template Self<TTBackend>, Neurite>` \n\n
///          "," are not allowed as part of preprocessor parameter -> replace
///          with COMMA() \n
///           -> self_specifier:
///          `Neuron<typename Base::template Self<TTBackend> COMMA() Neurite>`
/// @param  ...: List of all data members of this class
#define BDM_CLASS_HEADER_ADV(class_name, self_specifier, ...)                  \
 public:                                                                       \
  /* reduce verbosity of some types and variables by defining a local alias */ \
  using Base::idx_;                                                            \
                                                                               \
  using Backend = typename Base::Backend;                                      \
                                                                               \
  template <typename T>                                                        \
  using vec = typename Backend::template vec<T>;                               \
                                                                               \
  /* Used internally to create the same object, but with */                    \
  /* different backend - required since inheritance chain is not known */      \
  /* inside a mixin. */                                                        \
  template <typename TTBackend = Backend>                                      \
  using Self = self_specifier;                                                 \
                                                                               \
  /* all template versions of this class are friends of each other */          \
  /* so they can access each others data members */                            \
  template <typename T>                                                        \
  friend class class_name;                                                     \
                                                                               \
  /* Equivalent to std::vector<> push_back - it adds the scalar values to */   \
  /* all data members */                                                       \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type push_back(                        \
      const Self<Scalar>& other) {                                             \
    BDM_CLASS_HEADER_PUSH_BACK_BODY(__VA_ARGS__);                              \
    Base::push_back(other);                                                    \
  }                                                                            \
                                                                               \
  /* Equivalent to std::vector<> clear - it removes all elements from */       \
  /* all data members */                                                       \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type clear() {                         \
    Base::clear();                                                             \
    BDM_CLASS_HEADER_CLEAR_BODY(__VA_ARGS__)                                   \
  }                                                                            \
                                                                               \
  /* Equivalent to std::vector<> reserve - it increases the capacity */        \
  /* of all data member containers */                                          \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type reserve(                          \
      std::size_t new_capacity) {                                              \
    Base::reserve(new_capacity);                                               \
    BDM_CLASS_HEADER_RESERVE_BODY(new_capacity, __VA_ARGS__)                   \
  }                                                                            \
                                                                               \
  Self<SoaRef> operator[](size_t idx) { return Self<SoaRef>(this, idx); }      \
                                                                               \
  const Self<SoaRef> operator[](size_t idx) const {                            \
    return Self<SoaRef>(const_cast<Self<Backend>*>(this), idx);                \
  }                                                                            \
                                                                               \
 protected:                                                                    \
  /* Constructor to create reference object */                                 \
  template <typename T>                                                        \
  class_name(T* other, size_t idx)                                             \
      : Base(other, idx),                                                      \
        REMOVE_TRAILING_COMMAS(BDM_CLASS_HEADER_CPY_CTOR_INIT(__VA_ARGS__)) {} \
                                                                               \
 private:

/// simpflified interface for standard simulation object with one template
/// parameter named Base.
/// Documentation see BDM_CLASS_HEADER_ADV
#define BDM_CLASS_HEADER(class_name, ...)                                   \
  BDM_CLASS_HEADER_ADV(class_name,                                          \
                       class_name<typename Base::template Self<TTBackend>>, \
                       __VA_ARGS__)

#endif  // SIMULATION_OBJECT_UTIL_H_
