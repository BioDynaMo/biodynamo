// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
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

#include "core/agent/agent_pointer.h"
#include "core/agent/agent_uid.h"
#include "core/agent/new_agent_event.h"
#include "core/container/inline_vector.h"
#include "core/container/math_array.h"
#include "core/interaction_force.h"
#include "core/shape.h"
#include "core/util/macros.h"
#include "core/util/root.h"
#include "core/util/spinlock.h"
#include "core/util/type.h"

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
#define BDM_AGENT_HEADER(class_name, base_class, class_version_id)           \
 public:                                                                     \
  using Base = base_class;                                                   \
                                                                             \
  explicit class_name(TRootIOCtor* io_ctor) {}                               \
                                                                             \
  /** Create a new instance of this object using the default constructor. */ \
  Agent* New() const override { return new class_name(); }                   \
  /** Create a new instance of this object using the copy constructor. */    \
  Agent* NewCopy() const override { return new class_name(*this); }          \
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

class Behavior;

/// Contains code required by all agents
class Agent {
 public:
  Agent();

  explicit Agent(TRootIOCtor* io_ctor);

  Agent(const Agent& other);

  virtual ~Agent();

  /// Create a new instance of this object using the default constructor.
  virtual Agent* New() const = 0;

  /// Create a copy of this object.
  virtual Agent* NewCopy() const = 0;

  /// This method is called to initialize new agents that are created
  /// during a NewAgentEvent. Override this method to initialize attributes of
  /// your own Agent subclasses.
  /// NB: Don't forget to call the implementation of the base class first.
  /// `Base::Initialize(event);`
  /// Failing to do so will result in errors.
  virtual void Initialize(const NewAgentEvent& event);

  /// This method is called to update the existing agent at the end of
  /// a NewAgentEvent. Override this method to update attributes of
  /// your own Agent subclasses.
  /// NB: Don't forget to call the implementation of the base class first.
  /// `Base::Update(event);`
  /// Failing to do so will result in errors.
  virtual void Update(const NewAgentEvent& event);

  /// This method creates a new agent for each entry in prototypes.\n
  /// The prototypes list defines the type of the new agent.
  /// This function calls `prototype->New()` internally.
  /// New agents are automatically added to the execution context right after
  /// they are initialized.
  /// This function sets the attributes `NewAgentEvent::existing_agent`
  /// and `NewAgentEvent::new_agents` to their correct values.
  /// Lastly, this function calls  `this->Update(event)`\n
  /// The newly created and initialized agents can be found in
  /// `event.new_agents`.
  void CreateNewAgents(const NewAgentEvent& event,
                       const std::initializer_list<Agent*>& prototypes) {
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    event.existing_agent = this;
    for (auto* p : prototypes) {
      auto* new_agent = p->New();
      new_agent->Initialize(event);
      event.new_agents.push_back(new_agent);
      ctxt->AddAgent(new_agent);
    }
    Update(event);
  }

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
  /// will be called before the operations are executed for this agent.\n
  /// Subclasses define the critical region by adding the uids of all
  /// agents that must not be processed in parallel. \n
  /// Don't forget to add the uid of the current agent.\n
  /// \see `Param::thread_safety_mechanism`
  virtual void CriticalRegion(std::vector<AgentPointer<Agent>>* uids) {}

  uint32_t GetBoxIdx() const;

  void SetBoxIdx(uint32_t idx);

  void SetStaticnessNextTimestep(bool value) const {
    is_static_next_ts_ = value;
  }

  bool GetPropagateStaticness() const {
    return propagate_staticness_neighborhood_;
  }

  void SetPropagateStaticness(bool value = true) {
    propagate_staticness_neighborhood_ = value;
  }

  /// If the agent is not static, a call to this method
  /// sets all neighbors to 'not static'. \n
  /// This method will be called twice:
  /// 1) At the end of the iteration using an agent op (`beginning=false`).\n
  /// 2) At the beginning of the iteration as pre_scheduled op
  /// (`beginning=true`).\n
  ///
  /// 1 is faster because it can use cached neighbors, but it might miss some
  ///   conditions. (Neighbors modifying an agent after PropagateStaticness
  ///   has been called. Agent which is larger than the largest agent.)
  /// 2 is slower but able to process all conditions.
  /// Therefore, we use 1 whenever possible and 2 for the remaining conditions.
  void PropagateStaticness(bool beginning = false);

  void UpdateStaticness();

  bool IsStatic() const { return is_static_; }

  /// Return agent pointer
  template <typename TAgent = Agent>
  AgentPointer<TAgent> GetAgentPtr() const {
    static_assert(!std::is_pointer<TAgent>::value,
                  "Cannot be of pointer type!");
    return AgentPointer<TAgent>(const_cast<Agent*>(this));
  }

  // ---------------------------------------------------------------------------
  // Behaviors
  /// Add a behavior to this agent
  void AddBehavior(Behavior* behavior);

  /// Remove a behavior from this agent
  /// The parameter `behavior` will be deleted if the instance is  stored in
  /// this agent and must not be used after the call to `RemoveBehavior`.
  void RemoveBehavior(const Behavior* behavior);

  /// Execute all behaviorsq
  void RunBehaviors();

  /// Return all behaviors
  const InlineVector<Behavior*, 2>& GetAllBehaviors() const;
  // ---------------------------------------------------------------------------

  virtual Double3 CalculateDisplacement(const InteractionForce* force,
                                        double squared_radius, double dt) = 0;

  virtual void ApplyDisplacement(const Double3& displacement) = 0;

  virtual const Double3& GetPosition() const = 0;

  virtual void SetPosition(const Double3& pos) = 0;

  virtual double GetDiameter() const = 0;

  virtual void SetDiameter(double diameter) = 0;

  void RemoveFromSimulation() const;

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
  uint32_t box_idx_ = std::numeric_limits<uint32_t>::max();
  /// collection of behaviors which define the internal behavior
  InlineVector<Behavior*, 2> behaviors_;

 private:
  Spinlock lock_;  //!

  /// Helper variable used to support removal of behaviors while
  /// `RunBehaviors` iterates over them.
  uint16_t run_behavior_loop_idx_ = 0;

  /// If an agent is static, we should not compute the mechanical forces
  bool is_static_ = false;  //!
  /// If an agent becomes non-static (i.e. it moved or grew), we should set this
  /// flag to true to also compute mechanical forces on the neighboring agents
  bool propagate_staticness_neighborhood_ = true;  //!
  /// Flag to determine of an agent is static in the next timestep
  mutable bool is_static_next_ts_ = false;  //!

  /// Function to copy behaviors from existing Agent to this one
  /// and to initialize them.
  /// This function sets the attributes `NewAgentEvent::existing_behavior`
  /// and `NewAgentEvent::new_behaviors` to their correct value.
  void InitializeBehaviors(const NewAgentEvent& event);

  /// @brief Function to invoke the Update method of the behavior or remove
  ///                  it from `current`.
  /// Forwards the call to Update to each behavior of the existing
  /// agent and removes behaviors if they are flagged.
  /// This function sets the attributes `NewAgentEvent::existing_behavior`
  /// and `NewAgentEvent::new_behaviors` to their correct value.
  void UpdateBehaviors(const NewAgentEvent& event);

  BDM_CLASS_DEF(Agent, 1)
};

}  // namespace bdm

#endif  // CORE_AGENT_AGENT_H_
