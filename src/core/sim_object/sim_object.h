// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_SIM_OBJECT_SIM_OBJECT_H_
#define CORE_SIM_OBJECT_SIM_OBJECT_H_

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "core/biology_module/biology_module.h"
#include "core/diffusion_grid.h"
#include "core/event/event.h"
#include "core/resource_manager.h"
#include "core/sim_object/backend.h"
#include "core/sim_object/so_pointer.h"
#include "core/sim_object/so_uid.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/macros.h"
#include "core/util/root.h"
#include "core/util/type.h"

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
using ToBackend =
    decltype(ADLHelper(std::declval<TSoScalar *>(), std::declval<TBackend>()));

/// Templated type trait that converts the given type to a scalar backend.\n
/// Shorter version of `ToBackend<SomeType, Scalar>`.
template <typename TSoScalar>
using ToScalar =
    decltype(ADLHelper(std::declval<TSoScalar *>(), std::declval<Scalar>()));

/// Templated type trait that converts the given type to a soa backend.\n
/// Shorter version of `ToBackend<SomeType, Soa>`.
template <typename TSoScalar>
using ToSoa =
    decltype(ADLHelper(std::declval<TSoScalar *>(), std::declval<Soa>()));

/// This type trait is need to encapsulate the derived type and pass it into
/// simulation objects. Inside `BDM_SIM_OBJECT` a template specialization is
/// created for each simulation object.
/// @tparam TSoExt template template parameter of a simulation object
template <template <typename TCompileTimeParam, typename TDerived> class TSoExt>
struct Capsule;

