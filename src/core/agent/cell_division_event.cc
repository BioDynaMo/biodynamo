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

#include "core/agent/cell_division_event.h"
#include "core/agent/eps_production_event.h"


namespace bdm {

const NewAgentEventUid CellDivisionEvent::kUid =
    NewAgentEventUidGenerator::GetInstance()->GenerateUid();

const NewAgentEventUid EpsProductionEvent::kUid =
    NewAgentEventUidGenerator::GetInstance()->GenerateUid();

}  // namespace bdm
