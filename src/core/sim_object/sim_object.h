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
  /** Create a new instance of this object using the default constructor. */   \
  SimObject* GetInstance() const override { return new class_name(); }\
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

  virtual ~SimObjectExt() {
    for(auto* el : biology_modules_) {
      delete el;
    }
  }

  /// Create a new instance of this object using the default constructor.
  virtual SimObject* GetInstance() const { return new SimObject(); }

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
  SoPointer GetSoPtr() const { return SoPointer(uid_); }

  void SetNumaNode(typename SoHandle::NumaNode_t numa_node) {
    numa_node_ = numa_node;
  }

  // ---------------------------------------------------------------------------
  // Biology modules
  using BiologyModules =
      typename TCompileTimeParam::template CTMap<MostDerivedScalar,
                                                 0>::BiologyModules::Variant_t;

  /// Add a biology module to this sim object
  /// @tparam TBiologyModule type of the biology module. Must be in the set of
  ///         types specified in `BiologyModules`
  void AddBiologyModule(BaseBiologyModule* module) {
    biology_modules_.emplace_back(module);
  }

  /// Remove a biology module from this sim object
  void RemoveBiologyModule(const BaseBiologyModule *remove_module) {
    for (unsigned int i = 0; i < biology_modules_.size(); i++) {
      if (biology_modules_[i] == remove_module) {
        biology_modules_.erase(biology_modules_.begin() + i);
        // if remove_module was before or at the current run_bm_loop_idx_,
        // correct it by subtracting one.
        run_bm_loop_idx_ -= i > run_bm_loop_idx_ ? 0 : 1;
      }
    }
  }

  /// Execute all biology modulesq
  void RunBiologyModules() {
    for (auto* bm : biology_modules_) {
      bm->Run(this);
    }
  }

  /// Get all biology modules of this cell that match the given type.
  /// @tparam TBiologyModule  type of the biology module
  template <typename TBiologyModule>
  std::vector<const BaseBiologyModule*> GetBiologyModules() const {
    for (unsigned int i = 0; i < biology_modules_.size(); i++) {
      if (dynamic_cast<const TBiologyModule*>(biology_modules_[i]) != nullptr) {
        modules.push_back(biology_modules_[i]);
      }
    }
    return modules;
  }

  /// Return all biology modules
  const auto &GetAllBiologyModules() const { return biology_modules_; }
  // ---------------------------------------------------------------------------

  void RemoveFromSimulation() const {
    Simulation::GetActive()->GetExecutionContext()->RemoveFromSimulation(uid_);
  }

  virtual void EventConstructor(const Event& event, SimObject* other, uint64_t new_oid = 0) {
    box_idx_ = other->GetBoxIdx();
    // biology modules
    auto* other_bms = &(other->biology_modules_);
    // copy biology_modules_ to me
    CopyBiologyModules(event, other_bms);
  }

  virtual void EventHandler(const Event &event, SimObject *other1, SimObject* other2 = nullptr) {
    // call event handler for biology modules
    auto *left_bms = &(other1->biology_modules_);
    auto *right_bms = &(other2->biology_modules_);
    BiologyModuleEventHandler(event, left_bms, right_bms);
  }

 protected:
  /// unique id
  SoUid uid_;
  /// Grid box index
  uint32_t box_idx_;
  /// collection of biology modules which define the internal behavior
  std::vector<BaseBiologyModules*> biology_modules_;

 private:
  /// Helper variable used to support removal of biology modules while
  /// `RunBiologyModules` iterates over them.
  /// Due to problems with restoring this member for the SOA data layout, it is
  /// not ignored for ROOT I/O.
  uint32_t run_bm_loop_idx_ = 0;

  /// This data member holds information on which NUMA node this sim object is
  /// stored.
  typename SoHandle::NumaNode_t numa_node_;

  /// @brief Function to copy biology modules from one structure to another
  /// @param event event will be passed on to biology module to determine
  ///        whether it should be copied to destination
  /// @param src  source vector of biology modules
  /// @param dest destination vector of biology modules
  /// @tparam TBiologyModules std::vector<Variant<[list of biology modules]>>
  void CopyBiologyModules(const Event& event, decltype(biology_modules_) *dest) {
    for (auto* bm : biology_modules_) {
      if(bm->Copy(event.kEventId)) {
        auto* new_bm = bm->GetInstance();
        new_bm->EventConstructor(event, bm);
        dest->push_back(new_bm);
      }
    }
  }

  /// @brief Function to invoke the EventHandler of the biology module or remove
  ///                  it from `current`.
  /// Forwards the event handler call to each biology modules of the triggered
  /// simulation object and removes biology modules if they are flagged.
  void BiologyModuleEventHandler(const Event &event, decltype(biology_modules_) *other1,
                                 decltype(biology_modules_) *other2) {
    // call event handler for biology modules
    uint64_t cnt = 0;
    for(auto* bm : biology_modules_) {
      bool copy = bm->Copy(event.kEventId);
      if (!bm->Remove(event.kEventId)) {
        if(copy) {
          auto* other1_bm = other1 != nullptr ? (*other1)[cnt] : nullptr;
          auto* other2_bm = other2 != nullptr ? (*other2)[cnt] : nullptr;
          bm->EventHandler(event, other1_bm, other2_bm);
        } else {
          bm->EventHandler(event, nullptr, nullptr);
        }
      }
      cnt += copy ? 1 : 0;
    }

    // remove biology modules from current
    for (auto it = biology_modules_.begin(); it != biology_modules_.end();) {
      auto* bm = *it;
      if (bm->Remove(event.kEventId)) {
        delete *it;
        it = current->erase(it);
      } else {
        ++it;
      }
    }
  }
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SIM_OBJECT_H_
