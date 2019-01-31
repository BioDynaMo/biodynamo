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

#ifndef CORE_SIM_OBJECT_SO_POINTER_H_
#define CORE_SIM_OBJECT_SO_POINTER_H_

#include <cstdint>
#include <limits>
#include <ostream>
#include <type_traits>

#include "core/simulation.h"
#include "core/simulation_backup.h"

namespace bdm {

/// Simulation object pointer. Required to point into simulation objects with
/// `Soa` backend. `SoaRef` has the drawback that its size depends on the number
/// of data members. Benefit compared to SoHandle is, that the compiler knows
/// the type returned by `Get` and can therefore inline the code from the callee
/// and perform optimizations
/// @tparam TSoSimBackend simulation object type with simulation backend
/// @tparam TBackend backend - required to avoid extracting it from
///         TSoSimBackend which would result in "incomplete type errors" in
///         certain cases.
/// NB: ROOT IO only supports `so_container_` that point into the
/// `ResourceManager`. Separate containers will not be serialized correctly!
template <typename TSoSimBackend, typename TBackend>
class SoPointer {
  using SoSoaRef = typename TSoSimBackend::template Self<SoaRef>;

 public:
  explicit SoPointer(SoUid uid) : uid_(uid) {}

  /// constructs an SoPointer object representing a nullptr
  SoPointer() {}

  /// Equals operator that enables the following statement `so_ptr == nullptr;`
  bool operator==(std::nullptr_t) const {
    return uid_ == std::numeric_limits<uint64_t>::max();
  }

  /// Not equal operator that enables the following statement `so_ptr !=
  /// nullptr;`
  bool operator!=(std::nullptr_t) const { return !this->operator==(nullptr); }

  bool operator==(const SoPointer& other) const { return uid_ == other.uid_; }

  /// Assignment operator that changes the internal representation to nullptr.
  /// Makes the following statement possible `so_ptr = nullptr;`
  SoPointer& operator=(std::nullptr_t) {
    uid_ = std::numeric_limits<uint64_t>::max();
    return *this;
  }

  template <typename TTBackend = TBackend, typename TSimulation = Simulation>
  typename std::enable_if<std::is_same<TTBackend, Scalar>::value,
                          TSoSimBackend&>::type
  operator->() {
    assert(*this != nullptr);
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    return ctxt->template GetSimObject<TSoSimBackend>(uid_);
  }

  template <typename TTBackend = TBackend, typename TSimulation = Simulation>
  typename std::enable_if<std::is_same<TTBackend, Scalar>::value,
                          const TSoSimBackend&>::type
  operator->() const {
    assert(*this != nullptr);
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    return ctxt->template GetConstSimObject<SoSoaRef>(uid_);
  }

  template <typename TTBackend = TBackend, typename TSimulation = Simulation>
  typename std::enable_if<std::is_same<TTBackend, Soa>::value, SoSoaRef>::type
  operator->() {
    assert(*this != nullptr);
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    return ctxt->template GetSimObject<SoSoaRef>(uid_);
  }

  template <typename TTBackend = TBackend, typename TSimulation = Simulation>
  typename std::enable_if<std::is_same<TTBackend, Soa>::value,
                          const SoSoaRef>::type
  operator->() const {
    assert(*this != nullptr);
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    return ctxt->template GetConstSimObject<SoSoaRef>(uid_);
  }

  friend std::ostream& operator<<(
      std::ostream& str, const SoPointer<TSoSimBackend, TBackend>& so_ptr) {
    str << "{ uid: " << so_ptr.uid_ << "}";
    return str;
  }

 private:
  SoUid uid_ = std::numeric_limits<uint64_t>::max();

  BDM_TEMPLATE_CLASS_DEF_CUSTOM_STREAMER(SoPointer, 2);
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SO_POINTER_H_
