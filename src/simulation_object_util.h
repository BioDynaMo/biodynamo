#ifndef SIMULATION_OBJECT_UTIL_H_
#define SIMULATION_OBJECT_UTIL_H_

#include <algorithm>
#include <cassert>
#include <exception>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "backend.h"
#include "diffusion_grid.h"
#include "macros.h"
#include "resource_manager.h"
#include "root_util.h"
#include "type_util.h"

namespace bdm {

using std::enable_if;
using std::is_same;

/// Type trait to convert a Simulation object into another Backend.\n
/// Using the internal templated type alias `typename T::template Self<Backend>`
/// is not always possible (if the type is still incomplete).\n
/// Preprocessor macro `BDM_SIM_CLASS` generates template specializations
/// @tparam TSoScalar simulation object with scalar backend
/// @tparam TBackend  desired backend
///
///     // Usage:
///     typename ToBackend<Neuron, Soa>::type neurons;
template <typename TSoScalar, typename TBackend>
struct ToBackend;

/// This type trait is need to encapsulate the derived type and pass it into
/// simulation objects. Inside `BDM_SIM_CLASS` a template specialization is
/// created for each simulation object.
/// @tparam TSoExt template template parameter of a simulation object
template <template <typename TCompileTimeParam, typename TDerived,
                    template <typename, typename> class TBase> class TSoExt>
struct Capsule;

/// Macro to define a new simulation object
/// @param sim_object
/// @param base_class
///
///     // Example usage to extend class Cell
///     BDM_SIM_OBJECT(MyCell, Cell) {
///       BDM_SIM_OBJECT_HEADER(MyCellExt, 1, data_member_);
///      public:
///       MyCellExt() {}
///       ...
///      private:
///       vec<int> data_member_;
///     };
/// This creates one class and three type aliases
///   * `MyCellExt`: class containing the actual code.
///      The postfix `Ext` stands for extension.
///      NB: Inside the body you have to use `MyCellExt` -- e.g.:
///      `BDM_SIM_OBJECT_HEADER(MyCellExt, ...)` or constructor.
///   * `MyCell`: scalar type alias (no template parameter required)
///      = scalar name
///   * `SoaMyCell`: soa type alias (no template parameter required)
///   * `MyCell_TCTParam_TDerived`: for internal usage only. Used to pass it
///      as template argument for `TBase` (takes two template arguments itself)
/// Furthermore, it creates template spezializations for `ToBackend` and
/// `Capsule`
#define BDM_SIM_OBJECT(sim_object, base_class)                                  \
  template <typename TCompileTimeParam = CompileTimeParam<>,                   \
            typename TDerived = char,                                          \
            template <typename, typename> class TBase =                        \
                base_class##_TCTParam_TDerived>                                \
  struct sim_object##Ext;                                                      \
                                                                               \
  template <template <typename TCompileTimeParam, typename TDerived,           \
                      template <typename, typename> class TBase> class TSoExt> \
  struct Capsule;                                                              \
                                                                               \
  template <typename TSoScalar, typename TBackend>                             \
  struct ToBackend;                                                            \
                                                                               \
  template <typename TCompileTimeParam, typename TDerived>                     \
  using sim_object##_TCTParam_TDerived =                                       \
      sim_object##Ext<TCompileTimeParam, TDerived>;                            \
                                                                               \
  template <>                                                                  \
  struct Capsule<sim_object##Ext> {                                            \
    template <typename TCompileTimeParam, typename TDerived>                   \
    using type = sim_object##Ext<TCompileTimeParam, TDerived,                  \
                                 base_class##_TCTParam_TDerived>;              \
  };                                                                           \
                                                                               \
  using sim_object =                                                           \
      sim_object##Ext<CompileTimeParam<Scalar>, Capsule<sim_object##Ext>>;     \
  using Soa##sim_object =                                                      \
      sim_object##Ext<CompileTimeParam<Soa>, Capsule<sim_object##Ext>>;        \
                                                                               \
  template <>                                                                  \
  struct ToBackend<sim_object, Scalar> {                                       \
    using type = sim_object;                                                   \
  };                                                                           \
  template <>                                                                  \
  struct ToBackend<sim_object, Soa> {                                          \
    using type = Soa##sim_object;                                              \
  };                                                                           \
  template <>                                                                  \
  struct ToBackend<Soa##sim_object, Scalar> {                                  \
    using type = sim_object;                                                   \
  };                                                                           \
  template <>                                                                  \
  struct ToBackend<Soa##sim_object, Soa> {                                     \
    using type = Soa##sim_object;                                              \
  };                                                                           \
                                                                               \
  template <typename TCompileTimeParam>                                        \
  using sim_object##Test =                                                     \
      sim_object##Ext<TCompileTimeParam, Capsule<sim_object##Ext>>;            \
                                                                               \
  template <typename TCompileTimeParam, typename TDerived,                     \
            template <typename, typename> class TBase>                         \
  struct sim_object##Ext : public TBase<TCompileTimeParam, TDerived>

