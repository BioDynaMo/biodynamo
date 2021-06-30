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

#include "neuroscience/new_agent_event/neurite_bifurcation_event.h"
#include "neuroscience/new_agent_event/neurite_branching_event.h"
#include "neuroscience/new_agent_event/new_neurite_extension_event.h"
#include "neuroscience/new_agent_event/side_neurite_extension_event.h"
#include "neuroscience/new_agent_event/split_neurite_element_event.h"

namespace bdm {
namespace neuroscience {

const NewAgentEventUid NewNeuriteExtensionEvent::kUid =
    NewAgentEventUidGenerator::GetInstance()->GenerateUid();
const NewAgentEventUid NeuriteBifurcationEvent::kUid =
    NewAgentEventUidGenerator::GetInstance()->GenerateUid();
const NewAgentEventUid NeuriteBranchingEvent::kUid =
    NewAgentEventUidGenerator::GetInstance()->GenerateUid();
const NewAgentEventUid SplitNeuriteElementEvent::kUid =
    NewAgentEventUidGenerator::GetInstance()->GenerateUid();
const NewAgentEventUid SideNeuriteExtensionEvent::kUid =
    NewAgentEventUidGenerator::GetInstance()->GenerateUid();

}  // namespace neuroscience
}  // namespace bdm
