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

#include "core/container/inline_vector.h"
#include "core/container/math_array.h"
#include "core/shape.h"
#include "core/sim_object/so_pointer.h"
#include "core/sim_object/so_uid.h"
#include "core/sim_object/so_visitor.h"
#include "core/util/macros.h"
#include "core/util/root.h"
#include "core/util/spinlock.h"

namespace bdm {

/// Macro to insert required boilerplate code into simulation object
/// @param   class_name scalar class name of the simulation object
/// @param   base_class scalar class name of the base simulation object
/// @param   class_version_id required for ROOT I/O (see ROOT BDM_CLASS_DEF
///          Macro).
///          Every time the layout of the class is changed, class_version_id
///          must be incremented by one. The class_version_id should be greater
///          or equal to 1.
/// @param  ...: List of all data members of this class
#define BDM_SIM_OBJECT_HEADER(class_name, base_class, class_version_id)      \
 public:                                                                     \
  using Base = base_class;                                                   \
                                                                             \
  explicit class_name(TRootIOCtor* io_ctor) {}                               \
                                                                             \
  /** Create a new instance of this object using the default constructor. */ \
  SimObject* GetInstance(const Event& event, SimObject* other,               \
                         uint64_t new_oid = 0) const override {              \
    return new class_name(event, other, new_oid);                            \
  }                                                                          \
                                                                             \
  /** Create a copy of this object. */                                       \
  SimObject* GetCopy() const override { return new class_name(*this); }      \
                                                                             \
  const char* GetTypeName() const override { return #class_name; }           \
                                                                             \
 protected:                                                                  \
  /** Cast `this` to the base class pointer (one level up) */                \
  Base* UpCast() { return static_cast<Base*>(this); }                        \
                                                                             \
  /** Cast `this` to the base class pointer (one level up) */                \
  const Base* UpCast() const { return static_cast<const Base*>(this); }      \
                                                                             \
  BDM_CLASS_DEF_OVERRIDE(class_name, class_version_id)

// -----------------------------------------------------------------------------

struct Event;
struct BaseBiologyModule;

/// Contains code required by all simulation objects
class SimObject {
 public:
  SimObject();

  SimObject(const Event& event, SimObject* other, uint64_t new_oid = 0);

  explicit SimObject(TRootIOCtor* io_ctor);

  SimObject(const SimObject& other);

  virtual ~SimObject();

  /// Create a new instance of this object using the default constructor.
  virtual SimObject* GetInstance(const Event& event, SimObject* other,
                                 uint64_t new_oid = 0) const = 0;

  /// Create a copy of this object.
  virtual SimObject* GetCopy() const = 0;

  virtual const char* GetTypeName() const { return "SimObject"; }

  virtual Shape GetShape() const = 0;

  /// Returns the data members that are required to visualize this simulation
  /// object.
  virtual std::set<std::string> GetRequiredVisDataMembers() const {
    return {"position_", "diameter_"};
  }

  virtual void RunDiscretization();

  void AssignNewUid();

  const SoUid& GetUid() const;

  Spinlock* GetLock() { return &lock_; }

  /// If the thread-safety mechanism is set to user-specified this function
  /// will be called before the operations are executed for this simulation
  /// object.\n
  /// Subclasses define the critical region by adding the locks of all
  /// simulation objects that must not be processed in parallel. \n
  /// Don't forget to add the lock of the current simulation object.\n
  /// \see `Param::thread_safety_mechanism_`
  virtual void CriticalRegion(std::vector<Spinlock*>* locks) {}

  uint32_t GetBoxIdx() const;

  void SetBoxIdx(uint32_t idx);

  void SetRunDisplacementNextTimestep(bool run) const {
    run_displacement_next_ts_ = run;
  }

  bool GetRunDisplacementForAllNextTs() const {
    return run_displacement_for_all_next_ts_;
  }

  void SetRunDisplacementForAllNextTs(bool value = true) {
    run_displacement_for_all_next_ts_ = value;
  }

  void DistributeRunDisplacementInfo(bool detect_static_sim_objects);

  void UpdateRunDisplacement() {
    run_displacement_ = run_displacement_next_ts_;
    run_displacement_next_ts_ = false;
  }

  bool RunDisplacement() const { return run_displacement_; }

  /// Return simulation object pointer
  template <typename TSimObject = SimObject>
  SoPointer<TSimObject> GetSoPtr() const {
    static_assert(!std::is_pointer<TSimObject>::value,
                  "Cannot be of pointer type!");
    return SoPointer<TSimObject>(uid_);
  }

  // ---------------------------------------------------------------------------
  // Biology modules
  /// Add a biology module to this sim object
  void AddBiologyModule(BaseBiologyModule* module);

  /// Remove a biology module from this sim object
  void RemoveBiologyModule(const BaseBiologyModule* remove_module);

  /// Execute all biology modulesq
  void RunBiologyModules();

  /// Return all biology modules
  const InlineVector<BaseBiologyModule*, 2>& GetAllBiologyModules() const;
  // ---------------------------------------------------------------------------

  virtual Double3 CalculateDisplacement(double squared_radius, double dt) = 0;

  virtual void ApplyDisplacement(const Double3& displacement) = 0;

  virtual const Double3& GetPosition() const = 0;

  virtual void SetPosition(const Double3& pos) = 0;

  virtual double GetDiameter() const = 0;

  virtual void SetDiameter(double diameter) = 0;

  void RemoveFromSimulation() const;

  virtual void EventHandler(const Event& event, SimObject* other1,
                            SimObject* other2 = nullptr);

  void* operator new(size_t size) {  // NOLINT
    auto* mem_mgr = Simulation::GetActive()->GetMemoryManager();
    if (mem_mgr) {
      return mem_mgr->New(size);
    } else {
      return malloc(size);
    }
  }

  void operator delete(void* p) {  // NOLINT
    auto* mem_mgr = Simulation::GetActive()->GetMemoryManager();
    if (mem_mgr) {
      mem_mgr->Delete(p);
    } else {
      free(p);
    }
  }

 protected:
  /// unique id
  SoUid uid_;
  /// Grid box index
  uint32_t box_idx_ = 0;
  /// collection of biology modules which define the internal behavior
  InlineVector<BaseBiologyModule*, 2> biology_modules_;

 private:
  Spinlock lock_;  //!

  /// Helper variable used to support removal of biology modules while
  /// `RunBiologyModules` iterates over them.
  uint32_t run_bm_loop_idx_ = 0;

  bool run_displacement_ = true;                   //!
  bool run_displacement_for_all_next_ts_ = false;  //!
  mutable bool run_displacement_next_ts_ = true;   //!

  /// @brief Function to copy biology modules from one structure to another
  /// @param event event will be passed on to biology module to determine
  ///        whether it should be copied to destination
  /// @param src  source vector of biology modules
  /// @param dest destination vector of biology modules
  void CopyBiologyModules(const Event& event, decltype(biology_modules_)* dest);

  /// @brief Function to invoke the EventHandler of the biology module or remove
  ///                  it from `current`.
  /// Forwards the event handler call to each biology modules of the triggered
  /// simulation object and removes biology modules if they are flagged.
  void BiologyModuleEventHandler(const Event& event,
                                 decltype(biology_modules_)* other1,
                                 decltype(biology_modules_)* other2);

  BDM_CLASS_DEF(SimObject, 1)
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SIM_OBJECT_H_
