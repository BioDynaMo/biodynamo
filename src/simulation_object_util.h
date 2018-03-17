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

/// Templated type trait to convert a simulation object into another Backend.\n
/// Using the internal templated type alias `typename T::template Self<Backend>`
/// is not always possible (if the type is still incomplete).\n
/// Preprocessor macro `BDM_SIM_OBJECT` generates `ADLHelper` function
/// declerations whose return value is the converted type. This techniques is
/// called argument dependant look-up (ADL) and is required to find the type
/// even in different namespaces.
/// @tparam TSoScalar simulation object with scalar backend
/// @tparam TBackend  desired backend
///
///     // Usage:
///     ToBackend<Neuron, Soa> neurons;
template <typename TSoScalar, typename TBackend>
using ToBackend = decltype(ADLHelper(std::declval<TSoScalar*>(), std::declval<TBackend>()));

/// Templated type trait that converts the given type to a scalar backend.\n
/// Shorter version of `ToBackend<SomeType, Scalar>`.
template <typename TSoScalar>
using ToScalar = decltype(ADLHelper(std::declval<TSoScalar*>(), std::declval<Scalar>()));

/// Templated type trait that converts the given type to a soa backend.\n
/// Shorter version of `ToBackend<SomeType, Soa>`.
template <typename TSoScalar>
using ToSoa = decltype(ADLHelper(std::declval<TSoScalar*>(), std::declval<Soa>()));

