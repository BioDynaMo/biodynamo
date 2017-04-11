#ifndef SIMULATION_OBJECT_UTIL_H_
#define SIMULATION_OBJECT_UTIL_H_

#include <exception>
#include <memory>
#include "backend.h"
#include "macros.h"

/// Macro to make default template definition for classes, structs and
/// using statement seasier. Defines two template parameters: A data member
/// selector and a backend. Names of template parameters are specified using
/// the corresponding parameter. Uses default parameter SelectAllMembers and
/// VcVectorBackend.
/// @param selector_name: template paramter name of the data member selector
/// @param backend_name: template parameter name for the backend
#define BDM_DEFAULT_TEMPLATE(selector_name, backend_name)            \
  template <template <typename, typename, int> class selector_name = \
                SelectAllMembers,                                    \
            typename backend_name = VcVectorBackend>

/// Macro to define data member for a simulation object
/// Hides complexity needed to conditionally remove the data member
#define BDM_DATA_MEMBER(access_modifier, type_name, var_name) \
 public:                                                      \
  static const int kDataMemberUid##var_name = __COUNTER__;    \
  access_modifier:                                            \
  typename Base::template MemberSelector<                     \
      type_name, SelfUnique, kDataMemberUid##var_name>::type var_name

#define BDM_PUBLIC_MEMBER(type_name, var_name) \
  BDM_DATA_MEMBER(public, REMOVE_TRAILING_COMMAS(type_name), var_name)

#define BDM_PROTECTED_MEMBER(type_name, var_name) \
  BDM_DATA_MEMBER(protected, REMOVE_TRAILING_COMMAS(type_name), var_name)

#define BDM_PRIVATE_MEMBER(type_name, var_name) \
  BDM_DATA_MEMBER(private, REMOVE_TRAILING_COMMAS(type_name), var_name)

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

#define BDM_CLASS_HEADER_ASSIGNMENT_OP_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_ASSIGNMENT_OP_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_ASSIGNMENT_OP_BODY_ITERATOR(data_member) \
  data_member = other.data_member;

#define BDM_CLASS_HEADER_SOA_PUSH_BACK_OP_IF_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_SOA_PUSH_BACK_OP_IF_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_SOA_PUSH_BACK_OP_IF_BODY_ITERATOR(data_member) \
  {                                                                     \
    typename decltype(data_member)::value_type tmp;                     \
    Base::CopyUtil(&tmp, 0, other.data_member, 0, 0);                   \
    data_member.push_back(tmp);                                         \
  }

#define BDM_CLASS_HEADER_SOA_PUSH_BACK_OP_ELSE_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_SOA_PUSH_BACK_OP_ELSE_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_SOA_PUSH_BACK_OP_ELSE_BODY_ITERATOR(data_member) \
  Base::CopyUtil(&data_member, Base::size_ - 1, Base::size_last_vector_,  \
                 other.data_member, 0, 0);

#define BDM_CLASS_HEADER_VECTOR_PUSH_BACK_OP_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_VECTOR_PUSH_BACK_OP_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_VECTOR_PUSH_BACK_OP_BODY_ITERATOR(data_member) \
  Base::CopyUtil(&data_member, 0, Base::size_, other.data_member, 0, 0);

#define BDM_CLASS_HEADER_COPYTO_OP_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_COPYTO_OP_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_COPYTO_OP_BODY_ITERATOR(data_member)             \
  Base::CopyUtil(&dest->data_member, 0, dest_idx, data_member, src_v_idx, \
                 src_idx);

#define BDM_CLASS_HEADER_COPYFROM_OP_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_COPYFROM_OP_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_COPYFROM_OP_BODY_ITERATOR(data_member)           \
  Base::CopyUtil(&data_member, 0u, dest_idx, src->data_member, src_v_idx, \
                 src_idx);

