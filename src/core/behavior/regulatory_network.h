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

#ifndef CORE_BEHAVIOR_REGULATORY_NETWORK_H_
#define CORE_BEHAVIOR_REGULATORY_NETWORK_H_

#ifdef USE_BOOST

#include "core/behavior/behavior.h"

#ifndef __ROOTCLING__
#include "boost/numeric/odeint.hpp"
#include "boost/phoenix/core.hpp"
#include "boost/phoenix/operator.hpp"
#endif

namespace bdm {
class RegulatoryNetwork : public Behavior {
  BDM_BEHAVIOR_HEADER(RegulatoryNetwork, Behavior, 1);

 public:
  RegulatoryNetwork() { AlwaysCopyToNew(); }
  virtual ~RegulatoryNetwork() = default;

  void Run(Agent* agent) override {};

};
}

#endif  // USE_BOOST

#endif  // CORE_BEHAVIOR_REGULATORY_NETWORK_H_