/// This type trait is need to encapsulate the derived type and pass it into
/// simulation objects. Inside `BDM_SIM_OBJECT` a template specialization is
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
  class sim_object##Ext;                                                      \
                                                                               \
  template <template <typename TCompileTimeParam, typename TDerived,           \
                      template <typename, typename> class TBase> class TSoExt> \
  struct Capsule;                                                              \
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
  /** Functions used to associate a return type with a number of parameter */  \
  /** types: e.g. `SoaCell ADLHelper(Cell, Soa);`*/  \
  /** These functions can then be used to implement `bdm::ToBackend` */  \
  /** This technique is called argument dependant look-up and required to */   \
  /** find this association in different namespaces */   \
  sim_object ADLHelper(sim_object*, Scalar);                                   \
  Soa##sim_object ADLHelper(sim_object*, Soa);                                 \
  sim_object ADLHelper(Soa##sim_object*, Scalar); \
  Soa##sim_object ADLHelper(Soa##sim_object*, Soa); \
                                                                               \
  template <typename TCompileTimeParam>                                        \
  using sim_object##Test =                                                     \
      sim_object##Ext<TCompileTimeParam, Capsule<sim_object##Ext>>;            \
                                                                               \
  template <typename TCompileTimeParam, typename TDerived,                     \
            template <typename, typename> class TBase>                         \
  class sim_object##Ext : public TBase<TCompileTimeParam, TDerived>

/// Macro to make the out-of-class definition of functions and members
/// less verbose. Inserts the required template statements.
///
///     // Usage:
///     BDM(Cell, SimulationObject) {
///        BDM_SIM_OBJECT_HEADER(...);
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
  class sim_object##Ext;                                                      \
                                                                               \
  template <template <typename TCompileTimeParam, typename TDerived,           \
                      template <typename, typename> class TBase> class TSoExt> \
  struct Capsule;                                                              \
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
  /** Functions used to associate a return type with a number of parameter */  \
  /** types: e.g. `SoaCell ADLHelper(Cell, Soa);`*/  \
  /** These functions can then be used to implement `bdm::ToBackend` */  \
  /** This technique is called argument dependant look-up and required to */   \
  /** find this association in different namespaces */   \
  sim_object ADLHelper(sim_object*, Scalar);                                   \
  Soa##sim_object ADLHelper(sim_object*, Soa);                                 \
  sim_object ADLHelper(Soa##sim_object*, Scalar); \
  Soa##sim_object ADLHelper(Soa##sim_object*, Soa); \
      \
  template <typename TCompileTimeParam>                                        \
  using sim_object##Test =                                                     \
      sim_object##Ext<TCompileTimeParam, Capsule<sim_object##Ext>>;            \
                                                                               \
  template <typename TCompileTimeParam, typename TDerived,                     \
            template <typename, typename> class TBase>                         \
  class sim_object##Ext : public TBase<TCompileTimeParam, TDerived>

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
  using SimBackend = typename TCompileTimeParam::SimulationBackend;\
                                                                               \
  /** Templated type alias to create the most derived type with a specific */  \
  /** backend. */                                                             \
  template <typename TTBackend>                                                \
  using TMostDerived = typename TDerived::template type<                       \
      typename TCompileTimeParam::template Self<TTBackend>, TDerived>;         \
  /** MostDerived type with scalar backend */                                  \
  using MostDerived = TMostDerived<Scalar>;                                    \
  /** MostDerived type with simulation backend */                              \
  using MostDerivedSB = TMostDerived<SimBackend>;             \
  /** MostDerived SoPointer type */ \
  using MostDerivedSoPtr = SoPointer<MostDerivedSB, SimBackend>; \
                                                                               \
  /** Templated type alias to obtain the same type as `this`, but with */      \
  /** different backend. */                                                    \
  template <typename TTBackend>                                                \
  using Self =                                                                 \
      class_name<typename TCompileTimeParam::template Self<TTBackend>,         \
                 TDerived, TBase>;                                             \
                 \
  /** Templated type alias to convert an external type to the simulation  */ \
  /** backend.  */\
  template <typename T>\
  using ToSimBackend = decltype(ADLHelper(std::declval<T*>(), std::declval<SimBackend>()));\
\
  /** Templated type alias to get a `SoPointer` for the given external type */ \
  template <typename T> \
  using ToSoPtr = SoPointer<ToSimBackend<T>, SimBackend>;\
                                                                               \
  template <typename, typename, template <typename, typename> class>           \
  friend class class_name;                                                     \
                                                                               \
  /** Only used for Soa backends to be consistent with  */                     \
  /** e.g. `std::vector<T>::value_type`. */                                    \
  using value_type = Self<Soa>;                                                \
                                                                               \
  /** This function is called during ROOT LinkDef generation. */               \
  /** It adds a linkdef entry for each data member or base type. */            \
  /** If this type is subclassed it also adds an entry of itself. */           \
  /** TODO link to documentation */                                            \
  static void AddToLinkDef(std::set<LinkDefDescriptor>& entries) {             \
    AddSelfToLinkDefEntries<Self<Backend>>(entries, true);                     \
    Self<Backend>().ForEachDataMember([&](auto* data_member, const std::string& dm_name) {      \
      AddAllLinkDefEntries<decltype(data_member)>(entries, true);              \
    });                                                                        \
    Base::AddToLinkDef(entries);                                               \
  }                                                                            \
  \
  /** Returns the ResourceManager */ \
  /** Avoids the "invalid use of incomplete type" error caused if the  */ \
  /** global `Rm()` function in resource_manager.h would be used */ \
  /** FIXME: make static */ \
  template <typename TResourceManager = ResourceManager<>> \
  TResourceManager* Rm() { \
    return TResourceManager::Get(); \
  } \
  \
  template <typename TResourceManager = ResourceManager<>> \
  SoHandle GetSoHandle() const { \
    auto type_idx = TResourceManager::template GetTypeIndex<MostDerived>(); \
    return SoHandle(type_idx, Base::GetElementIdx()); \
  }\
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
  MostDerivedSoPtr GetSoPtr() { \
    auto* container = Rm()->template Get<MostDerived>();\
    return MostDerivedSoPtr(container, kIdx);\
    /** FIXME: only works for SOA backends **/ \
    /** FIXME: add test **/ \
  } \
  \
  void RemoveFromSimulation() { \
    auto container = Rm()->template Get<MostDerived>();\
    container->DelayedRemove(kIdx);\
    /** FIXME: only works for SOA backends **/ \
    /** FIXME: add test **/ \
  } \
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
  /** Safe method to add an element to this vector. */                         \
  /** Does not invalidate, iterators, pointers or references. */               \
  /** Changes do not take effect until they are commited.*/                    \
  /** @param element that should be added to the vector*/                      \
  /** @return  index of the added element in `data_`. Will be bigger than*/    \
  /**          `size()` */                                                     \
  template <typename T = Backend>                                              \
  uint64_t DelayedPushBack(                                                    \
      const Self<Scalar>& other) {                                           \
    std::lock_guard<std::recursive_mutex> lock(Base::mutex_);                  \
    PushBackImpl(other); \
    return Base::TotalSize() - 1;                                                      \
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
  void PopBack() override {                           \
    BDM_SIM_OBJECT_POP_BACK_BODY(__VA_ARGS__);                               \
    Base::PopBack();                                                \
  }                                                                            \
                                                                               \
 private:                                                                      \
  BDM_ROOT_CLASS_DEF_OVERRIDE(class_name, class_version_id)

/// Simulation object pointer. Required to point into simulation objects with
/// `Soa` backend. `SoaRef` has the drawback that its size depends on the number
/// of data members. Benefit compared to SoHandle is, that the compiler knows
/// the type returned by `Get` and can therefore inline the code from the callee
/// and perform optimizations
/// @tparam TSoSimBackend simulation object type with simulation backend
/// @tparam TBackend backend - required to avoid extracting it from TSo which
///         would result in "incomplete type errors" in certain cases.
template <typename TSoSimBackend, typename TBackend>
class SoPointer {
  /// Determine correct container
  using Container = typename TBackend::template Container<TSoSimBackend>;

 public:
  SoPointer(Container* container, uint64_t element_idx)
      : so_container_(container), element_idx_(element_idx) {}

  /// constructs an SoPointer object representing a nullptr
  SoPointer() {}

  /// TODO change to operator `so_ptr == nullptr` or `so_ptr != nullptr`
  bool IsNullPtr() const {
    return element_idx_ == std::numeric_limits<uint64_t>::max();
  }

  bool operator==(const SoPointer<TSoSimBackend, TBackend>& other) const {
    return element_idx_ == other.element_idx_ && so_container_ == other.so_container_;
  }

  /// Assignment operator that changes the internal representation to nullptr.
  /// Makes the following statement possible `so_ptr = nullptr;`
  SoPointer<TSoSimBackend, TBackend>& operator=(std::nullptr_t) {
    element_idx_ = std::numeric_limits<uint64_t>::max();
    return *this;
  }

  /// Method to return the object it points to. Unfortunately, it is not
  /// possible to use `operator->`, which would lead to a nice syntax like:
  /// `so_ptr->SomeFunction()`. `operator->` must return a pointer which is
  /// not possible for Soa backends (`operator[]` returns a temporary SoaRef
  /// object).
  template <typename TTBackend = TBackend>
  auto& Get(typename std::enable_if<std::is_same<TTBackend, Scalar>::value>::type*
               p = 0) {
    assert(!IsNullPtr());
    return (*so_container_)[element_idx_];
  }

  template <typename TTBackend = TBackend>
  const auto& Get(typename std::enable_if<std::is_same<TTBackend, Scalar>::value>::type*
               p = 0) const {
    assert(!IsNullPtr());
    return (*so_container_)[element_idx_];
  }

  template <typename TTBackend = TBackend>
  auto Get(typename std::enable_if<std::is_same<TTBackend, Soa>::value>::type*
               p = 0) {
    assert(!IsNullPtr());
    return (*so_container_)[element_idx_];
  }

  template <typename TTBackend = TBackend>
  const auto Get(typename std::enable_if<std::is_same<TTBackend, Soa>::value>::type*
               p = 0) const {
    assert(!IsNullPtr());
    return (*so_container_)[element_idx_];
  }

  friend std::ostream& operator<<(std::ostream& str, const SoPointer<TSoSimBackend, TBackend>& so_ptr) {
    str << "{ container: " << so_ptr.so_container_ << ", element_idx: " << so_ptr.element_idx_ << "}";
    return str;
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
uint64_t Divide(T&& progenitor, Container* container, Params... parameters) {
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
auto Divide(T&& progenitor, Params... parameters) {
  // daughter type is scalar version of T
  using DaughterType =
      typename std::remove_reference<T>::type::template Self<Scalar>;
  auto container = TResourceManager::Get()->template Get<DaughterType>();
  return typename DaughterType::MostDerivedSoPtr(container, Divide(progenitor, container, parameters...));
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