/// Macro to make the out-of-class definition of functions and members
/// less verbose. Inserts the required template statements.
///
///     // Usage:
///     BDM(Cell, SimulationObject) {
///        BDM_CLASS_HEADER(...);
///      public:
///       void Foo();
///       ...
///     };
///     BDM_SO_DEFINE(inline void CellExt)::Foo() { ... }
#define BDM_SO_DEFINE(...)                                 \
  template <typename TCompileTimeParam, typename TDerived, \
            template <typename, typename> class TBase>     \
  __VA_ARGS__<TCompileTimeParam, TDerived, TBase>

/// Macro to define a new simulation object.
/// For testing purposes it is required to specify the name of the compile
/// time parameter struct as additional parameter.
/// \param sim_object
/// \param base_class
/// \param compile_time_param
/// \see BDM_SIM_OBJECT
#define BDM_SIM_OBJECT_TEST(sim_object, base_class, compile_time_param)         \
  template <typename TCompileTimeParam = compile_time_param<>,                 \
            typename TDerived = char,                                          \
            template <typename, typename> class TBase =                        \
                base_class##_TCTParam_TDerived>                                \
  struct sim_object##Ext;                                                      \
                                                                               \
  template <template <typename TCompileTimeParam, typename TDerived,           \
                      template <typename, typename> class TBase> class TSoExt> \
  struct Capsule;                                                              \
                                                                               \
  template <typename TSoScalar, typename TBackend>                             \
  struct ToBackend;                                                            \
                                                                               \
  template <typename TCompileTimeParam, typename TDerived>                     \
  using sim_object##_TCTParam_TDerived =                                       \
      sim_object##Ext<TCompileTimeParam, TDerived>;                            \
                                                                               \
  template <>                                                                  \
  struct Capsule<sim_object##Ext> {                                            \
    template <typename TCompileTimeParam, typename TDerived>                   \
    using type = sim_object##Ext<TCompileTimeParam, TDerived,                  \
                                 base_class##_TCTParam_TDerived>;              \
  };                                                                           \
                                                                               \
  using sim_object =                                                           \
      sim_object##Ext<compile_time_param<Scalar>, Capsule<sim_object##Ext>>;   \
  using Soa##sim_object =                                                      \
      sim_object##Ext<compile_time_param<Soa>, Capsule<sim_object##Ext>>;      \
                                                                               \
  template <>                                                                  \
  struct ToBackend<sim_object, Scalar> {                                       \
    using type = sim_object;                                                   \
  };                                                                           \
  template <>                                                                  \
  struct ToBackend<sim_object, Soa> {                                          \
    using type = Soa##sim_object;                                              \
  };                                                                           \
  template <>                                                                  \
  struct ToBackend<Soa##sim_object, Scalar> {                                  \
    using type = sim_object;                                                   \
  };                                                                           \
  template <>                                                                  \
  struct ToBackend<Soa##sim_object, Soa> {                                     \
    using type = Soa##sim_object;                                              \
  };                                                                           \
                                                                               \
  template <typename TCompileTimeParam>                                        \
  using sim_object##Test =                                                     \
      sim_object##Ext<TCompileTimeParam, Capsule<sim_object##Ext>>;            \
                                                                               \
  template <typename TCompileTimeParam, typename TDerived,                     \
            template <typename, typename> class TBase>                         \
  struct sim_object##Ext : public TBase<TCompileTimeParam, TDerived>

// -----------------------------------------------------------------------------
// Helper macros used to generate code for all data members of a class

#define BDM_SIM_OBJECT_PUSH_BACK_BODY(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_PUSH_BACK_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_PUSH_BACK_BODY_ITERATOR(data_member) \
  data_member.push_back(other.data_member[0]);

#define BDM_SIM_OBJECT_POP_BACK_BODY(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_POP_BACK_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_POP_BACK_BODY_ITERATOR(data_member) \
  data_member.pop_back();

#define BDM_SIM_OBJECT_SWAP_AND_POP_BACK_BODY(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_SWAP_AND_POP_BACK_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_SWAP_AND_POP_BACK_BODY_ITERATOR(data_member) \
  std::swap(data_member[index], data_member[size - 1]);             \
  data_member.pop_back();

#define BDM_SIM_OBJECT_CPY_CTOR_INIT(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_CPY_CTOR_INIT_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_CPY_CTOR_INIT_ITERATOR(data_member) \
  data_member(other->data_member),

