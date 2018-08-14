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

/// Macro to define a new simulation object
/// @param sim_object
/// @param base_class
///
///     // Example usage to extend class Cell
///     BDM_SIM_OBJECT(MyCell, bdm::Cell) {
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
struct DerivedPlaceholder;

#define BDM_SIM_OBJECT(sim_object, base_class)                                \
  template <typename TCompileTimeParam,                                       \
            typename TDerived = DerivedPlaceholder>                           \
  class sim_object##Ext;                                                      \
                                                                              \
  using sim_object = sim_object##Ext < CompileTimeParam,                      \
        sim_object##Ext<CompileTimeParam, sim_object##Ext<CompileTimeParam>>; \
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
#define BDM_SIM_OBJECT_HEADER(class_name, class_version_id, ...)             \
 public:                                                                     \
  using Base = bdm::SimulationObjectExt<TCompileTimeParam, TDerived>;        \
                                                                             \
  /** reduce verbosity by defining a local alias */                          \
  using MostDerived = typename TDerived::template type<TCompileTimeParam, TDerived>;      \
  using MostDerivedSoPtr = SoPointer<class_name>;                            \
                                                                             \
  template <typename TResourceManager = ResourceManager<>>                   \
  SoHandle GetSoHandle() const {                                             \
    auto type_idx = TResourceManager::template GetTypeIndex<MostDerived>();  \
    return SoHandle(type_idx, Base::GetElementIdx());                        \
  }                                                                          \
                                                                             \
  explicit class_name(TRootIOCtor* io_ctor) {}                               \
  class_name(const class_name& other) = default;                             \
                                                                             \
  using Simulation_t = Simulation<TCompileTimeParam>;                        \
                                                                             \
  MostDerivedSoPtr GetSoPtr() {                                              \
    auto* rm = Simulation_t::GetActive()->GetResourceManager();              \
    auto* container = rm->template Get<MostDerived>();                       \
    return MostDerivedSoPtr(container, Base::GetElementIdx());               \
  }                                                                          \
                                                                             \
  void RemoveFromSimulation() {                                              \
    auto* rm = Simulation_t::GetActive()->GetResourceManager();              \
    auto container = rm->template Get<MostDerived>();                        \
    container->DelayedRemove(Base::GetElementIdx());                         \
  }                                                                          \
                                                                             \
  /** Returns the Scalar name of the container minus the "Ext"     */        \
  static const std::string GetScalarTypeName() {                             \
    static std::string kScalarType = #class_name;                            \
    return kScalarType.substr(0, kScalarType.size() - 3);                    \
  }                                                                          \
                                                                             \
  /** Executes the given function for all data members             */        \
  /**  Function could be a lambda in the following form:           */        \
  /**  `[](auto* data_member, const std::string& dm_name) { ... }` */        \
  template <typename TFunction>                                              \
  void ForEachDataMember(TFunction f) {                                      \
    BDM_SIM_OBJECT_FOREACHDM_BODY(__VA_ARGS__)                               \
    Base::ForEachDataMember(f);                                              \
  }                                                                          \
                                                                             \
  /** Executes the given function for the specified data members    */       \
  /** Function could be a lambda in the following form              */       \
  /** `[](auto* data_member, const std::string& dm_name) { ... }`   */       \
  template <typename TFunction>                                              \
  void ForEachDataMemberIn(std::set<std::string> dm_selector, TFunction f) { \
    BDM_SIM_OBJECT_FOREACHDMIN_BODY(__VA_ARGS__)                             \
    Base::ForEachDataMemberIn(dm_selector, f);                               \
  }                                                                          \
                                                                             \
 private:                                                                    \
  /** Cast `this` to the most derived type */                                \
  /** Used to call the method of the subclass without virtual functions */   \
  /** e.g. `ThisMD()->Method()` */                                           \
  /** (CRTP - static polymorphism) */                                        \
  MostDerived* ThisMD() { return static_cast<MostDerived*>(this); }          \
  const MostDerived* ThisMD() const {                                        \
    return static_cast<MostDerived*>(this);                                  \
  }                                                                          \
                                                                             \
  ClassDef(class_name, class_version_id)

}  // namespace bdm

#endif  // SIMULATION_OBJECT_UTIL_H_