/// Macro to define a new simulation object
/// @param sim_object
/// @param base_class
///
///     // Example usage to extend class Cell
///     BDM_SIM_OBJECT(MyCell, Cell) {
///       BDM_SIM_OBJECT_HEADER(MyCell, Cell, 1, data_member_);
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
#define BDM_SIM_OBJECT(sim_object, base_class)                                \
  template <typename TCompileTimeParam = CompileTimeParam<>,                  \
            typename TDerived = char>                                         \
  class sim_object##Ext;                                                      \
                                                                              \
  template <template <typename TCTParam, typename TDerived> class TSoExt>     \
  struct Capsule;                                                             \
                                                                              \
  template <>                                                                 \
  struct Capsule<sim_object##Ext> {                                           \
    template <typename TCompileTimeParam, typename TDerived>                  \
    using type = sim_object##Ext<TCompileTimeParam, TDerived>;                \
  };                                                                          \
                                                                              \
  using sim_object =                                                          \
      sim_object##Ext<CompileTimeParam<Scalar>, Capsule<sim_object##Ext>>;    \
  using Soa##sim_object =                                                     \
      sim_object##Ext<CompileTimeParam<Soa>, Capsule<sim_object##Ext>>;       \
                                                                              \
  /** Functions used to associate a return type with a number of parameter */ \
  /** types: e.g. `SoaCell ADLHelper(Cell, Soa);`*/                           \
  /** These functions can then be used to implement `bdm::ToBackend` */       \
  /** This technique is called argument dependant look-up and required to */  \
  /** find this association in different namespaces */                        \
  sim_object ADLHelper(sim_object *, Scalar);                                 \
  Soa##sim_object ADLHelper(sim_object *, Soa);                               \
  sim_object ADLHelper(Soa##sim_object *, Scalar);                            \
  Soa##sim_object ADLHelper(Soa##sim_object *, Soa);                          \
                                                                              \
  template <typename TCompileTimeParam>                                       \
  using sim_object##Test =                                                    \
      sim_object##Ext<TCompileTimeParam, Capsule<sim_object##Ext>>;           \
                                                                              \
  template <typename TCompileTimeParam, typename TDerived>                    \
  class sim_object##Ext : public base_class##Ext<TCompileTimeParam, TDerived>

/// Macro to make the out-of-class definition of functions and members
/// less verbose. Inserts the required template statements.
///
///     // Usage:
///     BDM(Cell, SimObject) {
///        BDM_SIM_OBJECT_HEADER(...);
///      public:
///       void Foo();
///       ...
///     };
///     BDM_SO_DEFINE(inline void CellExt)::Foo() { ... }
#define BDM_SO_DEFINE(...)                                 \
  template <typename TCompileTimeParam, typename TDerived> \
  __VA_ARGS__<TCompileTimeParam, TDerived>

// -----------------------------------------------------------------------------
// Helper macros used to generate code for all data members of a class

#define BDM_SIM_OBJECT_PUSH_BACK_BODY(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_PUSH_BACK_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_PUSH_BACK_BODY_ITERATOR(data_member) \
  data_member.push_back(other.data_member[other.kIdx]);

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

#define BDM_SIM_OBJECT_RESIZE_BODY(...) \
  EVAL(LOOP_2_1(BDM_SIM_OBJECT_RESIZE_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_RESIZE_BODY_ITERATOR(new_cap, data_member) \
  data_member.resize(new_cap);

#define BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY_ITERATOR(data_member) \
  data_member[kIdx] = rhs.data_member[rhs.kIdx];

#define BDM_SIM_OBJECT_ASSIGNMENT_OP_MOVE_BODY(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_ASSIGNMENT_OP_MOVE_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_ASSIGNMENT_OP_MOVE_BODY_ITERATOR(data_member) \
  data_member = std::move(rhs.data_member);

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
/// @param  class_name scalar class name of the simulation object
/// @param   base_class scalar class name of the base simulation object
/// @param   class_version_id required for ROOT I/O (see ROOT BDM_CLASS_DEF
///          Macro).
///          Every time the layout of the class is changed, class_version_id
///          must be incremented by one. The class_version_id should be greater
///          or equal to 1.
/// @param  ...: List of all data members of this class
#define BDM_SIM_OBJECT_HEADER(class_name, base_class, class_version_id, ...)   \
 public:                                                                       \
  using Base = base_class##Ext<TCompileTimeParam, TDerived>;                   \
                                                                               \
  /** reduce verbosity by defining a local alias */                            \
  using Base::kIdx;                                                            \
                                                                               \
  using Backend = typename Base::Backend;                                      \
                                                                               \
  template <typename T>                                                        \
  using vec = typename Backend::template vec<T>;                               \
                                                                               \
  using SimBackend = typename TCompileTimeParam::SimulationBackend;            \
                                                                               \
  /** Templated type alias to create the most derived type with a specific */  \
  /** backend. */                                                              \
  template <typename TTBackend>                                                \
  using MostDerived = typename TDerived::template type<                        \
      typename TCompileTimeParam::template Self<TTBackend>, TDerived>;         \
  /** MostDerived type with scalar backend */                                  \
  using MostDerivedScalar = MostDerived<Scalar>;                               \
  /** MostDerived SoPointer type with simulation backend */                    \
  using MostDerivedSoPtr = SoPointer<MostDerived<SimBackend>, SimBackend>;     \
                                                                               \
  /** Templated type alias to obtain the same type as `this`, but with */      \
  /** different backend. */                                                    \
  template <typename TTBackend>                                                \
  using Self =                                                                 \
      class_name##Ext<typename TCompileTimeParam::template Self<TTBackend>,    \
                      TDerived>;                                               \
                                                                               \
  /** Templated type alias to convert an external type to the simulation  */   \
  /** backend.  */                                                             \
  template <typename T>                                                        \
  using ToSimBackend =                                                         \
      decltype(ADLHelper(std::declval<T *>(), std::declval<SimBackend>()));    \
                                                                               \
  /** Templated type alias to get a `SoPointer` for the given external type */ \
  template <typename T>                                                        \
  using ToSoPtr = SoPointer<ToSimBackend<T>, SimBackend>;                      \
                                                                               \
  template <typename, typename>                                                \
  friend class class_name##Ext;                                                \
                                                                               \
  /** friend event handler to give it access to private members */             \
  template <typename TEvent, typename TFirst, typename... TRemaining>          \
  friend struct EventHandler;                                                  \
                                                                               \
  /** Only used for Soa backends to be consistent with  */                     \
  /** e.g. `std::vector::value_type`. */                                       \
  using value_type = Self<Soa>;                                                \
                                                                               \
  using Simulation_t =                                                         \
      Simulation<typename TCompileTimeParam::template Self<Soa>>;              \
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
  static const std::string GetScalarTypeName() { return #class_name; }         \
                                                                               \
  explicit class_name##Ext(TRootIOCtor *io_ctor) {}                            \
  class_name##Ext(const class_name##Ext &other) = default;                     \
                                                                               \
  /** Constructor to create SOA reference object */                            \
  template <typename T>                                                        \
  class_name##Ext(T *other, size_t idx)                                        \
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
    Base::clear();                                                             \
    BDM_SIM_OBJECT_CLEAR_BODY(__VA_ARGS__)                                     \
  }                                                                            \
                                                                               \
  /** Equivalent to std::vector<> reserve - it increases the capacity */       \
  /** of all data member containers */                                         \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type reserve(                          \
      std::size_t new_capacity) {                                              \
    Base::reserve(new_capacity);                                               \
    BDM_SIM_OBJECT_RESERVE_BODY(new_capacity, __VA_ARGS__)                     \
  }                                                                            \
                                                                               \
  /** Equivalent to std::vector<> resize */                                    \
  template <typename T = Backend>                                              \
  typename enable_if<is_soa<T>::value>::type resize(std::size_t new_size) {    \
    Base::resize(new_size);                                                    \
    BDM_SIM_OBJECT_RESIZE_BODY(new_size, __VA_ARGS__)                          \
  }                                                                            \
                                                                               \
  Self<SoaRef> operator[](size_t idx) { return Self<SoaRef>(this, idx); }      \
                                                                               \
  const Self<SoaRef> operator[](size_t idx) const {                            \
    return Self<SoaRef>(const_cast<Self<Backend> *>(this), idx);               \
  }                                                                            \
                                                                               \
  template <typename T = Backend>                                              \
  typename enable_if<is_same<T, SoaRef>::value, Self<SoaRef> &>::type          \
  operator=(const Self<Scalar> &rhs) {                                         \
    BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY(__VA_ARGS__)                             \
    Base::operator=(rhs);                                                      \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  template <typename T = Backend>                                              \
  typename enable_if<is_same<T, Soa>::value || is_same<T, Scalar>::value,      \
                     Self<Backend> &>::type                                    \
  operator=(Self<Backend> &&rhs) {                                             \
    BDM_SIM_OBJECT_ASSIGNMENT_OP_MOVE_BODY(__VA_ARGS__)                        \
    Base::operator=(std::move(rhs));                                           \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  Self<Backend> &operator=(const Self<Backend> &rhs) {                         \
    BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY(__VA_ARGS__)                             \
    Base::operator=(rhs);                                                      \
    return *this;                                                              \
  }                                                                            \
                                                                               \
 protected:                                                                    \
  /** Equivalent to std::vector<> push_back - it adds the scalar values to */  \
  /** all data members */                                                      \
  void PushBackImpl(const MostDerived<Scalar> &other) override {               \
    BDM_SIM_OBJECT_PUSH_BACK_BODY(__VA_ARGS__);                                \
    Base::PushBackImpl(other);                                                 \
  }                                                                            \
                                                                               \
  /** Equivalent to std::vector<> push_back - it adds the scalar values to */  \
  /** all data members */                                                      \
  void PushBackImpl(const MostDerived<SoaRef> &other) override {               \
    BDM_SIM_OBJECT_PUSH_BACK_BODY(__VA_ARGS__);                                \
    Base::PushBackImpl(other);                                                 \
  }                                                                            \
                                                                               \
  /** Remove last element from each data member */                             \
  void PopBack() override {                                                    \
    BDM_SIM_OBJECT_POP_BACK_BODY(__VA_ARGS__);                                 \
    Base::PopBack();                                                           \
  }                                                                            \
                                                                               \
  /** Cast `this` to the base class pointer (one level up) */                  \
  Base *UpCast() { return static_cast<Base *>(this); }                         \
                                                                               \
  /** Cast `this` to the base class pointer (one level up) */                  \
  const Base *UpCast() const { return static_cast<const Base *>(this); }       \
                                                                               \
 private:                                                                      \
  /** Cast `this` to the most derived type */                                  \
  /** Used to call the method of the subclass without virtual functions */     \
  /** e.g. `ThisMD()->Method()` */                                             \
  /** (CRTP - static polymorphism) */                                          \
  MostDerived<Backend> *ThisMD() {                                             \
    return static_cast<MostDerived<Backend> *>(this);                          \
  }                                                                            \
  const MostDerived<Backend> *ThisMD() const {                                 \
    return static_cast<MostDerived<Backend> *>(this);                          \
  }                                                                            \
                                                                               \
  BDM_ROOT_CLASS_DEF_OVERRIDE(class_name##Ext, class_version_id)

template <typename TCompileTimeParam, typename TDerived>
class ScalarSimObject;

/// Contains implementation for SimObject that are specific to SOA
/// backend. The peculiarity of SOA objects is that it is simulation object
/// and container at the same time.
template <typename TCompileTimeParam, typename TDerived>
class SoaSimObject {
 public:
  using Backend = typename TCompileTimeParam::Backend;
  template <typename, typename>
  friend class SoaSimObject;

  template <typename TTBackend>
  using MostDerived = typename TDerived::template type<
      typename TCompileTimeParam::template Self<TTBackend>, TDerived>;

  template <typename TBackend>
  using Self = SoaSimObject<typename TCompileTimeParam::template Self<TBackend>,
                            TDerived>;

  SoaSimObject() : size_(1) {}

  /// Detect failing return value optimization (RVO)
  /// Copy-ctor declaration to please compiler, but missing implementation.
  /// Therefore, if it gets called somewhere (failing RVO optimization),
  /// the linker would throw an error.
  explicit SoaSimObject(const Self<SoaRef> &other);

  /// Detect failing return value optimization (RVO)
  /// Copy-ctor declaration to please compiler, but missing implementation.
  /// Therefore, if it gets called somewhere (failing RVO optimization),
  /// the linker would throw an error.
  explicit SoaSimObject(const Self<Soa> &other);

  template <typename T>
  SoaSimObject(T *other, size_t idx) : kIdx(idx), size_(other->size_) {}

  virtual ~SoaSimObject() {}

  SoaSimObject &operator=(SoaSimObject &&other) {
    size_ = other.size_;
    return *this;
  }

  SoaSimObject &operator=(const Self<Soa> &other) {
    size_ = other.size_;
    return *this;
  }

  SoaSimObject &operator=(const Self<SoaRef> &) {
    // Do not copy size! This is used to assign: soa[idx] = soaref;
    return *this;
  }

  Self<Backend> &operator=(
      const ScalarSimObject<typename TCompileTimeParam::template Self<Scalar>,
                            TDerived> &) {
    // Do not copy size! This is used to assign: soa[idx] = scalar;
    return *this;
  }

  uint32_t GetElementIdx() const { return kIdx; }

  void SetElementIdx(uint32_t element_idx) {}

  /// Returns the vector's size. Uncommited changes are not taken into account
  size_t size() const {  // NOLINT
    return size_;
  }

  template <typename TTBackend>
  void push_back(const MostDerived<TTBackend> &element) {  // NOLINT
    PushBackImpl(element);
  }

  void pop_back() {  // NOLINT
    PopBack();
  }

  /// Equivalent to std::vector<> clear - it removes all elements from all
  /// data members
  void clear() {  // NOLINT
    size_ = 0;
  }

  /// Equivalent to std::vector<> reserve - it increases the capacity
  /// of all data member containers
  void reserve(size_t new_capacity) {}  // NOLINT

  /// Equivalent to std::vector<> resize
  void resize(size_t new_size) {  // NOLINT
    size_ = new_size;
  }

  template <typename Function>
  void ForEachDataMember(Function l) {}

  template <typename Function>
  void ForEachDataMemberIn(const std::set<std::string> &dm_selector,
                           Function f) {
    // validate data_members
    // all data members should have been removed from the set. Remaining
    // entries do not exist
    if (dm_selector.size() != 0) {
      std::stringstream sstr;
      for (auto &element : dm_selector) {
        sstr << element << ", ";
      }
      Fatal("ForEachDataMemberIn",
            "Please check your config file. The following data members do not "
            "exist: %s",
            sstr.str().c_str());
    }
  }

 protected:
  const size_t kIdx = 0;

  /// Append a scalar element
  virtual void PushBackImpl(const MostDerived<Scalar> &other) { size_++; }

  /// Append a soa ref element
  virtual void PushBackImpl(const MostDerived<SoaRef> &other) { size_++; }

  /// Remove last element
  virtual void PopBack() { size_--; }

 private:
  /// size_ is of type size_t& if Backend == SoaRef; otherwise size_t
  typename type_ternary_operator<is_same<Backend, SoaRef>::value, size_t &,
                                 size_t>::type size_;

  // use modified class def, due to possible SoaRef backend
  BDM_ROOT_CLASS_DEF(SoaSimObject, 1);
};

/// Contains implementations for SimObject that are specific to scalar
/// backend
template <typename TCompileTimeParam, typename TDerived>
class ScalarSimObject {
 public:
  using Backend = typename TCompileTimeParam::Backend;
  template <typename TTBackend>
  using MostDerived = typename TDerived::template type<
      typename TCompileTimeParam::template Self<TTBackend>, TDerived>;

  ScalarSimObject() : element_idx_(0) {}
  ScalarSimObject(const ScalarSimObject &other)
      : element_idx_(other.element_idx_) {}

  virtual ~ScalarSimObject() {}

  ScalarSimObject &operator=(ScalarSimObject &&other) {
    element_idx_ = other.element_idx_;
    return *this;
  }

  ScalarSimObject &operator=(const ScalarSimObject &) { return *this; }

  std::size_t size() const { return 1; }  // NOLINT

  /// NB: Cannot be used in the Constructur, because the ResourceManager`
  /// didn't initialize `element_idx_` yet.
  uint32_t GetElementIdx() const { return element_idx_; }

  // assign the array index of this object in the ResourceManager
  void SetElementIdx(uint32_t element_idx) { element_idx_ = element_idx; }

 protected:
  static const std::size_t kIdx = 0;
  // array index of this object in the ResourceManager
  uint32_t element_idx_ = 0;

  /// Append a scalar element
  virtual void PushBackImpl(const MostDerived<Scalar> &other) {}

  /// Append a SoaRef element
  virtual void PushBackImpl(const MostDerived<SoaRef> &other) {}

  /// Remove last element
  virtual void PopBack() {}

  BDM_ROOT_CLASS_DEF(ScalarSimObject, 2);
};

/// Helper type trait to map backends to simulation object implementations
template <typename TCompileTimeParam, typename TDerived>
struct SimObjectImpl {
  using Backend = typename TCompileTimeParam::Backend;
  using type = typename type_ternary_operator<
      is_same<Backend, Scalar>::value,
      ScalarSimObject<TCompileTimeParam, TDerived>,
      SoaSimObject<TCompileTimeParam, TDerived>>::type;
};

/// Contains code required by all simulation objects
template <typename TCompileTimeParam, typename TDerived>
class SimObjectExt : public SimObjectImpl<TCompileTimeParam, TDerived>::type {
  // used to fullfill BDM_SIM_OBJECT_HEADER requirement
  template <typename T, typename U>
  using SimObjectBaseExt = typename SimObjectImpl<T, U>::type;

  BDM_SIM_OBJECT_HEADER(SimObject, SimObjectBase, 1, uid_, box_idx_,
                        biology_modules_, run_bm_loop_idx_, run_displacement_,
                        run_displacement_for_all_next_ts_,
                        run_displacement_next_ts_, numa_node_);

 public:
  SimObjectExt() : Base() { uid_[kIdx] = SoUidGenerator::Get()->NewSoUid(); }

  template <typename TEvent, typename TOther>
  SimObjectExt(const TEvent &event, TOther *other, uint64_t new_oid = 0) {
    uid_[kIdx] = SoUidGenerator::Get()->NewSoUid();
    box_idx_[kIdx] = other->GetBoxIdx();
    // biology modules
    auto &other_bms = other->biology_modules_[other->kIdx];
    // copy biology_modules_ to me
    CopyBiologyModules(event, &other_bms, &biology_modules_[kIdx]);
  }

  virtual ~SimObjectExt() {}

  /// This function determines if the type of this simulation object is the same
  /// as `TSo` without taking the backend into account.
  /// @tparam TSo Simulation object type with any backend
  template <typename TSo>
  static constexpr bool IsSoType() {
    using TSoScalar = typename TSo::template Self<Scalar>;
    return std::is_same<MostDerived<Scalar>, TSoScalar>::value;
  }

  /// This function determines if the type of this simulation object is the same
  /// as `object` without taking the backend into account.
  /// @param object simulation object can have any backend
  template <typename TSo>
  static constexpr bool IsSoType(const TSo *object) {
    using Type = std::decay_t<std::remove_pointer_t<decltype(object)>>;
    using ScalarType = typename Type::template Self<Scalar>;
    return std::is_same<MostDerived<Scalar>, ScalarType>::value;
  }

  /// Casts this to a simulation object of type `TSo` with the current `Backend`
  /// This function is used to simulate if constexpr functionality and won't be
  /// needed after we swith to C++17
  /// @tparam TSo target simulaton object type with any backend
  template <typename TSo>
  constexpr auto &&ReinterpretCast() {
    using TargetType = typename TSo::template Self<Backend>;
    return reinterpret_cast<TargetType &&>(*this);
  }

  /// Casts this to a simulation object of type `TSo` with the current `Backend`
  /// This function is used to simulate if constexpr functionality and won't be
  /// needed after we swith to C++17
  /// @tparam TSo target simulaton object type with any backend
  template <typename TSo>
  constexpr const auto *ReinterpretCast() const {
    using TargetType = typename TSo::template Self<Backend>;
    return reinterpret_cast<const TargetType *>(this);
  }

  /// Casts this to a simulation object of type `TSo` with the current `Backend`
  /// This function is used to simulate if constexpr functionality and won't be
  /// needed after we swith to C++17
  /// @tparam TSo target simulaton object type with any backend
  template <typename TSo>
  constexpr auto &&ReinterpretCast(const TSo *object) {
    using TargetType = typename TSo::template Self<Backend>;
    return reinterpret_cast<TargetType &&>(*this);
  }

  MostDerived<Backend> *operator->() {
    return static_cast<MostDerived<Backend> *>(this);
  }

  const MostDerived<Backend> *operator->() const {
    return static_cast<const MostDerived<Backend> *>(this);
  }

  void RunDiscretization() {}

  SoUid GetUid() const { return uid_[kIdx]; }

  uint32_t GetBoxIdx() const { return box_idx_[kIdx]; }

  void SetBoxIdx(uint32_t idx) { box_idx_[kIdx] = idx; }

  void SetRunDisplacementNextTimestep(bool run) const {
    const_cast<SimObjectExt *>(this)->run_displacement_next_ts_[kIdx] = run;
  }

  bool GetRunDisplacementForAllNextTs() const {
    return run_displacement_for_all_next_ts_[kIdx];
  }

  void SetRunDisplacementForAllNextTs(bool value = true) {
    run_displacement_for_all_next_ts_[kIdx] = value;
  }

  void ApplyRunDisplacementForAllNextTs() {
    if (!run_displacement_for_all_next_ts_[kIdx]) {
      return;
    }
    run_displacement_for_all_next_ts_[kIdx] = false;
    run_displacement_next_ts_[kIdx] = true;
    auto *ctxt = Simulation_t::GetActive()->GetExecutionContext();
    ctxt->ForEachNeighbor(
        [this](auto *neighbor, double squared_distance) {
          double distance =
              this->ThisMD()->GetDiameter() + neighbor->GetDiameter();
          if (squared_distance < distance * distance) {
            neighbor->SetRunDisplacementNextTimestep(true);
          }
        },
        *ThisMD());
  }

  void UpdateRunDisplacement() {
    run_displacement_[kIdx] = run_displacement_next_ts_[kIdx];
    run_displacement_next_ts_[kIdx] = false;
  }

  bool RunDisplacement() const { return run_displacement_[kIdx]; }

  // TODO(ahmad) this only works for SOA backend add check for SOA
  // used only for cuda and opencl code
  uint32_t *GetBoxIdPtr() { return box_idx_.data(); }

  /// Return simulation object pointer
  MostDerivedSoPtr GetSoPtr() const { return MostDerivedSoPtr(uid_[kIdx]); }

  void SetNumaNode(typename SoHandle::NumaNode_t numa_node) {
    numa_node_[kIdx] = numa_node;
  }

  template <typename TResourceManager = ResourceManager<>>
  SoHandle GetSoHandle() const {
    auto type_idx =
        TResourceManager::template GetTypeIndex<MostDerivedScalar>();
    return SoHandle(numa_node_[kIdx], type_idx, Base::GetElementIdx());
  }

  // ---------------------------------------------------------------------------
  // Biology modules
  using BiologyModules =
      typename TCompileTimeParam::template CTMap<MostDerivedScalar,
                                                 0>::BiologyModules::Variant_t;

  /// Add a biology module to this cell
  /// @tparam TBiologyModule type of the biology module. Must be in the set of
  ///         types specified in `BiologyModules`
  template <typename TBiologyModule>
  void AddBiologyModule(TBiologyModule &&module) {
    biology_modules_[kIdx].emplace_back(module);
  }

  /// Remove a biology module from this cell
  template <typename TBiologyModule>
  void RemoveBiologyModule(const TBiologyModule *remove_module) {
    for (unsigned int i = 0; i < biology_modules_[kIdx].size(); i++) {
      const TBiologyModule *module =
          get_if<TBiologyModule>(&biology_modules_[kIdx][i]);
      if (module == remove_module) {
        biology_modules_[kIdx].erase(biology_modules_[kIdx].begin() + i);
        // if remove_module was before or at the current run_bm_loop_idx_[kidx],
        // correct it by subtracting one.
        run_bm_loop_idx_[kIdx] -= i > run_bm_loop_idx_[kIdx] ? 0 : 1;
      }
    }
  }

  /// Execute all biology modulesq
  void RunBiologyModules() {
    RunVisitor<MostDerived<Backend>> visitor(
        static_cast<MostDerived<Backend> *>(this));
    for (run_bm_loop_idx_[kIdx] = 0;
         run_bm_loop_idx_[kIdx] < biology_modules_[kIdx].size();
         ++run_bm_loop_idx_[kIdx]) {
      auto &module = biology_modules_[kIdx][run_bm_loop_idx_[kIdx]];
      visit(visitor, module);
    }
  }

  /// Get all biology modules of this cell that match the given type.
  /// @tparam TBiologyModule  type of the biology module
  template <typename TBiologyModule>
  std::vector<const TBiologyModule *> GetBiologyModules() const {
    std::vector<const TBiologyModule *> modules;
    for (unsigned int i = 0; i < biology_modules_[kIdx].size(); i++) {
      const TBiologyModule *module =
          get_if<TBiologyModule>(&biology_modules_[kIdx][i]);
      if (module != nullptr) {
        modules.push_back(module);
      }
    }
    return modules;
  }

  /// Return all biology modules
  const auto &GetAllBiologyModules() const { return biology_modules_[kIdx]; }
  // ---------------------------------------------------------------------------

  void RemoveFromSimulation() const {
    Simulation_t::GetActive()->GetExecutionContext()->RemoveFromSimulation(
        uid_[kIdx]);
  }

  template <typename TEvent, typename TOther>
  void EventHandler(const TEvent &event, TOther *other) {
    // Run displacement if a new sim object has been created with an event.
    SetRunDisplacementForAllNextTs();
    // call event handler for biology modules
    auto *other_bms = &(other->biology_modules_[other->kIdx]);
    BiologyModuleEventHandler(event, &(biology_modules_[kIdx]), other_bms);
  }

  template <typename TEvent, typename TLeft, typename TRight>
  void EventHandler(const TEvent &event, TLeft *left, TRight *right) {
    // Run displacement if a new sim object has been created with an event.
    SetRunDisplacementForAllNextTs();
    // call event handler for biology modules
    auto *left_bms = &(left->biology_modules_[left->kIdx]);
    auto *right_bms = &(right->biology_modules_[right->kIdx]);
    BiologyModuleEventHandler(event, &(biology_modules_[kIdx]), left_bms,
                              right_bms);
  }

 protected:
  /// unique id
  vec<SoUid> uid_ = {{}};
  /// Grid box index
  vec<uint32_t> box_idx_ = {{}};
  /// collection of biology modules which define the internal behavior
  vec<std::vector<BiologyModules>> biology_modules_;

 private:
  /// Helper variable used to support removal of biology modules while
  /// `RunBiologyModules` iterates over them.
  /// Due to problems with restoring this member for the SOA data layout, it is
  /// not ignored for ROOT I/O.
  vec<uint32_t> run_bm_loop_idx_ = {{0}};

  vec<bool> run_displacement_ = {{true}};                   //!
  vec<bool> run_displacement_for_all_next_ts_ = {{false}};  //!
  vec<bool> run_displacement_next_ts_ = {{true}};           //!

  /// This data member holds information on which NUMA node this sim object is
  /// stored.
  vec<typename SoHandle::NumaNode_t> numa_node_ = {{}};

  /// @brief Function to copy biology modules from one structure to another
  /// @param event event will be passed on to biology module to determine
  ///        whether it should be copied to destination
  /// @param src  source vector of biology modules
  /// @param dest destination vector of biology modules
  /// @tparam TBiologyModules std::vector<Variant<[list of biology modules]>>
  template <typename TEvent, typename TSrcBms, typename TDestBms>
  void CopyBiologyModules(const TEvent &event, TSrcBms *src, TDestBms *dest) {
    auto copy = [&](auto &bm) {
      if (bm.Copy(event.kEventId)) {
        raw_type<decltype(bm)> new_bm(event, &bm);
        dest->emplace_back(std::move(new_bm));
      }
    };
    for (auto &module : *src) {
      visit(copy, module);
    }
  }

  /// @brief Function to invoke the EventHandler of the biology module or remove
  ///                  it from `current`.
  /// Forwards the event handler call to each biology modules of the triggered
  /// simulation object and removes biology modules if they are flagged.
  template <typename TEvent, typename TBiologyModules1,
            typename... TBiologyModules>
  void BiologyModuleEventHandler(const TEvent &event, TBiologyModules1 *current,
                                 TBiologyModules *... bms) {
    // call event handler for biology modules
    uint64_t cnt = 0;
    auto call_bm_event_handler = [&](auto &bm) {
      using BiologyModuleType = raw_type<decltype(bm)>;

      /// return nullptr of condition is false or pointer to object in variant
      auto extract = [](bool condition, auto *variant) -> BiologyModuleType * {
        if (condition) {
          return get_if<BiologyModuleType>(variant);
        }
        return nullptr;
      };

      if (!bm.Remove(event.kEventId)) {
        bool copy = bm.Copy(event.kEventId);
        bm.EventHandler(event, extract(copy, &((*bms)[cnt]))...);
        cnt += copy ? 1 : 0;
      }
    };
    for (auto &el : *current) {
      visit(call_bm_event_handler, el);
    }

    // remove biology modules from current
    bool remove;
    auto remove_from_current = [&](auto &bm) {
      remove = bm.Remove(event.kEventId);
    };
    for (auto it = current->begin(); it != current->end();) {
      remove = false;
      visit(remove_from_current, *it);
      if (remove) {
        it = current->erase(it);
      } else {
        ++it;
      }
    }
  }
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SIM_OBJECT_H_
