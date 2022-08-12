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

#ifndef CORE_DISTRIBUTION_DISTRIBUTOR_H_
#define CORE_DISTRIBUTION_DISTRIBUTOR_H_

namespace bdm {
namespace experimental {

struct Distributor {
  Distributor();
  virtual ~Distributor();

  /// Moves agents outside the local space to the process that owns
  /// the corresponding space
  virtual void MigrateAgents() = 0;

  /// TODO documentation
  virtual void UpdateAura() = 0;
};

// -----------------------------------------------------------------------------
enum DistributorType { kSpatialSTK };

// -----------------------------------------------------------------------------
/// Factory method to create a concrete distributor
Distributor* CreateDistributor(DistributorType type);

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_DISTRIBUTION_DISTRIBUTOR_H_
