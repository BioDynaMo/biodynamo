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

#include "core/sim_object/so_pointer.h"

#include "core/simulation.h"
#include "core/sim_object/sim_object.h"
#include "core/execution_context/in_place_exec_ctxt.h"

namespace bdm {

SoPointer::SoPointer(SoUid uid) : uid_(uid) {}

SoPointer::SoPointer() {}

bool SoPointer::operator==(std::nullptr_t) const {
  return uid_ == std::numeric_limits<uint64_t>::max();
}

bool SoPointer::operator!=(std::nullptr_t) const { return !this->operator==(nullptr); }

bool SoPointer::operator==(const SoPointer& other) const { return uid_ == other.uid_; }

SoPointer& SoPointer::operator=(std::nullptr_t) {
  uid_ = std::numeric_limits<uint64_t>::max();
  return *this;
}

SimObject* SoPointer::operator->() {
  assert(*this != nullptr);
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  return ctxt->GetSimObject(uid_);
}

const SimObject* SoPointer::operator->() const {
  assert(*this != nullptr);
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  return ctxt->GetConstSimObject(uid_);
}

std::ostream& operator<<(std::ostream& str, const SoPointer& so_ptr) {
  str << "{ uid: " << so_ptr.uid_ << "}";
  return str;
}

}  // namespace bdm
