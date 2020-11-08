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

#ifndef CORE_AGENT_AGENT_H_
#define CORE_AGENT_AGENT_H_

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
#include "core/agent/agent_pointer.h"
#include "core/agent/agent_uid.h"
#include "core/force.h"
#include "core/util/macros.h"
#include "core/util/root.h"
#include "core/util/spinlock.h"

namespace bdm {

/// Macro to insert required boilerplate code into agent
/// @param   class_name scalar class name of the agent
/// @param   base_class scalar class name of the base agent
/// @param   class_version_id required for ROOT I/O (see ROOT BDM_CLASS_DEF
///          Macro).
///          Every time the layout of the class is changed, class_version_id
///          must be incremented by one. The class_version_id should be greater
///          or equal to 1.
/// @param  ...: List of all data members of this class
#define BDM_AGENT_HEADER(class_name, base_class, class_version_id)      \
 public:                                                                     \
  using Base = base_class;                                                   \
                                                                             \
  explicit class_name(TRootIOCtor* io_ctor) {}                               \
                                                                             \
  /** Create a new instance of this object using the default constructor. */ \
  Agent* GetInstance(const Event& event, Agent* other,               \
                         uint64_t new_oid = 0) const override {              \
    return new class_name(event, other, new_oid);                            \
  }                                                                          \
                                                                             \
  /** Create a copy of this object. */                                       \
  Agent* GetCopy() const override { return new class_name(*this); }      \
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
struct Behavior;

/// Contains code required by all agents
class Agent {
 public:
  Agent();

  Agent(const Event& event, Agent* other, uint64_t new_oid = 0);

  explicit Agent(TRootIOCtor* io_ctor);

  Agent(const Agent& other);

  virtual ~Agent();

  /// Create a new instance of this object using the default constructor.
  virtual Agent* GetInstance(const Event& event, Agent* other,
                                 uint64_t new_oid = 0) const = 0;

  /// Create a copy of this object.
  virtual Agent* GetCopy() const = 0;

  virtual const char* GetTypeName() const { return "Agent"; }

  virtual Shape GetShape() const = 0;

  /// Returns the data members that are required to visualize this simulation
  /// object.
  virtual std::set<std::string> GetRequiredVisDataMembers() const {
    return {"position_", "diameter_"};
  }

  virtual void RunDiscretization();

  void AssignNewUid();

  const AgentUid& GetUid() const;

  Spinlock* GetLock() { return &lock_; }

  /// If the thread-safety mechanism is set to user-specified this function
  /// will be called before the operations are executed for this simulation
  /// object.\n
  /// Subclasses define the critical region by adding the locks of all
  /// agents that must not be processed in parallel. \n
  /// Don't forget to add the lock of the current agent.\n
  /// \see `Param::thread_safety_mechanism`
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

  void DistributeRunDisplacementInfo();

  void UpdateRunDisplacement() {
    run_displacement_ = run_displacement_next_ts_;
    run_displacement_next_ts_ = false;
  }

  bool RunDisplacement() const { return run_displacement_; }

  /// Return agent pointer
  template <typename TAgent = Agent>
  AgentPointer<TAgent> GetAgentPtr() const {
    static_assert(!std::is_pointer<TAgent>::value,
                  "Cannot be of pointer type!");
    return AgentPointer<TAgent>(uid_);
  }

  // ---------------------------------------------------------------------------
  // Behaviors
  /// Add a behavior to this agent
  void AddBehavior(Behavior* behavior);

  /// Remove a behavior from this agent
  void RemoveBehavior(const Behavior* behavior);

  /// Execute all behaviorsq
  void RunBehaviors();

  /// Return all behaviors
  const InlineVector<Behavior*, 2>& GetAllBehaviors() const;
  // ---------------------------------------------------------------------------

  virtual Double3 CalculateDisplacement(const Force* force, double squared_radius, double dt) = 0;

  virtual void ApplyDisplacement(const Double3& displacement) = 0;

  virtual const Double3& GetPosition() const = 0;

  virtual void SetPosition(const Double3& pos) = 0;

  virtual double GetDiameter() const = 0;

  virtual void SetDiameter(double diameter) = 0;

  void RemoveFromSimulation() const;

  virtual void EventHandler(const Event& event, Agent* other1,
                            Agent* other2 = nullptr);

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
  AgentUid uid_;
  /// Grid box index
  uint32_t box_idx_ = 0;
  /// collection of behaviors which define the internal behavior
  InlineVector<Behavior*, 2> behaviors_;

 private:
  Spinlock lock_;  //!

  /// Helper variable used to support removal of behaviors while
  /// `RunBehaviors` iterates over them.
  uint32_t run_behavior_loop_idx_ = 0;

  bool run_displacement_ = true;                   //!
  bool run_displacement_for_all_next_ts_ = false;  //!
  mutable bool run_displacement_next_ts_ = true;   //!

  /// @brief Function to copy behaviors from one structure to another
  /// @param event event will be passed on to behavior to determine
  ///        whether it should be copied to destination
  /// @param src  source vector of behaviors
  /// @param dest destination vector of behaviors
  void CopyBehaviors(const Event& event, decltype(behaviors_)* dest);

  /// @brief Function to invoke the EventHandler of the behavior or remove
  ///                  it from `current`.
  /// Forwards the event handler call to each behaviors of the triggered
  /// agent and removes behaviors if they are flagged.
  void BehaviorEventHandler(const Event& event,
                                 decltype(behaviors_)* other1,
                                 decltype(behaviors_)* other2);

  BDM_CLASS_DEF(Agent, 1)
};

}  // namespace bdm

#endif  // CORE_AGENT_AGENT_H_