#define BDM_SIM_OBJECT_CLEAR_BODY(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_CLEAR_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_CLEAR_BODY_ITERATOR(data_member) data_member.clear();

#define BDM_SIM_OBJECT_RESERVE_BODY(...) \
  EVAL(LOOP_2_1(BDM_SIM_OBJECT_RESERVE_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_RESERVE_BODY_ITERATOR(new_cap, data_member) \
  data_member.reserve(new_cap);

#define BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY_ITERATOR(data_member) \
  data_member[kIdx] = rhs.data_member[0];

#define BDM_SIM_OBJECT_FOREACHDM_BODY(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_FOREACHDM_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_FOREACHDM_BODY_ITERATOR(data_member) \
  f(&data_member, #data_member);

#define BDM_SIM_OBJECT_FOREACHDMIN_BODY(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_FOREACHDMIN_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_FOREACHDMIN_BODY_ITERATOR(data_member) \
  {                                                           \
    auto it = dm_selector.find(#data_member);                 \
    if (it != dm_selector.end()) {                            \
      f(&data_member, #data_member);                          \
      dm_selector.erase(it);                                  \
    }                                                         \
  }

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
/// @param  ...: List of all data members of this class
#define BDM_SIM_OBJECT_HEADER(class_name, class_version_id, ...)               \
 public:                                                                       \
  using Base = TBase<TCompileTimeParam, TDerived>;                             \
                                                                               \
  /** reduce verbosity by defining a local alias */                            \
  using Base::kIdx;                                                            \
                                                                               \
  using Backend = typename Base::Backend;                                      \
                                                                               \
  template <typename T>                                                        \
  using vec = typename Backend::template vec<T>;                               \
                                                                               \
  /** Templated type alias to create the most derived type with a specific */  \
  /** template. */                                                             \
  template <typename TTBackend>                                                \
  using TMostDerived = typename TDerived::template type<                       \
      typename TCompileTimeParam::template Self<TTBackend>, TDerived>;         \
                                                                               \
  /** MostDerived type with scalar backend */                                  \
  using MostDerived = TMostDerived<Scalar>;                                    \
  /** MostDerived type with simulation backend */                              \
  using MostDerivedSB =                                                        \
      TMostDerived<typename TCompileTimeParam::SimulationBackend>;             \
                                                                               \
  /** Templated type alias to obtain the same type as `this`, but with */      \
  /** different backend. */                                                    \
  template <typename TTBackend>                                                \
  using Self =                                                                 \
      class_name<typename TCompileTimeParam::template Self<TTBackend>,         \
                 TDerived, TBase>;                                             \
                                                                               \
  template <typename, typename, template <typename, typename> class>           \
  friend class class_name;                                                     \
                                                                               \
  using value_type = Self<Soa>;                                                \
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
  /** Returns the Scalar name of the container minus the "Ext"     */          \
  static const std::string GetScalarTypeName() {                               \
    static std::string kScalarType = #class_name;                              \
    return kScalarType.substr(0, kScalarType.size() - 3);                      \
  }                                                                            \
                                                                               \
  /** Constructor to create SOA reference object */                            \
  template <typename T>                                                        \
  class_name(T* other, size_t idx)                                             \
      : Base(other, idx),                                                      \
        REMOVE_TRAILING_COMMAS(BDM_SIM_OBJECT_CPY_CTOR_INIT(__VA_ARGS__)) {}   \
                                                                               \
  /** Executes the given function for all data members             */          \
  /**  Function could be a lambda in the following form:           */          \
  /**  `[](auto* data_member, const std::string& dm_name) { ... }` */          \
  template <typename Function, typename T = Backend>                           \
  typename enable_if<is_soa<T>::value>::type ForEachDataMember(Function f) {   \
    BDM_SIM_OBJECT_FOREACHDM_BODY(__VA_ARGS__)                                 \
    Base::ForEachDataMember(f);                                                \
  }                                                                            \
                                                                               \
  /** Executes the given function for the specified data members    */         \
  /** Function could be a lambda in the following form              */         \
  /** `[](auto* data_member, const std::string& dm_name) { ... }`   */         \
  template <typename Function, typename T = Backend>                           \
  typename enable_if<is_soa<T>::value>::type ForEachDataMemberIn(              \
      std::set<std::string> dm_selector, Function f) {                         \
    BDM_SIM_OBJECT_FOREACHDMIN_BODY(__VA_ARGS__)                               \
    Base::ForEachDataMemberIn(dm_selector, f);                                 \
  }                                                                            \
                                                                               \
  /** Equivalent to std::vector<> clear - it removes all elements from */      \
  /** all data members */                                                      \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type clear() {                         \
    std::lock_guard<std::recursive_mutex> lock(Base::mutex_);                  \
    Base::clear();                                                             \
    BDM_SIM_OBJECT_CLEAR_BODY(__VA_ARGS__)                                     \
  }                                                                            \
                                                                               \
  /** Equivalent to std::vector<> reserve - it increases the capacity */       \
  /** of all data member containers */                                         \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type reserve(                          \
      std::size_t new_capacity) {                                              \
    std::lock_guard<std::recursive_mutex> lock(Base::mutex_);                  \
    Base::reserve(new_capacity);                                               \
    BDM_SIM_OBJECT_RESERVE_BODY(new_capacity, __VA_ARGS__)                     \
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
    BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY(__VA_ARGS__)                             \
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
    /* sort indices in descending order to prevent out of bounds accesses */   \
    auto descending = [](auto a, auto b) { return a > b; };                    \
    std::sort(Base::to_be_removed_.begin(), Base::to_be_removed_.end(),        \
              descending);                                                     \
    for (size_t idx : Base::to_be_removed_) {                                  \
      assert(idx < Base::size() && "Removed index outside array boundaries");  \
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
  void PushBackImpl(const TMostDerived<Scalar>& other) override {              \
    BDM_SIM_OBJECT_PUSH_BACK_BODY(__VA_ARGS__);                                \
    Base::PushBackImpl(other);                                                 \
  }                                                                            \
                                                                               \
  /** Swap element with last element and remove last element from each */      \
  /** data member */                                                           \
  void SwapAndPopBack(size_t index, size_t size) override {                    \
    BDM_SIM_OBJECT_SWAP_AND_POP_BACK_BODY(__VA_ARGS__);                        \
    Base::SwapAndPopBack(index, size);                                         \
  }                                                                            \
                                                                               \
  /** Remove last element from each data member */                             \
  void PopBack(size_t index, size_t size) override {                           \
    BDM_SIM_OBJECT_POP_BACK_BODY(__VA_ARGS__);                                 \
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

/// Simulation object pointer. Required to point into simulation objects with
/// `Soa` backend. `SoaRef` has the drawback that its size depends on the number
/// of data members. Benefit compared to SoHandle is, that the compiler knows
/// the type returned by `Get` and can therefore inline the code from the callee
/// and perform optimizations
/// @tparam TSo simulation object type - backend invariant - will be transformed
///         inside the object.
/// @tparam TBackend backend - required to avoid extracting it from TSo which
///         would result in "incomplete type errors" in certain cases.
template <typename TSo, typename TBackend>
class SoPointer {
  /// Determine correct container
  using Container = typename TBackend::template Container<
      typename ToBackend<TSo, TBackend>::type>;

 public:
  SoPointer(Container* container, uint64_t element_idx)
      : so_container_(container), element_idx_(element_idx) {}

  /// constructs an SoPointer object representing a nullptr
  SoPointer() {}

  bool IsNullPtr() const {
    return element_idx_ == std::numeric_limits<uint64_t>::max();
  }

  /// This method is required, since `operator->` must return a pointer type.
  /// This is not possible for Soa backends where `operator[]` returns an
  /// rvalue.
  template <typename TTBackend = TBackend>
  typename std::enable_if<std::is_same<TTBackend, Scalar>::value,
                          typename ToBackend<TSo, Scalar>::type&>::type
  Get() {
    return (*so_container_)[element_idx_];
  }

  template <typename TTBackend = TBackend>
  auto Get(typename std::enable_if<std::is_same<TTBackend, Soa>::value>::type*
               p = 0) {
    return (*so_container_)[element_idx_];
  }

 private:
  Container* so_container_ = nullptr;
  uint64_t element_idx_ = std::numeric_limits<uint64_t>::max();
};

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

template <typename TSimObject, typename TResourceManager = ResourceManager<>>
void Delete(const TSimObject& sim_object) {
  auto rm = TResourceManager::Get();
  auto container = rm->template Get<TSimObject>();
  container->DelayedRemove(sim_object.GetElementIdx());
}

/// Get the diffusion grid which holds the substance of specified name
template <typename TResourceManager = ResourceManager<>>
static DiffusionGrid* GetDiffusionGrid(int substance_id) {
  auto dg = TResourceManager::Get()->GetDiffusionGrid(substance_id);
  assert(dg != nullptr &&
         "Tried to get non-existing diffusion grid. Did you specify the "
         "correct substance name?");
  return dg;
}

/// Get the total number of simulation objects
template <typename TResourceManager = ResourceManager<>>
static size_t GetNumSimObjects() {
  return TResourceManager::Get()->GetNumSimObjects();
}

}  // namespace bdm

#endif  // SIMULATION_OBJECT_UTIL_H_
