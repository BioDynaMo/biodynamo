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
#ifndef MY_NEURITE_H_
#define MY_NEURITE_H_

#include "core/sim_object/sim_object.h"
#include "neuroscience/neurite_element.h"

namespace bdm {

// Define my custom neurite MyNeurite, which extends NeuriteElement
BDM_SIM_OBJECT(MyNeurite, experimental::neuroscience::NeuriteElement) {
  BDM_SIM_OBJECT_HEADER(MyNeurite, experimental::neuroscience::NeuriteElement,
                        1, can_branch_);

 public:
  MyNeuriteExt() {}
  MyNeuriteExt(const array<double, 3>& position) : Base(position) {}

  /// Default event constructor
  template <typename TEvent, typename TOther>
  MyNeuriteExt(const TEvent& event, TOther* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {}

  /// Default event handler
  template <typename TEvent, typename... TOthers>
  void EventHandler(const TEvent& event, TOthers*... others) {
    Base::EventHandler(event, others...);
  }

  void SetCanBranch(int b) { can_branch_[kIdx] = b; }
  bool GetCanBranch() const { return can_branch_[kIdx]; }

 private:
  vec<bool> can_branch_;
};

}  // namespace bdm

#endif  // MY_NEURITE_H_
