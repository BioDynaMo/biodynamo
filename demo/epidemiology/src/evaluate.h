// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef EVALUATE_H_
#define EVALUATE_H_

#include <TAxis.h>
#include <TLegend.h>
#include <TMultiGraph.h>

#include <cmath>
#include <vector>

#include "biodynamo.h"
#include "person.h"

namespace bdm {

using experimental::TimeSeries;

// ---------------------------------------------------------------------------
inline void SetupResultCollection(Simulation* sim) {
  auto* ts = sim->GetTimeSeries();
  // susceptible
  auto susceptible = [](Simulation* sim) {
    auto condition = L2F([](Agent* a) {
      return bdm_static_cast<Person*>(a)->state_ == State::kSusceptible;
    });
    auto result = static_cast<double>(bdm::experimental::Count(sim, condition));
    auto num_agents = sim->GetResourceManager()->GetNumAgents() - 1;
    return result / static_cast<double>(num_agents);
  };
  ts->AddCollector("susceptible", susceptible);
  // infected
  auto infected = [](Simulation* sim) {
    auto condition = L2F([](Agent* a) {
      return bdm_static_cast<Person*>(a)->state_ == State::kInfected;
    });
    auto result = static_cast<double>(bdm::experimental::Count(sim, condition));
    auto num_agents = sim->GetResourceManager()->GetNumAgents() - 1;
    return result / static_cast<double>(num_agents);
  };
  ts->AddCollector("infected", infected);
  // recovered
  auto recovered = [](Simulation* sim) {
    auto condition = L2F([](Agent* a) {
      return bdm_static_cast<Person*>(a)->state_ == State::kRecovered;
    });
    auto result = static_cast<double>(bdm::experimental::Count(sim, condition));
    auto num_agents = sim->GetResourceManager()->GetNumAgents() - 1;
    return result / static_cast<double>(num_agents);
  };
  ts->AddCollector("recovered", recovered);
}

// ---------------------------------------------------------------------------
inline void PlotResults(const TimeSeries* analytical, const TimeSeries* mean,
                        const std::vector<TimeSeries>& individual_rd,
                        const experimental::Style& style,
                        const std::string& folder,
                        const bool plot_legend = true,
                        const std::string& filename = "result") {
  TimeSeries allts;
  if (mean) {
    allts.Add(*mean, "mean");
  }
  if (analytical) {
    allts.Add(*analytical, "analytical");
  }
  int i = 0;
  for (auto& ind_ts : individual_rd) {
    allts.Add(ind_ts, Concat("i", i++));
  }
  LineGraph lg(&allts, "", "Time [h]", "Population Fraction", plot_legend,
               style, 350, 250);
  if (analytical) {
    lg.Add("susceptible-analytical", " ", "L", kBlue, 1.0, kDashed, 2, kBlue,
           1.0, 1, 1, 0, 1.0, 0);
    lg.Add("infected-analytical", " ", "L", kRed, 1.0, kDashed, 2, kRed, 1.0, 1,
           1, 0, 1.0, 0);
    lg.Add("recovered-analytical", "SIR analytical", "L", kGreen, 1.0, kDashed,
           2, kGreen, 1.0, 1, 1, 0, 1.0, 0);
  }
  if (mean) {
    lg.Add("susceptible-mean", " ", "L", kBlue, 1.0, kSolid, 2, kBlue, 1.0, 1,
           1, 0, 1.0, 0);
    lg.Add("infected-mean", " ", "L", kRed, 1.0, kSolid, 2, kRed, 1.0, 1, 1, 0,
           1.0, 0);
    lg.Add("recovered-mean", "SIR sim mean", "L", kGreen, 1.0, kSolid, 2,
           kGreen, 1.0, 1, 1, 0, 1.0, 0);
  }

  for (uint64_t i = 0; i < individual_rd.size(); ++i) {
    lg.Add(Concat("susceptible-i", i), " ", "L", kBlue, 0.2, kSolid, 1, kBlue,
           0.2);
    lg.Add(Concat("infected-i", i), " ", "L", kRed, 0.2, kSolid, 1, kRed, 0.2);
    lg.Add(Concat("recovered-i", i), "SIR single sim", "L", kGreen, 0.2, kSolid,
           1, kGreen, 0.2);
  }

  if (plot_legend) {
    // Keep only the first 4 entries of the legend
    lg.SetLegendPosNDC(0.5482955, 0.3799213, 0.9261364, 0.7224409);
    auto* legend = lg.GetTLegend();
    auto list = legend->GetListOfPrimitives();
    for (uint64_t i = 0; i < individual_rd.size() * 3; i++) {
      list->Remove(list->At(9));
    }
  }

  lg.GetTMultiGraph()->SetMaximum(1.);
  lg.SaveAs(Concat(folder, "/", filename), {".svg", ".root", ".C"});
}

}  // namespace bdm

#endif  // EVALUATE_H_
