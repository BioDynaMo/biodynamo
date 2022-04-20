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

/// Class `AgentPointer` supports two different modes:
/// First, an indirect mode in which the AgentUid is used to determine
/// an agent.\n
/// Second, a direct mode in which the raw `Agent*` is used.
/// The indirect mode is necessary if the `Agent*` changes during the
/// simulation. This can happen due to sorting and balancing operation
/// to improve the memory layout, or in a distributed runtime, where an
/// agent might reside in a different address space.
/// If the `Agent*` of an agent does not change during a simulation,
/// the direct mode can be used to achieve better performance.
enum AgentPointerMode { kIndirect, kDirect };
/// Global variable to select the agent pointer mode. \n
/// Replacing the global variable with an attribute in `Param`
/// is too costly in terms of performance.
/// \see AgentPointerMode
extern AgentPointerMode gAgentPointerMode;

/// Agent pointer. Required to point to an agent
/// throughout the whole simulation. \n
/// This class provides a common interface for different modes.
/// See `AgentPointerMode` for more details.
/// Benefit compared to AgentHandle is, that the compiler knows
/// the type returned by `Get` and can therefore inline the code from the callee
/// and perform optimizations.
/// @tparam TAgent agent type
template <typename TAgent = Agent>
class AgentPointer {
 public:
  explicit AgentPointer(const AgentUid& uid) {
    if (uid == AgentUid()) {
      *this = nullptr;
      return;
    }
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      d_.uid = uid;
    } else {
      auto* ctxt = Simulation::GetActive()->GetExecutionContext();
      d_.agent = Cast<Agent, TAgent>(ctxt->GetAgent(uid));
    }
  }
  explicit AgentPointer(TAgent* agent) {
    if (!agent) {
      *this = nullptr;
    }
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      d_.uid = agent->GetUid();
    } else {
      d_.agent = agent;
    }
  }

  /// Allow `AgentPointer<> ap = nullptr` definitions
  AgentPointer(std::nullptr_t) { *this = nullptr; }

  /// constructs an AgentPointer object representing a nullptr
  AgentPointer() { *this = nullptr; }

  ~AgentPointer() = default;

  uint64_t GetUidAsUint64() const {
    if (*this == nullptr) {
      return AgentUid();
    }
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      return d_.uid;
    } else {
      return d_.agent->GetUid();
    }
  }

  AgentUid GetUid() const {
    if (*this == nullptr) {
      return AgentUid();
    }
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      return d_.uid;
    } else {
      return d_.agent->GetUid();
    }
  }

  /// Equals operator that enables the following statement `agent_ptr ==
  /// nullptr;`
  bool operator==(std::nullptr_t) const {
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      return d_.uid == AgentUid();
    } else {
      return d_.agent == nullptr;
    }
  }

  /// Not equal operator that enables the following statement `agent_ptr !=
  /// nullptr;`
  bool operator!=(std::nullptr_t) const { return !this->operator==(nullptr); }

  bool operator==(const AgentPointer& other) const {
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      return d_.uid == other.d_.uid;
    } else {
      return d_.agent == other.d_.agent;
    }
  }

  bool operator!=(const AgentPointer& other) const {
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      return d_.uid != other.d_.uid;
    } else {
      return d_.agent != other.d_.agent;
    }
  }

  template <typename TOtherAgent>
  bool operator==(const TOtherAgent* other) const {
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      if (other) {
        return d_.uid == other->GetUid();
      } else {
        return d_.uid == AgentUid();
      }
    } else {
      if (d_.agent != nullptr && other != nullptr) {
        return d_.agent->GetUid() == other->GetUid();
      } else if (d_.agent == nullptr && other == nullptr) {
        return true;
      } else {
        return false;
      }
    }
  }

  template <typename TOtherAgent>
  bool operator!=(const TOtherAgent* other) const {
    return !this->operator==(other);
  }

  /// Assignment operator that changes the internal representation to nullptr.
  /// Makes the following statement possible `agent_ptr = nullptr;`
  AgentPointer& operator=(std::nullptr_t) {
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      d_.uid = AgentUid();
    } else {
      d_.agent = nullptr;
    }
    return *this;
  }

  TAgent* operator->() {
    assert(*this != nullptr);
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      if (d_.uid == AgentUid()) {
        return nullptr;
      }
      auto* ctxt = Simulation::GetActive()->GetExecutionContext();
      return Cast<Agent, TAgent>(ctxt->GetAgent(d_.uid));
    } else {
      return d_.agent;
    }
  }

  const TAgent* operator->() const {
    assert(*this != nullptr);
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      if (d_.uid == AgentUid()) {
        return nullptr;
      }
      auto* ctxt = Simulation::GetActive()->GetExecutionContext();
      return Cast<const Agent, const TAgent>(ctxt->GetConstAgent(d_.uid));
    } else {
      return d_.agent;
    }
  }

  friend std::ostream& operator<<(std::ostream& str,
                                  const AgentPointer& agent_ptr) {
    str << "{ uid: " << agent_ptr.GetUid() << "}";
    return str;
  }

  TAgent& operator*() { return *(this->operator->()); }

  const TAgent& operator*() const { return *(this->operator->()); }

  operator bool() const { return *this != nullptr; }  // NOLINT

  operator AgentPointer<Agent>() const {
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      return AgentPointer<Agent>(d_.uid);
    } else {
      return AgentPointer<Agent>(Cast<TAgent, Agent>(d_.agent));
    }
  }

  TAgent* Get() { return this->operator->(); }

  const TAgent* Get() const { return this->operator->(); }

  bool operator<(const AgentPointer& other) const {
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      return d_.uid < other.d_.uid;
    } else {
      return d_.agent < other.d_.agent;
    }
  }

  /// Replace with std::variant once we move to >= c++17
  union Data {
    AgentUid uid;
    TAgent* agent;
  };

 private:
  Data d_ = {AgentUid()};  //!

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

  BDM_CLASS_DEF_NV(AgentPointer, 3);
};

// The following custom streamer should be visible to rootcling for dictionary
// generation, but not to the interpreter!
#if (!defined(__CLING__) || defined(__ROOTCLING__)) && defined(USE_DICT)

template <typename TAgent>
inline void AgentPointer<TAgent>::Streamer(TBuffer& R__b) {
  if (R__b.IsReading()) {
    R__b.ReadClassBuffer(AgentPointer::Class(), this);
    AgentUid restored_uid;
    R__b.ReadClassBuffer(AgentUid::Class(), &restored_uid);
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      d_.uid = restored_uid;
    } else if (restored_uid != AgentUid()) {
      auto* ctxt = Simulation::GetActive()->GetExecutionContext();
      d_.agent = Cast<Agent, TAgent>(ctxt->GetAgent(restored_uid));
    } else {
      d_.agent = nullptr;
    }
  } else {
    R__b.WriteClassBuffer(AgentPointer::Class(), this);
    AgentUid uid;
    if (gAgentPointerMode == AgentPointerMode::kIndirect) {
      uid = d_.uid;
      R__b.WriteClassBuffer(AgentUid::Class(), &d_.uid);
    } else if (d_.agent != nullptr) {
      uid = d_.agent->GetUid();
    }
    R__b.WriteClassBuffer(AgentUid::Class(), &uid);
  }
}

#endif  // !defined(__CLING__) || defined(__ROOTCLING__)

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
