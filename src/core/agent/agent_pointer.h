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

#ifndef CORE_AGENT_AGENT_POINTER_H_
#define CORE_AGENT_AGENT_POINTER_H_

#include <cstdint>
#include <limits>
#include <ostream>
#include <type_traits>

#include "core/agent/agent_uid.h"
#include "core/execution_context/execution_context.h"
#include "core/simulation.h"
#include "core/util/root.h"

namespace bdm {

class Agent;

/// Agent pointer. Required to point to an agent with
/// throughout the whole simulation. Raw pointers cannot be used, because
/// an agent might be copied to a different NUMA domain, or if it resides
/// on a different address space in case of a distributed runtime.
/// Benefit compared to AgentHandle is, that the compiler knows
/// the type returned by `Get` and can therefore inline the code from the callee
/// and perform optimizations.
/// @tparam TAgent agent type
template <typename TAgent>
class AgentPointer {
 public:
  explicit AgentPointer(const AgentUid& uid) {
    agent_ = Cast<Agent, TAgent>(Simulation::GetActive()->GetExecutionContext()->GetAgent(uid));
  }
  explicit AgentPointer(Agent* agent) : agent_(Cast<Agent, TAgent>(agent)) {}

  /// constructs an AgentPointer object representing a nullptr
  AgentPointer() = default;

  ~AgentPointer() = default;

  uint64_t GetUidAsUint64() const { 
    if (*this == nullptr) {
      return AgentUid();
    }
    return agent_->GetUid(); 
  }

  AgentUid GetUid() const { 
    if (*this == nullptr) {
      return AgentUid();
    }
    return agent_->GetUid(); 
  }

  /// Equals operator that enables the following statement `agent_ptr ==
  /// nullptr;`
  bool operator==(std::nullptr_t) const { return agent_ == nullptr; }

  /// Not equal operator that enables the following statement `agent_ptr !=
  /// nullptr;`
  bool operator!=(std::nullptr_t) const { return !this->operator==(nullptr); }

  bool operator==(const AgentPointer& other) const {
    return agent_ == other.agent_;
  }

  bool operator!=(const AgentPointer& other) const {
    return agent_ != other.agent_;
  }

  template <typename TOtherAgent>
  bool operator==(const TOtherAgent* other) const {
    if (other == nullptr) {
      return agent_ == nullptr;
    } else {
      return agent_->GetUid() == other->GetUid();
    }
  }

  template <typename TOtherAgent>
  bool operator!=(const TOtherAgent* other) const {
    return !this->operator==(other);
  }

  /// Assignment operator that changes the internal representation to nullptr.
  /// Makes the following statement possible `agent_ptr = nullptr;`
  AgentPointer& operator=(std::nullptr_t) {
    agent_ = nullptr;
    return *this;
  }

  TAgent* operator->() {
    assert(*this != nullptr);
    return agent_;
    // auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    // return Cast<Agent, TAgent>(ctxt->GetAgent(uid_));
  }

  const TAgent* operator->() const {
    assert(*this != nullptr);
    return agent_;
    // auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    // return Cast<const Agent, const TAgent>(ctxt->GetConstAgent(uid_));
  }

  friend std::ostream& operator<<(std::ostream& str,
                                  const AgentPointer& agent_ptr) {
    str << "{ uid: " << agent_ptr.GetUid() << "}";
    return str;
  }

  TAgent& operator*() { return *(this->operator->()); }

  const TAgent& operator*() const { return *(this->operator->()); }

  operator bool() const { return *this != nullptr; }  // NOLINT

  operator AgentPointer<Agent>() const { return AgentPointer<Agent>(Cast<TAgent, Agent>(agent_)); }

  TAgent* Get() { return this->operator->(); }

  const TAgent* Get() const { return this->operator->(); }

  bool operator<(const AgentPointer& other) const {
    return agent_ < other.agent_;
  }

 private:
  // AgentUid uid_;
  TAgent* agent_ = nullptr;

  template <typename TFrom, typename TTo>
  typename std::enable_if<std::is_base_of<TFrom, TTo>::value, TTo*>::type Cast(
      TFrom* agent) const {
    return static_cast<TTo*>(agent);
  }

  template <typename TFrom, typename TTo>
  typename std::enable_if<!std::is_base_of<TFrom, TTo>::value, TTo*>::type Cast(
      TFrom* agent) const {
    return dynamic_cast<TTo*>(agent);
  }

  BDM_CLASS_DEF_NV(AgentPointer, 2);
};

template <typename T>
struct is_agent_ptr {                   // NOLINT
  static constexpr bool value = false;  // NOLINT
};

template <typename T>
struct is_agent_ptr<AgentPointer<T>> {  // NOLINT
  static constexpr bool value = true;   // NOLINT
};

namespace detail {

struct ExtractUid {
  template <typename T>
  static typename std::enable_if<is_agent_ptr<T>::value, uint64_t>::type GetUid(
      const T& t) {
    return t.GetUid();
  }

  template <typename T>
  static typename std::enable_if<!is_agent_ptr<T>::value, uint64_t>::type
  GetUid(const T& t) {
    return 0;  // std::numeric_limits<uint64_t>::max();
  }
};

}  // namespace detail

}  // namespace bdm

#endif  // CORE_AGENT_AGENT_POINTER_H_
