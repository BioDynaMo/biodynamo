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
#include "core/sim_object/so_pointer.h"
#include "core/sim_object/so_uid.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/macros.h"
#include "core/util/root.h"
#include "core/util/type.h"

namespace bdm {

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
  using Base = base_class;                   \
                                                                               \
  /** reduce verbosity by defining a local alias */                            \
  using Base::kIdx;                                                            \
                                                                               \
  /** Templated type alias to get a `SoPointer` for the given external type */ \
  template <typename T>                                                        \
  using ToSoPtr = SoPointer<ToSimBackend<T>, SimBackend>;                      \
                                                                               \
  /** friend event handler to give it access to private members */             \
  template <typename TEvent, typename TFirst, typename... TRemaining>          \
  friend struct EventHandler;                                                  \
                                                                               \
  static const std::string GetScalarTypeName() { return #class_name; }         \
                                                                               \
  explicit class_name(TRootIOCtor *io_ctor) {}                            \
  class_name(const class_name &other) = default;                     \
                                                                               \
  /** Executes the given function for all data members             */          \
  /**  Function could be a lambda in the following form:           */          \
  /**  `[](auto* data_member, const std::string& dm_name) { ... }` */          \
  template <typename Function>                           \
  void ForEachDataMember(Function f) {   \
    BDM_SIM_OBJECT_FOREACHDM_BODY(__VA_ARGS__)                                 \
    Base::ForEachDataMember(f);                                                \
  }                                                                            \
                                                                               \
  /** Executes the given function for the specified data members    */         \
  /** Function could be a lambda in the following form              */         \
  /** `[](auto* data_member, const std::string& dm_name) { ... }`   */         \
  template <typename Function>                           \
  void ForEachDataMemberIn(              \
      std::set<std::string> dm_selector, Function f) {                         \
    BDM_SIM_OBJECT_FOREACHDMIN_BODY(__VA_ARGS__)                               \
    Base::ForEachDataMemberIn(dm_selector, f);                                 \
  }                                                                            \
                                                                               \
 protected:                                                                    \
  /** Cast `this` to the base class pointer (one level up) */                  \
  Base *UpCast() { return static_cast<Base *>(this); }                         \
                                                                               \
  /** Cast `this` to the base class pointer (one level up) */                  \
  const Base *UpCast() const { return static_cast<const Base *>(this); }       \
                                                                               \
  BDM_ROOT_CLASS_DEF_OVERRIDE(class_name, class_version_id)


/// Contains code required by all simulation objects
class SimObjectExt {
  BDM_SIM_OBJECT_HEADER(SimObject, SimObjectBase, 1, uid_, box_idx_,
                        biology_modules_, run_bm_loop_idx_, numa_node_);

 public:
  SimObjectExt() : Base() { uid_ = SoUidGenerator::Get()->NewSoUid(); }

  template <typename TEvent, typename TOther>
  SimObjectExt(const TEvent &event, TOther *other, uint64_t new_oid = 0) {
    uid_ = SoUidGenerator::Get()->NewSoUid();
    box_idx_ = other->GetBoxIdx();
    // biology modules
    auto &other_bms = other->biology_modules_;
    // copy biology_modules_ to me
    CopyBiologyModules(event, &other_bms, &biology_modules_);
  }

  virtual ~SimObjectExt() {}

  // /// This function determines if the type of this simulation object is the same
  // /// as `TSo` without taking the backend into account.
  // /// @tparam TSo Simulation object type with any backend
  // template <typename TSo>
  // static constexpr bool IsSoType() {
  //   using TSoScalar = typename TSo::template Self<Scalar>;
  //   return std::is_same<MostDerived<Scalar>, TSoScalar>::value;
  // }
  //
  // /// This function determines if the type of this simulation object is the same
  // /// as `object` without taking the backend into account.
  // /// @param object simulation object can have any backend
  // template <typename TSo>
  // static constexpr bool IsSoType(const TSo *object) {
  //   using Type = std::decay_t<std::remove_pointer_t<decltype(object)>>;
  //   using ScalarType = typename Type::template Self<Scalar>;
  //   return std::is_same<MostDerived<Scalar>, ScalarType>::value;
  // }
  //
  // /// Casts this to a simulation object of type `TSo` with the current `Backend`
  // /// This function is used to simulate if constexpr functionality and won't be
  // /// needed after we swith to C++17
  // /// @tparam TSo target simulaton object type with any backend
  // template <typename TSo>
  // constexpr auto &&ReinterpretCast() {
  //   using TargetType = typename TSo::template Self<Backend>;
  //   return reinterpret_cast<TargetType &&>(*this);
  // }
  //
  // /// Casts this to a simulation object of type `TSo` with the current `Backend`
  // /// This function is used to simulate if constexpr functionality and won't be
  // /// needed after we swith to C++17
  // /// @tparam TSo target simulaton object type with any backend
  // template <typename TSo>
  // constexpr const auto *ReinterpretCast() const {
  //   using TargetType = typename TSo::template Self<Backend>;
  //   return reinterpret_cast<const TargetType *>(this);
  // }
  //
  // /// Casts this to a simulation object of type `TSo` with the current `Backend`
  // /// This function is used to simulate if constexpr functionality and won't be
  // /// needed after we swith to C++17
  // /// @tparam TSo target simulaton object type with any backend
  // template <typename TSo>
  // constexpr auto &&ReinterpretCast(const TSo *object) {
  //   using TargetType = typename TSo::template Self<Backend>;
  //   return reinterpret_cast<TargetType &&>(*this);
  // }

  void RunDiscretization() {}

  SoUid GetUid() const { return uid_; }

  uint32_t GetBoxIdx() const { return box_idx_; }

  void SetBoxIdx(uint32_t idx) { box_idx_ = idx; }

  /// Return simulation object pointer
  MostDerivedSoPtr GetSoPtr() const { return MostDerivedSoPtr(uid_); }

  void SetNumaNode(typename SoHandle::NumaNode_t numa_node) {
    numa_node_ = numa_node;
  }

  // ---------------------------------------------------------------------------
  // Biology modules
  // using BiologyModules =
  //     typename TCompileTimeParam::template CTMap<MostDerivedScalar,
  //                                                0>::BiologyModules::Variant_t;
  //
  // /// Add a biology module to this cell
  // /// @tparam TBiologyModule type of the biology module. Must be in the set of
  // ///         types specified in `BiologyModules`
  // template <typename TBiologyModule>
  // void AddBiologyModule(TBiologyModule &&module) {
  //   biology_modules_.emplace_back(module);
  // }
  //
  // /// Remove a biology module from this cell
  // template <typename TBiologyModule>
  // void RemoveBiologyModule(const TBiologyModule *remove_module) {
  //   for (unsigned int i = 0; i < biology_modules_.size(); i++) {
  //     const TBiologyModule *module =
  //         get_if<TBiologyModule>(&biology_modules_[i]);
  //     if (module == remove_module) {
  //       biology_modules_.erase(biology_modules_.begin() + i);
  //       // if remove_module was before or at the current run_bm_loop_idx_[kidx],
  //       // correct it by subtracting one.
  //       run_bm_loop_idx_ -= i > run_bm_loop_idx_ ? 0 : 1;
  //     }
  //   }
  // }
  //
  // /// Execute all biology modulesq
  // void RunBiologyModules() {
  //   RunVisitor<MostDerived<Backend>> visitor(
  //       static_cast<MostDerived<Backend> *>(this));
  //   for (run_bm_loop_idx_ = 0;
  //        run_bm_loop_idx_ < biology_modules_.size();
  //        ++run_bm_loop_idx_) {
  //     auto &module = biology_modules_[run_bm_loop_idx_];
  //     visit(visitor, module);
  //   }
  // }
  //
  // /// Get all biology modules of this cell that match the given type.
  // /// @tparam TBiologyModule  type of the biology module
  // template <typename TBiologyModule>
  // std::vector<const TBiologyModule *> GetBiologyModules() const {
  //   std::vector<const TBiologyModule *> modules;
  //   for (unsigned int i = 0; i < biology_modules_.size(); i++) {
  //     const TBiologyModule *module =
  //         get_if<TBiologyModule>(&biology_modules_[i]);
  //     if (module != nullptr) {
  //       modules.push_back(module);
  //     }
  //   }
  //   return modules;
  // }
  //
  // /// Return all biology modules
  // const auto &GetAllBiologyModules() const { return biology_modules_; }
  // ---------------------------------------------------------------------------

  void RemoveFromSimulation() const {
    Simulation::GetActive()->GetExecutionContext()->RemoveFromSimulation(
        uid_);
  }

  template <typename TEvent, typename TOther>
  void EventHandler(const TEvent &event, TOther *other) {
    // call event handler for biology modules
    auto *other_bms = &(other->biology_modules_);
    BiologyModuleEventHandler(event, &(biology_modules_), other_bms);
  }

  template <typename TEvent, typename TLeft, typename TRight>
  void EventHandler(const TEvent &event, TLeft *left, TRight *right) {
    // call event handler for biology modules
    auto *left_bms = &(left->biology_modules_[left->kIdx]);
    auto *right_bms = &(right->biology_modules_[right->kIdx]);
    BiologyModuleEventHandler(event, &(biology_modules_), left_bms,
                              right_bms);
  }

 protected:
  /// unique id
  SoUid uid_;
  /// Grid box index
  uint32_t box_idx_;
  /// collection of biology modules which define the internal behavior
  // std::vector<BiologyModules> biology_modules_;

 private:
  /// Helper variable used to support removal of biology modules while
  /// `RunBiologyModules` iterates over them.
  /// Due to problems with restoring this member for the SOA data layout, it is
  /// not ignored for ROOT I/O.
  uint32_t run_bm_loop_idx_ = 0;

  /// This data member holds information on which NUMA node this sim object is
  /// stored.
  typename SoHandle::NumaNode_t numa_node_;

  // /// @brief Function to copy biology modules from one structure to another
  // /// @param event event will be passed on to biology module to determine
  // ///        whether it should be copied to destination
  // /// @param src  source vector of biology modules
  // /// @param dest destination vector of biology modules
  // /// @tparam TBiologyModules std::vector<Variant<[list of biology modules]>>
  // template <typename TEvent, typename TSrcBms, typename TDestBms>
  // void CopyBiologyModules(const TEvent &event, TSrcBms *src, TDestBms *dest) {
  //   auto copy = [&](auto &bm) {
  //     if (bm.Copy(event.kEventId)) {
  //       raw_type<decltype(bm)> new_bm(event, &bm);
  //       dest->emplace_back(std::move(new_bm));
  //     }
  //   };
  //   for (auto &module : *src) {
  //     visit(copy, module);
  //   }
  // }
  //
  // /// @brief Function to invoke the EventHandler of the biology module or remove
  // ///                  it from `current`.
  // /// Forwards the event handler call to each biology modules of the triggered
  // /// simulation object and removes biology modules if they are flagged.
  // template <typename TEvent, typename TBiologyModules1,
  //           typename... TBiologyModules>
  // void BiologyModuleEventHandler(const TEvent &event, TBiologyModules1 *current,
  //                                TBiologyModules *... bms) {
  //   // call event handler for biology modules
  //   uint64_t cnt = 0;
  //   auto call_bm_event_handler = [&](auto &bm) {
  //     using BiologyModuleType = raw_type<decltype(bm)>;
  //
  //     /// return nullptr of condition is false or pointer to object in variant
  //     auto extract = [](bool condition, auto *variant) -> BiologyModuleType * {
  //       if (condition) {
  //         return get_if<BiologyModuleType>(variant);
  //       }
  //       return nullptr;
  //     };
  //
  //     if (!bm.Remove(event.kEventId)) {
  //       bool copy = bm.Copy(event.kEventId);
  //       bm.EventHandler(event, extract(copy, &((*bms)[cnt]))...);
  //       cnt += copy ? 1 : 0;
  //     }
  //   };
  //   for (auto &el : *current) {
  //     visit(call_bm_event_handler, el);
  //   }
  //
  //   // remove biology modules from current
  //   bool remove;
  //   auto remove_from_current = [&](auto &bm) {
  //     remove = bm.Remove(event.kEventId);
  //   };
  //   for (auto it = current->begin(); it != current->end();) {
  //     remove = false;
  //     visit(remove_from_current, *it);
  //     if (remove) {
  //       it = current->erase(it);
  //     } else {
  //       ++it;
  //     }
  //   }
  // }
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SIM_OBJECT_H_
