// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifndef CORE_BEHAVIOR_STATELESS_BEHAVIOR_H_
#define CORE_BEHAVIOR_STATELESS_BEHAVIOR_H_

#include "core/behavior/behavior.h"

namespace bdm {

// TODO documentation
class StatelessBehavior : public Behavior {
  BDM_BEHAVIOR_HEADER(StatelessBehavior, Behavior, 1);

 public:
  using FPtr = void (*)(Agent*);
  StatelessBehavior() : fptr_(nullptr) {}
  StatelessBehavior(const FPtr fptr) : fptr_(fptr) {}
  StatelessBehavior(const StatelessBehavior& other)
      : Behavior(other), fptr_(other.fptr_) {}
  virtual ~StatelessBehavior() {}

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
    fptr_ = static_cast<StatelessBehavior*>(event.existing_behavior)->fptr_;
  }

  void Run(Agent* agent) override { 
    if (!fptr_) {
      return;
    }
    fptr_(agent); 
  }

 private:
  FPtr fptr_;  //!
};

// The following custom streamer should be visible to rootcling for dictionary
// generation, but not to the interpreter!
#if (!defined(__CLING__) || defined(__ROOTCLING__)) && defined(USE_DICT)

// The custom streamer is needed because ROOT can't stream function pointers
// by default.
inline void StatelessBehavior::Streamer(TBuffer& R__b) {
  if (R__b.IsReading()) {
    R__b.ReadClassBuffer(StatelessBehavior::Class(), this);
    Long64_t l;
    R__b.ReadLong64(l);
    this->fptr_ = reinterpret_cast<StatelessBehavior::FPtr>(l);
  } else {
    R__b.WriteClassBuffer(StatelessBehavior::Class(), this);
    Long64_t l = reinterpret_cast<Long64_t>(this->fptr_);
    R__b.WriteLong64(l);
  }
}

#endif  // !defined(__CLING__) || defined(__ROOTCLING__)


}  // namespace bdm

#endif  // CORE_BEHAVIOR_STATELESS_BEHAVIOR_H_
