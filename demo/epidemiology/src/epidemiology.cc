// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#include "epidemiology.h"
#include "core/multi_simulation/multi_simulation.h"
#include "sim-param.h"

const bdm::ParamGroupUid SimParam::kUid =
    bdm::ParamGroupUidGenerator::Get()->NewUid();

int main(int argc, const char** argv) {
  // register parameters that are specific for this simulation
  bdm::Param::RegisterParamGroup(new bdm::SimParam());

  // define additional command line options
  bdm::CommandLineOptions clo(argc, argv);
  clo.AddOption<std::string>("mode", "sim-and-analytical");
  clo.AddOption<double>("beta", "0.06719");
  clo.AddOption<double>("gamma", "0.00521");
  clo.AddOption<uint64_t>("repeat", "10");
  clo.AddOption<bool>("no-legend", "false");
  auto mode = clo.Get<std::string>("mode");

  bdm::MultiSimulation pe(argc, argv);
  return pe.Execute(bdm::Simulate);
}
