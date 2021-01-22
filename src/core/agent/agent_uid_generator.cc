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

#include "core/agent/agent_uid_generator.h"

namespace bdm {

// constexpr typename AgentUid::Reused_t AgentUid::kReusedMax =
// std::numeric_limits<typename AgentUid::Reused_t>::max();
const typename AgentUid::Reused_t AgentUid::kReusedMax;

}  // namespace bdm
