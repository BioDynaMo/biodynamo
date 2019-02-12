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

#include "core/sim_object/so_pointer.h"
#include "core/sim_object/so_uid.h"
#include "core/util/root.h"
#include "core/util/macros.h"
#include "core/shape.h"

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
  static const std::string GetScalarTypeName() { return #class_name; }         \
                                                                               \
  explicit class_name(TRootIOCtor *io_ctor) {}                            \
  \
  /** Create a new instance of this object using the default constructor. */   \
  SimObject* GetInstance() const override { return new class_name(); }\
  \
  /** Create a copy of this object. */   \
  SimObject* GetCopy() const override { return new class_name(*this); }\
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
  BDM_CLASS_DEF_OVERRIDE(class_name, class_version_id)

// -----------------------------------------------------------------------------

class Event;
class BaseBiologyModule;

/// Contains code required by all simulation objects
class SimObject {
 public:
  SimObject();

  virtual ~SimObject();

  // ---------------------------------------------------------------------------
  static const std::string GetScalarTypeName();

  explicit SimObject(TRootIOCtor *io_ctor);
  SimObject(const SimObject &other);

  /// Executes the given function for all data members
  ///  Function could be a lambda in the following form:
  ///  `[](auto* data_member, const std::string& dm_name) { ... }`
  template <typename Function>
  void ForEachDataMember(Function f) {
    BDM_SIM_OBJECT_FOREACHDM_BODY(uid_, box_idx_,
                          biology_modules_, run_bm_loop_idx_)
  }

  /// Executes the given function for the specified data members
  /// Function could be a lambda in the following form
  /// `[](auto* data_member, const std::string& dm_name) { ... }`
  template <typename Function>
  void ForEachDataMemberIn(
      std::set<std::string> dm_selector, Function f) {
    BDM_SIM_OBJECT_FOREACHDMIN_BODY(uid_, box_idx_,
                          biology_modules_, run_bm_loop_idx_)
  }

  // ---------------------------------------------------------------------------

  /// Create a new instance of this object using the default constructor.
  virtual SimObject* GetInstance() const = 0;

  /// Create a copy of this object.
  virtual SimObject* GetCopy() const = 0;

  template <typename T> T* As() { return dynamic_cast<T*>(this); }
  template <typename T>
  const T* As() const { return dynamic_cast<const T*>(this); }

  virtual Shape GetShape() const = 0;

  virtual void RunDiscretization();

  SoUid GetUid() const;

  uint32_t GetBoxIdx() const;

  void SetBoxIdx(uint32_t idx);

  /// Return simulation object pointer
  template <typename TSimObject = SimObject>
  SoPointer<TSimObject> GetSoPtr() const { return SoPointer<TSimObject>(uid_); }

  // ---------------------------------------------------------------------------
  // Biology modules
  /// Add a biology module to this sim object
  /// @tparam TBiologyModule type of the biology module. Must be in the set of
  ///         types specified in `BiologyModules`
  void AddBiologyModule(BaseBiologyModule* module);

  /// Remove a biology module from this sim object
  void RemoveBiologyModule(const BaseBiologyModule *remove_module);

  /// Execute all biology modulesq
  void RunBiologyModules();

  /// Return all biology modules
  const std::vector<BaseBiologyModule*> &GetAllBiologyModules() const;
  // ---------------------------------------------------------------------------

  virtual std::array<double, 3> CalculateDisplacement(double squared_radius) = 0;

  virtual void ApplyDisplacement(const std::array<double, 3>& displacement) = 0;

  virtual const std::array<double, 3> GetPosition() const = 0;

  virtual void SetPosition(const std::array<double, 3>& pos) = 0;

  virtual double GetDiameter() const = 0;

  virtual void SetDiameter(double diameter) = 0;

  void RemoveFromSimulation() const;

  virtual void EventConstructor(const Event& event, SimObject* other, uint64_t new_oid = 0);

  virtual void EventHandler(const Event &event, SimObject *other1, SimObject* other2 = nullptr);

 protected:
  /// unique id
  SoUid uid_;
  /// Grid box index
  uint32_t box_idx_;
  /// collection of biology modules which define the internal behavior
  std::vector<BaseBiologyModule*> biology_modules_;

 private:
  /// Helper variable used to support removal of biology modules while
  /// `RunBiologyModules` iterates over them.
  /// Due to problems with restoring this member for the SOA data layout, it is
  /// not ignored for ROOT I/O.
  uint32_t run_bm_loop_idx_ = 0;

  /// @brief Function to copy biology modules from one structure to another
  /// @param event event will be passed on to biology module to determine
  ///        whether it should be copied to destination
  /// @param src  source vector of biology modules
  /// @param dest destination vector of biology modules
  /// @tparam TBiologyModules std::vector<Variant<[list of biology modules]>>
  void CopyBiologyModules(const Event& event, decltype(biology_modules_) *dest);

  /// @brief Function to invoke the EventHandler of the biology module or remove
  ///                  it from `current`.
  /// Forwards the event handler call to each biology modules of the triggered
  /// simulation object and removes biology modules if they are flagged.
  void BiologyModuleEventHandler(const Event &event, decltype(biology_modules_) *other1,
                                 decltype(biology_modules_) *other2);

  BDM_CLASS_DEF(SimObject, 1)
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SIM_OBJECT_H_
