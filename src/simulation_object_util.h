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
#include "simulation.h"
#include "so_pointer.h"
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
using ToBackend =
    decltype(ADLHelper(std::declval<TSoScalar*>(), std::declval<TBackend>()));

/// Templated type trait that converts the given type to a scalar backend.\n
/// Shorter version of `ToBackend<SomeType, Scalar>`.
template <typename TSoScalar>
using ToScalar =
    decltype(ADLHelper(std::declval<TSoScalar*>(), std::declval<Scalar>()));

/// Templated type trait that converts the given type to a soa backend.\n
/// Shorter version of `ToBackend<SomeType, Soa>`.
template <typename TSoScalar>
using ToSoa =
    decltype(ADLHelper(std::declval<TSoScalar*>(), std::declval<Soa>()));

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
  sim_object ADLHelper(sim_object*, Scalar);                                  \
  Soa##sim_object ADLHelper(sim_object*, Soa);                                \
  sim_object ADLHelper(Soa##sim_object*, Scalar);                             \
  Soa##sim_object ADLHelper(Soa##sim_object*, Soa);                           \
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
///     BDM(Cell, SimulationObject) {
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

#define BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY(...) \
  EVAL(LOOP(BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY_ITERATOR, __VA_ARGS__))

#define BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY_ITERATOR(data_member) \
  data_member[kIdx] = rhs.data_member[0];

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
      decltype(ADLHelper(std::declval<T*>(), std::declval<SimBackend>()));     \
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
  explicit class_name##Ext(TRootIOCtor* io_ctor) {}                            \
  class_name##Ext(const class_name##Ext& other) = default;                     \
                                                                               \
  /** Constructor to create SOA reference object */                            \
  template <typename T>                                                        \
  class_name##Ext(T* other, size_t idx)                                        \
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
  Self<Backend>& operator=(const Self<Scalar>& rhs) {                                         \
    BDM_SIM_OBJECT_ASSIGNMENT_OP_BODY(__VA_ARGS__)                             \
    Base::operator=(rhs);                                                      \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  Self<Backend>& operator=(Self<Backend>&& rhs) {                              \
    BDM_SIM_OBJECT_ASSIGNMENT_OP_MOVE_BODY(__VA_ARGS__)                        \
    Base::operator=(std::move(rhs));                                           \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  /** Safe method to add an element to this vector. */                         \
  /** Does not invalidate, iterators, pointers or references. */               \
  /** Changes do not take effect until they are commited.*/                    \
  /** @param other element that should be added to the vector*/                \
  /** @return  index of the added element in `data_`. Will be bigger than*/    \
  /**          `size()` */                                                     \
  template <typename T = Backend>                                              \
  uint64_t DelayedPushBack(const Self<Scalar>& other) {                        \
    std::lock_guard<std::recursive_mutex> lock(Base::mutex_);                  \
    PushBackImpl(other);                                                       \
    return Base::TotalSize() - 1;                                              \
  }                                                                            \
                                                                               \
 protected:                                                                    \
  /** Equivalent to std::vector<> push_back - it adds the scalar values to */  \
  /** all data members */                                                      \
  void PushBackImpl(const MostDerived<Scalar>& other) override {               \
    BDM_SIM_OBJECT_PUSH_BACK_BODY(__VA_ARGS__);                                \
    Base::PushBackImpl(other);                                                 \
  }                                                                            \
                                                                               \
  /** Equivalent to std::vector<> push_back - it adds the scalar values to */  \
  /** all data members */                                                      \
  void PushBackImpl(const MostDerived<SoaRef>& other) override {               \
    BDM_SIM_OBJECT_PUSH_BACK_BODY(__VA_ARGS__);                                \
    Base::PushBackImpl(other);                                                 \
  }                                                                            \
  /** Swap element with last element and remove last element from each */      \
  /** data member */                                                           \
  void SwapAndPopBack(size_t index, size_t size) override {                    \
    BDM_SIM_OBJECT_SWAP_AND_POP_BACK_BODY(__VA_ARGS__);                        \
    Base::SwapAndPopBack(index, size);                                         \
  }                                                                            \
                                                                               \
  /** Remove last element from each data member */                             \
  void PopBack() override {                                                    \
    BDM_SIM_OBJECT_POP_BACK_BODY(__VA_ARGS__);                                 \
    Base::PopBack();                                                           \
  }                                                                            \
                                                                               \
  /** Cast `this` to the base class pointer (one level up) */                  \
  Base* UpCast() { return static_cast<Base*>(this); }                          \
                                                                               \
  /** Cast `this` to the base class pointer (one level up) */                  \
  const Base* UpCast() const { return static_cast<const Base*>(this); }        \
                                                                               \
 private:                                                                      \
  /** Cast `this` to the most derived type */                                  \
  /** Used to call the method of the subclass without virtual functions */     \
  /** e.g. `ThisMD()->Method()` */                                             \
  /** (CRTP - static polymorphism) */                                          \
  MostDerived<Backend>* ThisMD() {                                             \
    return static_cast<MostDerived<Backend>*>(this);                           \
  }                                                                            \
  const MostDerived<Backend>* ThisMD() const {                                 \
    return static_cast<MostDerived<Backend>*>(this);                           \
  }                                                                            \
                                                                               \
  BDM_ROOT_CLASS_DEF_OVERRIDE(class_name##Ext, class_version_id)

}  // namespace bdm

#endif  // SIMULATION_OBJECT_UTIL_H_