/// Macro to insert required boilerplate code into simulation object
/// @param  class_name: class name witout template specifier e.g. \n
///         `class Foo {};` \n
///          -> class_name: `Foo` \n
///         `template <typename T> class Foo {};` \n
///          -> class_name: `Foo` \n
/// @param self_unique_specifier: used to point to static members / functions
///         of this class - use PlaceholderType for template parameters without
///         default parameter - e.g. \n
///         `class A {};` \n
///           -> self_specifier: A \n
///         `template<typename T=DefaultValue> class B {};` \n
///           -> self_specifier: B<> \n
///         `template<typename T, typename U> class C {};` \n
///           -> self_specifier: C<PlaceholderType COMMA() PlaceholderType>
/// @param   self_specifier: Used internally to create the same object, but with
///          different backend or MemberSelector - required since inheritance
///          chain
///          is not known inside a mixin. \n
///          Value: Type Id, but template parameter Base must be replaced with:
///          `typename Base::template Self<TTBackend COMMA() TTMemberSelector>`
///          \n\n
///          Example: original class: \n
///          `template<class Base, class Neurite> class Neuron : public Base
///          {};` \n
///          Type Id: `Neuron<Base, Neurite>`
///          replace Base:
///          `Neuron<typename Base::template Self<TTBackend COMMA()
///          TTMemberSelector>, Neurite>` \n\n
///          "," are not allowed as part of preprocessor parameter -> replace
///          with COMMA() \n
///           -> self_specifier: `Neuron<typename Base::template Self<TTBackend
///           COMMA() TTMemberSelector>
///           COMMA() Neurite>`
/// @param  ...: List of all data members of this class
#define BDM_CLASS_HEADER_ADV(class_name, self_unique_specifier,                \
                             self_specifier, ...)                              \
 public:                                                                       \
  /* reduce verbosity of some types and variables by defining a local alias */ \
  using Base::idx_;                                                            \
                                                                               \
  using Backend = typename Base::Backend;                                      \
  using real_v = typename Backend::real_v;                                     \
  using real_t = typename Backend::real_t;                                     \
                                                                               \
  template <typename T>                                                        \
  using SimdArray = typename Backend::template SimdArray<T>;                   \
                                                                               \
  template <typename T>                                                        \
  using Container = typename Backend::template Container<T>;                   \
                                                                               \
  /* Used to point to static members / functions in a unique way */            \
  using SelfUnique = self_unique_specifier;                                    \
                                                                               \
  /* Used internally to create the same object, but with */                    \
  /* different backend - required since inheritance chain is not known */      \
  /* inside a mixin. */                                                        \
  template <typename TTBackend = Backend, template <typename, typename, int>   \
                                          class TTMemberSelector =             \
                                              Base::template MemberSelector>   \
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
  /* Returns a new reference object to `this`  */                              \
  /* The main application is thread safety.  */                                \
  /* By default SOA container are not thread safe. They all share the same  */ \
  /* `idx_` data member. The new reference object has its own `idx_`  */       \
  std::unique_ptr<Self<VcSoaRefBackend>> GetSoaRef() {                         \
    return std::unique_ptr<Self<VcSoaRefBackend>>(                             \
        new Self<VcSoaRefBackend>(this));                                      \
  }                                                                            \
                                                                               \
  /* const version of GetSoaRef */                                             \
  Vc_ALWAYS_INLINE Self<VcSoaRefBackend> GetSoaRef() const {                   \
    throw std::logic_error("Function not implemented yet");                    \
  }                                                                            \
                                                                               \
  /* Append scalar on a vector simulation object */                            \
  template <typename T = Backend>                                              \
  typename enable_if<is_same<T, VcVectorBackend>::value>::type push_back(      \
      const Self<ScalarBackend>& other) {                                      \
    BDM_CLASS_HEADER_VECTOR_PUSH_BACK_OP_BODY(__VA_ARGS__);                    \
    Base::push_back(other);                                                    \
  }                                                                            \
                                                                               \
  /* Append scalar on a soa simulation object */                               \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type push_back(                        \
      const Self<ScalarBackend>& other) {                                      \
    if (Base::Elements() == 0 || Base::is_full()) {                            \
      BDM_CLASS_HEADER_SOA_PUSH_BACK_OP_IF_BODY(__VA_ARGS__);                  \
    } else {                                                                   \
      BDM_CLASS_HEADER_SOA_PUSH_BACK_OP_ELSE_BODY(__VA_ARGS__);                \
    }                                                                          \
    Base::push_back(other);                                                    \
  }                                                                            \
                                                                               \
  /* Append vector on a soa simulation object */                               \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type push_back(                        \
      const Self<VcVectorBackend>& other) {                                    \
    Base::push_back(other);                                                    \
    BDM_CLASS_HEADER_PUSH_BACK_BODY(__VA_ARGS__);                              \
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
  /* This operator is not thread safe! All threads modify the same index. */   \
  /* For parallel execution create a reference object for each thread -- */    \
  /* see GetSoaRef */                                                          \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value, Self<Backend>&>::type& operator[](      \
      int index) {                                                             \
    idx_ = index;                                                              \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value, const Self<Backend>>::type& operator[]( \
      int index) const {                                                       \
    idx_ = index;                                                              \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  /* Assigment operator if two objects are of the exact same type */           \
  Self<Backend>& operator=(const Self<Backend>& other) const {                 \
    Base::operator=(other);                                                    \
    BDM_CLASS_HEADER_ASSIGNMENT_OP_BODY(__VA_ARGS__)                           \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  void CopyTo(std::size_t src_v_idx, std::size_t src_idx,                      \
              std::size_t dest_v_idx, std::size_t dest_idx, T* dest) const {   \
    BDM_CLASS_HEADER_COPYTO_OP_BODY(__VA_ARGS__);                              \
    Base::CopyTo(src_v_idx, src_idx, dest_v_idx, dest_idx, dest);              \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  void CopyFrom(const T& source, std::size_t src_v_idx, std::size_t src_idx,   \
                std::size_t dest_v_idx, std::size_t dest_idx) {                \
    auto src =                                                                 \
        static_cast<const Self<VcSoaBackend, T::template MemberSelector>*>(    \
            &source);                                                          \
    BDM_CLASS_HEADER_COPYFROM_OP_BODY(__VA_ARGS__);                            \
    Base::CopyFrom(source, src_v_idx, src_idx, dest_v_idx, dest_idx);          \
  }                                                                            \
                                                                               \
 protected:                                                                    \
  /* Constructor to create SoaRefBackend */                                    \
  template <typename T = Backend>                                              \
  class_name(Self<VcSoaBackend>* other)                                        \
      : Base(other),                                                           \
        REMOVE_TRAILING_COMMAS(BDM_CLASS_HEADER_CPY_CTOR_INIT(__VA_ARGS__)) {} \
                                                                               \
 private:

/// simpflified interface for standard simulation object with one template
/// parameter named
/// Base
/// documentation see BDM_CLASS_HEADER_ADV
#define BDM_CLASS_HEADER(class_name, ...)                                    \
  BDM_CLASS_HEADER_ADV(                                                      \
      class_name, class_name<>,                                              \
      class_name<                                                            \
          typename Base::template Self<TTBackend COMMA() TTMemberSelector>>, \
      __VA_ARGS__)

#endif  // SIMULATION_OBJECT_UTIL_H_
