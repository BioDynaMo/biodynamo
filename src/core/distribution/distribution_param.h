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

#ifndef CORE_DISTRIBUTION_DISTRIBUTION_PARAM_H_
#define CORE_DISTRIBUTION_DISTRIBUTION_PARAM_H_

#include "core/param/param_group.h"

namespace bdm {
namespace experimental {

struct DistributionParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(DistributionParam, 1);
  /// The box length of the distributed mesh is determined by
  /// multiplying the UniformGridEnvironment::box_length_ with
  /// the box length factor
  unsigned box_length_factor = 10;
};

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_DISTRIBUTION_DISTRIBUTION_PARAM_H_
