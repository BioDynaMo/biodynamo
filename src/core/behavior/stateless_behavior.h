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

#ifndef CORE_BEHAVIOR_STATELESS_BEHAVIOR_H_
#define CORE_BEHAVIOR_STATELESS_BEHAVIOR_H_

#include "core/behavior/behavior.h"

namespace bdm {

/// \brief Simplifies the creation of behaviors without attributes.
/// Let's assume that we want to create a behavior that divides a
/// cell agent in each iteration. The following code example
/// uses the StatelessBehavior:
/// \code
/// StatelessBehavior rapid_division([](Agent* agent) {
///    bdm_static_cast<Cell*>(agent)->Divide(0.5);
/// });
/// \endcode
/// NB: The lambda expression is not allowed to use captures,
/// because StatelessBehavior casts it to a function pointer.
/// Without StatelessBehavior the following code would be required.
/// \code
/// struct RapidDivision : public Behavior {
///   BDM_BEHAVIOR_HEADER(RapidDivision, Behavior, 1);
///
///   RapidDivision() = default;
///   virtual ~RapidDivision() = default;
///
///   void Run(Agent* agent) override {
///     bdm_static_cast<Cell*>(agent)->Divide(0.5);
///   }
/// };
/// \endcode
class StatelessBehavior : public Behavior {
  BDM_BEHAVIOR_HEADER(StatelessBehavior, Behavior, 1);

 public:
  using FPtr = void (*)(Agent*);
  StatelessBehavior() : fptr_(nullptr) {}
  StatelessBehavior(const FPtr fptr) : fptr_(fptr) {}
  StatelessBehavior(const StatelessBehavior& other)
      : Behavior(other), fptr_(other.fptr_) {}
  virtual ~StatelessBehavior() = default;

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
