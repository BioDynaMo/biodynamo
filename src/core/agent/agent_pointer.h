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

#ifndef CORE_AGENT_AGENT_POINTER_H_
#define CORE_AGENT_AGENT_POINTER_H_

#include <cstdint>
#include <limits>
#include <ostream>
#include <type_traits>

#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/agent/agent_uid.h"
#include "core/simulation.h"
#include "core/util/root.h"

namespace bdm {

class Agent;

/// Simulation object pointer. Required to point to a agent with
/// throughout the whole simulation. Raw pointers cannot be used, because
/// a sim object might be copied to a different NUMA domain, or if it resides
/// on a different address space in case of a distributed runtime.
/// Benefit compared to AgentHandle is, that the compiler knows
/// the type returned by `Get` and can therefore inline the code from the callee
/// and perform optimizations.
/// @tparam TAgent agent type
template <typename TAgent>
class AgentPointer {
 public:
  explicit AgentPointer(const AgentUid& uid) : uid_(uid) {}

  /// constructs an AgentPointer object representing a nullptr
  AgentPointer() {}

  virtual ~AgentPointer() {}

  uint64_t GetUid() const { return uid_; }

  /// Equals operator that enables the following statement `agent_ptr == nullptr;`
  bool operator==(std::nullptr_t) const { return uid_ == AgentUid(); }

  /// Not equal operator that enables the following statement `agent_ptr !=
  /// nullptr;`
  bool operator!=(std::nullptr_t) const { return !this->operator==(nullptr); }

  bool operator==(const AgentPointer& other) const { return uid_ == other.uid_; }

  bool operator!=(const AgentPointer& other) const { return uid_ != other.uid_; }

  template <typename TSo>
  bool operator==(const TSo* other) const {
    return uid_ == other->GetUid();
  }

  template <typename TSo>
  bool operator!=(const TSo* other) const {
    return !this->operator==(other);
  }

  /// Assignment operator that changes the internal representation to nullptr.
  /// Makes the following statement possible `agent_ptr = nullptr;`
  AgentPointer& operator=(std::nullptr_t) {
    uid_ = AgentUid();
    return *this;
  }

  TAgent* operator->() {
    assert(*this != nullptr);
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    return Cast<Agent, TAgent>(ctxt->GetAgent(uid_));
  }

  const TAgent* operator->() const {
    assert(*this != nullptr);
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    return Cast<const Agent, const TAgent>(
        ctxt->GetConstAgent(uid_));
  }

  friend std::ostream& operator<<(std::ostream& str, const AgentPointer& agent_ptr) {
    str << "{ uid: " << agent_ptr.uid_ << "}";
    return str;
  }

  TAgent& operator*() { return *(this->operator->()); }

  const TAgent& operator*() const { return *(this->operator->()); }

  operator bool() const { return *this != nullptr; }

  TAgent* Get() { return this->operator->(); }

  const TAgent* Get() const { return this->operator->(); }

 private:
  AgentUid uid_;

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

  BDM_CLASS_DEF(AgentPointer, 2);
};

template <typename T>
struct is_agent_ptr {
  static constexpr bool value = false;  // NOLINT
};

template <typename T>
struct is_agent_ptr<AgentPointer<T>> {
  static constexpr bool value = true;  // NOLINT
};

namespace detail {

struct ExtractUid {
  template <typename T>
  static typename std::enable_if<is_agent_ptr<T>::value, uint64_t>::type GetUid(
      const T& t) {
    return t.GetUid();
  }

  template <typename T>
  static typename std::enable_if<!is_agent_ptr<T>::value, uint64_t>::type GetUid(
      const T& t) {
    return 0;  // std::numeric_limits<uint64_t>::max();
  }
};

}  // namespace detail

}  // namespace bdm

#endif  // CORE_AGENT_AGENT_POINTER_H_