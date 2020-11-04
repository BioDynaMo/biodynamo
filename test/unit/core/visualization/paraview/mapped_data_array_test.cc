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
#ifdef USE_PARAVIEW

#include "core/visualization/paraview/mapped_data_array.h"
#include <TClassTable.h>
#include <gtest/gtest.h>
#include "core/agent/agent_uid_generator.h"
#include "core/util/jit.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/neuroscience.h"
#include "unit/test_util/test_util.h"

namespace bdm {

// -----------------------------------------------------------------------------
TEST(GetDataMemberForVisTest, NeuriteElement) {
  neuroscience::InitModule();
  Simulation simulation(TEST_NAME);
  // Discard the first uid to have a non zero uid for the agent

  simulation.GetAgentUidGenerator()->NewAgentUid();
  using NeuriteElement = neuroscience::NeuriteElement;
  using NeuronOrNeurite = neuroscience::NeuronOrNeurite;
  NeuriteElement ne;

  auto* tclass = TClassTable::GetDict("bdm::neuroscience::NeuriteElement")();

  // diameter
  {
    ne.SetDiameter(123);
    auto dms = FindDataMemberSlow(tclass, "diameter_");
    ASSERT_EQ(1u, dms.size());

    GetDataMemberForVis<double*, NeuriteElement, double> get_dm;
    get_dm.dm_offset_ = dms[0]->GetOffset();
    EXPECT_EQ(123, *get_dm(&ne));
    EXPECT_EQ(123, get_dm(&ne)[0]);
  }
  // actual_length_
  {
    ne.SetActualLength(123);
    auto dms = FindDataMemberSlow(tclass, "actual_length_");
    ASSERT_EQ(1u, dms.size());

    GetDataMemberForVis<double*, NeuriteElement, double> get_dm;
    get_dm.dm_offset_ = dms[0]->GetOffset();
    EXPECT_EQ(123, *get_dm(&ne));
    EXPECT_EQ(123, get_dm(&ne)[0]);
  }
  // mass_location_
  {
    ne.SetMassLocation({1, 2, 3});
    auto dms = FindDataMemberSlow(tclass, "mass_location_");
    ASSERT_EQ(1u, dms.size());

    GetDataMemberForVis<double*, NeuriteElement, Double3> get_dm;
    get_dm.dm_offset_ = dms[0]->GetOffset();
    EXPECT_EQ(1, get_dm(&ne)[0]);
    EXPECT_EQ(2, get_dm(&ne)[1]);
    EXPECT_EQ(3, get_dm(&ne)[2]);
  }
  // uid_
  {
    auto dms = FindDataMemberSlow(tclass, "uid_");
    ASSERT_EQ(1u, dms.size());

    GetDataMemberForVis<uint64_t*, Agent, AgentUid> get_dm;
    get_dm.dm_offset_ = dms[0]->GetOffset();
    EXPECT_EQ(1u, *get_dm(&ne));
    EXPECT_EQ(1u, get_dm(&ne)[0]);
  }
  // agent_pointer_
  {
    auto dms = FindDataMemberSlow(tclass, "daughter_right_");
    ASSERT_EQ(1u, dms.size());

    GetDataMemberForVis<uint64_t*, NeuriteElement, AgentPointer<NeuronOrNeurite>>
        get_dm;
    get_dm.dm_offset_ = dms[0]->GetOffset();
    EXPECT_EQ(18446744073709551615ULL, *get_dm(&ne));
    EXPECT_EQ(18446744073709551615ULL, get_dm(&ne)[0]);
  }
}

}  // namespace bdm

#endif  // USE_PARAVIEW
