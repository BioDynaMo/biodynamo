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

#include "core/util/plot_memory_layout.h"

#include <algorithm>
#include <limits>
#include <string>
// ROOT
#include <TAxis.h>
#include <TCanvas.h>
#include <TFrame.h>
#include <TGraph.h>
#include <TGraphAsymmErrors.h>
#include <TGraphErrors.h>
#include <TH1F.h>
#include <TLegend.h>
#include <TMath.h>
#include <TMultiGraph.h>
#include <TPad.h>
#include <TStyle.h>
// BioDynaMo
#include "core/agent/agent.h"
#include "core/environment/environment.h"
#include "core/scheduler.h"
#include "core/simulation.h"
#include "core/simulation_space.h"

namespace bdm {

// -----------------------------------------------------------------------------
void PlotMemoryLayout(const std::vector<Agent*>& agents, int numa_node) {
  TCanvas c;
  c.SetCanvasSize(1920, 1200);
  std::vector<real_t> x(agents.size());
  std::vector<real_t> y(agents.size());

  uint64_t min = std::numeric_limits<uint64_t>::max();
  for (uint64_t i = 0; i < agents.size(); ++i) {
    auto val = reinterpret_cast<uint64_t>(agents[i]);
    if (val < min) {
      min = val;
    }
  }
  for (uint64_t i = 1; i < agents.size(); ++i) {
    x[i] = i;
    y[i] = reinterpret_cast<uint64_t>(agents[i]) - min;
  }
  TGraph graph(agents.size(), x.data(), y.data());
  graph.SetTitle(";Agent element index; Virtual memory address");
  graph.Draw("ap");

  c.Update();
  gPad->Modified();
  gPad->Update();
  c.Modified();
  c.cd(0);
  auto steps = Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
  auto dir = Simulation::GetActive()->GetOutputDir();
  c.SaveAs(Concat(dir, "/mem-layout-", steps, "-", numa_node, ".png").c_str());
}

// -----------------------------------------------------------------------------
void PlotMemoryHistogram(const std::vector<Agent*>& agents, int numa_node) {
  TCanvas c;
  c.SetCanvasSize(1920, 1200);

  TH1F hist("", "", 100, 1, 10000);
  hist.SetTitle(";#Delta bytes; Count");
  for (uint64_t i = 1; i < agents.size(); ++i) {
    float val = 0;
    auto t = reinterpret_cast<uint64_t>(agents[i]);
    auto l = reinterpret_cast<uint64_t>(agents[i - 1]);
    if (t > l) {
      val = t - l;
    } else {
      val = l - t;
    }
    val = std::min(val, 10000.f - 1);
    hist.Fill(val);
  }
  hist.Draw();

  c.Update();
  gPad->SetLogy();
  gPad->Modified();
  gPad->Update();
  c.Modified();
  c.cd(0);
  auto steps = Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
  auto dir = Simulation::GetActive()->GetOutputDir();
  c.SaveAs(
      Concat(dir, "/mem-layout-hist-", steps, "-", numa_node, ".png").c_str());
}

// -----------------------------------------------------------------------------
struct Fen : public Functor<void, Agent*, real_t> {
  std::vector<int64_t>& diffs;
  Agent* query;
  Fen(std::vector<int64_t>& diffs, Agent* query) : diffs(diffs), query(query) {}

  void operator()(Agent* neighbor, real_t) {
    if (neighbor == query) {
      return;
    }
    auto t = reinterpret_cast<int64_t>(query);
    auto l = reinterpret_cast<int64_t>(neighbor);
    diffs.push_back(t - l);
  }
};

// -----------------------------------------------------------------------------
struct Fea : public Functor<void, Agent*, AgentHandle> {
  std::vector<int64_t>& diffs;
  explicit Fea(std::vector<int64_t>& diffs) : diffs(diffs) {}

  void operator()(Agent* agent, AgentHandle) {
    Fen fen(diffs, agent);
    auto* sim = Simulation::GetActive();
    auto* space = sim->GetSimulationSpace();
    auto squared_radius = space->GetInteractionRadiusSquared();
    sim->GetExecutionContext()->ForEachNeighbor(fen, *agent, squared_radius);
  }
};

// -----------------------------------------------------------------------------
void PlotNeighborMemoryHistogram(bool before) {
  TCanvas c;
  c.SetCanvasSize(1920, 1200);
  auto* rm = Simulation::GetActive()->GetResourceManager();
  if (rm->GetNumAgents() == 0) {
    return;
  }
  std::vector<int64_t> diffs;
  diffs.reserve(rm->GetNumAgents() * 3);
  if (!before) {
    Simulation::GetActive()->GetEnvironment()->Update();
  }
  Fea fea(diffs);
  rm->ForEachAgent(fea);
  auto min = std::numeric_limits<float>::max();
  auto max = std::numeric_limits<float>::min();
  for (uint64_t i = 0; i < diffs.size(); ++i) {
    if (diffs[i] < min) {
      min = diffs[i];
    }
    if (diffs[i] > max) {
      max = diffs[i];
    }
  }
  min--;
  max++;
  uint64_t nbins = std::max(static_cast<uint64_t>(100u),
                            static_cast<uint64_t>((max - min) / 20000));
  TH1F hist("", "", nbins, min, max);
  hist.SetTitle(";#Delta bytes; Count");
  for (uint64_t i = 0; i < diffs.size(); ++i) {
    hist.Fill(diffs[i]);
  }
  hist.Draw();

  c.Update();
  gPad->SetLogy();
  gPad->Modified();
  gPad->Update();
  c.Modified();
  c.cd(0);
  auto steps = Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
  auto dir = Simulation::GetActive()->GetOutputDir();
  std::string suffix = "-end";
  if (before) {
    suffix = "-begin";
  }
  c.SaveAs(
      Concat(dir, "/mem-layout-neighbor-hist-", steps, suffix, ".png").c_str());
  c.SaveAs(
      Concat(dir, "/mem-layout-neighbor-hist-", steps, suffix, ".C").c_str());
}

}  // namespace bdm
