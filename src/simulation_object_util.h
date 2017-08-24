#ifndef SIMULATION_OBJECT_UTIL_H_
#define SIMULATION_OBJECT_UTIL_H_

#include <algorithm>
#include <exception>
#include <memory>
#include <type_traits>
#include <vector>
#include "backend.h"
#include "macros.h"
#include "resource_manager.h"
#include "root_util.h"
#include "type_util.h"

namespace bdm {

using std::enable_if;
using std::is_same;

// -----------------------------------------------------------------------------
// Helper macros used to generate code for all data members of a class

#define BDM_CLASS_HEADER_PUSH_BACK_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_PUSH_BACK_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_PUSH_BACK_BODY_ITERATOR(data_member) \
  data_member.push_back(other.data_member[0]);

#define BDM_CLASS_HEADER_POP_BACK_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_POP_BACK_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_POP_BACK_BODY_ITERATOR(data_member) \
  data_member.pop_back();

#define BDM_CLASS_HEADER_SWAP_AND_POP_BACK_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_SWAP_AND_POP_BACK_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_SWAP_AND_POP_BACK_BODY_ITERATOR(data_member) \
  std::swap(data_member[index], data_member[size - 1]);               \
  data_member.pop_back();

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
  data_member[kIdx] = rhs.data_member[0];

/// Macro to insert required boilerplate code into simulation object
/// @param  class_name class name witout template specifier e.g. \n
///         `class Foo {};` \n
///          -> class_name: `Foo` \n
///         `template <typename T> class Foo {};` \n
///          -> class_name: `Foo` \n
/// @param   class_version_id required for ROOT I/O (see ROOT ClassDef Macro).
///          Every time the layout of the class is changed, class_version_id
///          must be incremented by one. The class_version_id should be greater
///          or equal to 1.
/// @param   self_specifier Used internally to create the same object, but with
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
/// @param   friend_template_signature used to "friend" all template versions of
///          this class. Unfortunately, there is no generic definition for
///          different numbers of template arguments or types. Therefore, the
///          class template signature has to be repeated as a parameter to this
///          macro. Example: \n
///          1) `template <typename T> class Foo {...};`
///             -> friend_template_signature: `template <typename>` \n
///          2) `template <typename T, typename U> class Bar {...};`
///             -> friend_template_signature:
///             `template <typename COMMA() typename>`
/// @param  ...: List of all data members of this class
#define BDM_CLASS_HEADER_ADV(class_name, class_version_id, self_specifier,     \
                             friend_template_signature, ...)                   \
 public:                                                                       \
  /* reduce verbosity of some types and variables by defining a local alias */ \
  using Base::kIdx;                                                            \
                                                                               \
  using Backend = typename Base::Backend;                                      \
                                                                               \
  template <typename T>                                                        \
  using vec = typename Backend::template vec<T>;                               \
                                                                               \
  /** Used internally to create the same object, but with */                   \
  /** different backend - required since inheritance chain is not known */     \
  /** inside a mixin. */                                                       \
  template <typename TTBackend = Backend>                                      \
  using Self = self_specifier;                                                 \
                                                                               \
  /** all template versions of this class are friends of each other */         \
  /** so they can access each others data members */                           \
  friend_template_signature friend class class_name;                           \
                                                                               \
  explicit class_name(TRootIOCtor* io_ctor) {}                                 \
                                                                               \
  /** Create new empty object with SOA memory layout. */                       \
  /** Calling Self<Soa> soa; will have already one instance inside -- */       \
  /** the one with default parameters. */                                      \
  /** Therefore that one has to be removed. */                                 \
  static Self<Soa> NewEmptySoa(std::size_t reserve_capacity = 0) {             \
    Self<Soa> ret_value;                                                       \
    ret_value.clear();                                                         \
    if (reserve_capacity != 0) {                                               \
      ret_value.reserve(reserve_capacity);                                     \
    }                                                                          \
    return ret_value;                                                          \
  }                                                                            \
                                                                               \
  /** Constructor to create SOA reference object */                            \
  template <typename T>                                                        \
  class_name(T* other, size_t idx)                                             \
      : Base(other, idx),                                                      \
        REMOVE_TRAILING_COMMAS(BDM_CLASS_HEADER_CPY_CTOR_INIT(__VA_ARGS__)) {} \
                                                                               \
  /** Equivalent to std::vector<> clear - it removes all elements from */      \
  /** all data members */                                                      \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type clear() {                         \
    std::lock_guard<std::recursive_mutex> lock(Base::mutex_);                  \
    Base::clear();                                                             \
    BDM_CLASS_HEADER_CLEAR_BODY(__VA_ARGS__)                                   \
  }                                                                            \
                                                                               \
  /** Equivalent to std::vector<> reserve - it increases the capacity */       \
  /** of all data member containers */                                         \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type reserve(                          \
      std::size_t new_capacity) {                                              \
    std::lock_guard<std::recursive_mutex> lock(Base::mutex_);                  \
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
  template <typename T = Backend>                                              \
  typename enable_if<is_same<T, SoaRef>::value, Self<SoaRef>&>::type           \
  operator=(const Self<Scalar>& rhs) {                                         \
    BDM_CLASS_HEADER_ASSIGNMENT_OP_BODY(__VA_ARGS__)                           \
    Base::operator=(rhs);                                                      \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  /** This method commits changes made by `DelayedPushBack` and */             \
  /** `DelayedRemove`. */                                                      \
  /** CAUTION: \n*/                                                            \
  /**   * Commit invalidates pointers and references returned by*/             \
  /**     `DelayedPushBack`. \n*/                                              \
  /**   * If memory reallocations are required all pointers or references*/    \
  /**     into this container are invalidated\n*/                              \
  /** One removal has constant complexity. If the element which should be*/    \
  /** removed is not the last element it is swapped with the last one.*/       \
  /** In the next step it can be removed in constant time using pop_back.*/    \
  /** CAUTION: Swapping elements invalidates pointers to this element.*/       \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type Commit() {                        \
    std::lock_guard<std::recursive_mutex> lock(Base::mutex_);                  \
    /* commit delayed push backs */                                            \
    for (auto& element : to_be_added_) {                                       \
      PushBackImpl(element);                                                   \
    }                                                                          \
    to_be_added_.clear();                                                      \
    /* commit delayed removes */                                               \
    for (size_t idx : Base::to_be_removed_) {                                  \
      if (Base::size() > 1) {                                                  \
        SwapAndPopBack(idx, Base::size());                                     \
      } else {                                                                 \
        PopBack(idx, Base::size());                                            \
      }                                                                        \
    }                                                                          \
    Base::to_be_removed_.clear();                                              \
  }                                                                            \
                                                                               \
  /** Safe method to add an element to this vector. */                         \
  /** Does not invalidate, iterators, pointers or references. */               \
  /** Changes do not take effect until they are commited.*/                    \
  /** @param element that should be added to the vector*/                      \
  /** @return reference to the added element inside the temporary vector*/     \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value, Self<Scalar>&>::type DelayedPushBack(   \
      const Self<Scalar>& element) {                                           \
    std::lock_guard<std::recursive_mutex> lock(Base::mutex_);                  \
    to_be_added_.push_back(element);                                           \
    return to_be_added_[to_be_added_.size() - 1];                              \
  }                                                                            \
                                                                               \
 protected:                                                                    \
  /** Equivalent to std::vector<> push_back - it adds the scalar values to */  \
  /** all data members */                                                      \
  void PushBackImpl(const SimulationObject<Scalar>& o) override {              \
    auto other = *static_cast<const Self<Scalar>*>(&o);                        \
    BDM_CLASS_HEADER_PUSH_BACK_BODY(__VA_ARGS__);                              \
    Base::PushBackImpl(o);                                                     \
  }                                                                            \
                                                                               \
  /** Swap element with last element and remove last element from each */      \
  /** data member */                                                           \
  void SwapAndPopBack(size_t index, size_t size) override {                    \
    BDM_CLASS_HEADER_SWAP_AND_POP_BACK_BODY(__VA_ARGS__);                      \
    Base::SwapAndPopBack(index, size);                                         \
  }                                                                            \
                                                                               \
  /** Remove last element from each data member */                             \
  void PopBack(size_t index, size_t size) override {                           \
    BDM_CLASS_HEADER_POP_BACK_BODY(__VA_ARGS__);                               \
    Base::PopBack(index, size);                                                \
  }                                                                            \
                                                                               \
 private:                                                                      \
  /** Elements that are added using `DelayedPushBack` and not yet commited */  \
  typename type_ternary_operator<                                              \
      is_same<Backend, Soa>::value, std::vector<Self<Scalar>>,                 \
      VectorPlaceholder<Self<Scalar>>>::type to_be_added_;                     \
                                                                               \
  BDM_ROOT_CLASS_DEF_OVERRIDE(class_name, class_version_id)

/// simpflified interface for standard simulation object with one template
/// parameter named Base.
/// Documentation see BDM_CLASS_HEADER_ADV
#define BDM_CLASS_HEADER(class_name, class_version_id, ...)                 \
  BDM_CLASS_HEADER_ADV(class_name, class_version_id,                        \
                       class_name<typename Base::template Self<TTBackend>>, \
                       template <typename>, __VA_ARGS__)

/// Helper function to make cell division easier for the programmer.
/// Creates a new daughter object and passes it together with the given
/// parameters to the divide method in `T`. Afterwards the daughter is added
/// to the given container and a reference returned to the caller.
/// Uses `DelayedPushBack` - that means that this change must be commited
/// before it is visible in the container. @see TransactionalVector, CellExt
/// @param progenitor mother cell which gets divided
/// @param container where the new daughter cell should be added
/// @param parameters list of parameters that get forwarded to the right
///        implementation in `T`
/// @return "reference" to the new daughter in the temporary container is
///         returned. This reference will become invalid once `Commit()`
///         method of the container is called.
template <typename T, typename Container, typename... Params>
typename std::remove_reference<T>::type::template Self<Scalar>& Divide(
    T&& progenitor, Container* container, Params... parameters) {
  // daughter type is scalar version of T
  using DaughterType =
      typename std::remove_reference<T>::type::template Self<Scalar>;
  DaughterType daughter;
  progenitor.Divide(&daughter, parameters...);
  return container->DelayedPushBack(daughter);
}

/// Overloaded function to use ResourceManager to omit parameter container.
/// Container is obtained from the ResourceManager
template <typename T, typename... Params,
          typename TResourceManager = ResourceManager<>>
typename std::remove_reference<T>::type::template Self<Scalar>& Divide(
    T&& progenitor, Params... parameters) {
  // daughter type is scalar version of T
  using DaughterType =
      typename std::remove_reference<T>::type::template Self<Scalar>;
  auto container = TResourceManager::Get()->template Get<DaughterType>();
  return Divide(progenitor, container, parameters...);
}

/// Helper function to make cell death easier for the programmer.
/// Uses `DelayedRemove` - that means that this change must be commited
/// before it is visible in the container. @see TransactionalVector
/// Also added to offer consistent API together with Divide
/// @param container container from which the element should be removed
/// @param index specifies the element which gets removed
template <typename Container>
void Delete(Container* container, size_t index) {
  container->DelayedRemove(index);
}

}  // namespace bdm

#endif  // SIMULATION_OBJECT_UTIL_H_
