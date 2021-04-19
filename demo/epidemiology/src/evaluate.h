// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef EVALUATE_H_
#define EVALUATE_H_

#include <cmath>
#include <vector>

#include <TAxis.h>
#include <TCanvas.h>
#include <TFrame.h>
#include <TGraph.h>
#include <TGraphAsymmErrors.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include <TMath.h>
#include <TMultiGraph.h>
#include <TPad.h>
#include <TStyle.h>

#include "core/operation/reduction_op.h"
#include "core/resource_manager.h"
#include "core/simulation.h"

#include "person.h"

namespace bdm {

// ---------------------------------------------------------------------------
struct ResultData {
  std::vector<double> time_;
  std::vector<double> susceptible_;
  std::vector<double> infected_;
  std::vector<double> recovered_;
};

// ---------------------------------------------------------------------------
struct CountSIR : public Functor<void, Agent*, Double4*> {
  void operator()(Agent* agent, Double4* tl_result) {
    auto* person = bdm_static_cast<Person*>(agent);
    (*tl_result)[1] += person->state_ == State::kSusceptible;
    (*tl_result)[2] += person->state_ == State::kInfected;
    (*tl_result)[3] += person->state_ == State::kRecovered;
  }
};

// ---------------------------------------------------------------------------
struct CalcRates : public Functor<Double4, const SharedData<Double4>&> {
  Double4 operator()(const SharedData<Double4>& tl_results) override {
    Double4 result;
    auto* sim = Simulation::GetActive();
    for (auto& el : tl_results) {
      result += el;
    }
    // -1 because an additional cell has been added as a workaround
    auto num_agents = sim->GetResourceManager()->GetNumAgents() - 1;
    result /= num_agents;
    result[0] = sim->GetScheduler()->GetSimulatedSteps();
    return result;
  }
};

// ---------------------------------------------------------------------------
inline void TransferResult(ResultData* rd,
                           const std::vector<Double4>& op_result) {
  rd->time_.reserve(op_result.size());
  rd->susceptible_.reserve(op_result.size());
  rd->infected_.reserve(op_result.size());
  rd->recovered_.reserve(op_result.size());
  for (auto el : op_result) {
    rd->time_.push_back(el[0]);
    rd->susceptible_.push_back(el[1]);
    rd->infected_.push_back(el[2]);
    rd->recovered_.push_back(el[3]);
  }
}

// ---------------------------------------------------------------------------
inline double MSE(const std::vector<double>& v1,
                  const std::vector<double>& v2) {
  if (v1.size() != v2.size()) {
    Log::Fatal("MSE", "vectors must have same length");
  }
  double error = 0;
  for (size_t i = 0; i < v1.size(); ++i) {
    auto diff = v2[i] - v1[i];
    error += diff * diff;
  }
  return error / v1.size();
}

// ---------------------------------------------------------------------------
inline void FinalizePlot(TCanvas* c, TMultiGraph* mg,
                         uint64_t individual_rd_size,
                         const std::string& filename, bool plot_legend) {
  mg->Draw("A");

  // finalize plot
  if (plot_legend) {
    // Keep only the first 4 entries of the legend
    gStyle->SetLegendTextSize(0.06);
    auto* legend = c->BuildLegend(0.5482955, 0.3799213, 0.9261364, 0.7224409);
    auto list = legend->GetListOfPrimitives();
    for (uint64_t i = 0; i < individual_rd_size * 3; i++) {
      list->Remove(list->At(9));
    }
  }
  gStyle->SetTitleFontSize(0.1);

  auto set_axis_attr = [](auto* axis) {
    axis->SetTitleSize(0.09);
    axis->SetTitleOffset(1.0);
    axis->SetTickLength(0.02);
    axis->SetLabelSize(0.09);
    axis->SetLabelOffset(0.005);
  };
  set_axis_attr(mg->GetXaxis());
  set_axis_attr(mg->GetYaxis());
  mg->GetYaxis()->SetNdivisions(5);
  if (!plot_legend) {
    mg->GetXaxis()->SetNdivisions(5);
  }

  // TCanvas::Update() draws the frame, after which one can change it
  c->Update();
  c->GetFrame()->SetBorderSize(12);
  gPad->Modified();
  gPad->Update();
  c->Modified();
  c->cd(0);
  std::cout << "Result plot created at: "
            << Concat(std::string(filename), ".svg") << std::endl;
  c->SaveAs(Concat(std::string(filename), ".svg").c_str());
  c->SaveAs(Concat(std::string(filename), ".C").c_str());
}

// ---------------------------------------------------------------------------
inline void PlotResults(const ResultData* analytical, const ResultData* mean,
                        const std::vector<ResultData>& individual_rd,
                        const std::string& folder,
                        const std::string& title_suffix = "",
                        const bool plot_legend = true,
                        const std::string& filename = "result") {
  TCanvas c;
  c.SetCanvasSize(350, 250);
  TMultiGraph mg;
  c.SetGrid();
  c.SetRightMargin(0.05);
  c.SetLeftMargin(0.2);
  c.SetTopMargin(0.04);
  c.SetBottomMargin(0.2);
  mg.SetTitle(";Time [h];Population Fraction");

  uint64_t n = mean ? mean->time_.size() : analytical->time_.size();

  // analytical
  if (analytical) {
    {
      auto* gr = new TGraph(n, analytical->time_.data(),
                            analytical->susceptible_.data());
      gr->SetTitle(" ");
      gr->SetFillStyle(0);
      gr->SetLineStyle(kDashed);
      gr->SetLineColor(kBlue);
      gr->SetMarkerColor(kBlue);
      gr->SetLineWidth(2);
      mg.Add(gr, "L");
    }
    {
      auto* gr =
          new TGraph(n, analytical->time_.data(), analytical->infected_.data());
      gr->SetTitle(" ");
      gr->SetFillStyle(0);
      gr->SetLineStyle(kDashed);
      gr->SetLineColor(kRed);
      gr->SetMarkerColor(kRed);
      gr->SetLineWidth(2);
      mg.Add(gr, "L");
    }
    {
      auto* gr = new TGraph(n, analytical->time_.data(),
                            analytical->recovered_.data());
      gr->SetTitle("SIR analytical");
      gr->SetFillStyle(0);
      gr->SetLineStyle(kDashed);
      gr->SetLineColor(kGreen);
      gr->SetMarkerColor(kGreen);
      gr->SetLineWidth(2);
      mg.Add(gr, "L");
    }
  }

  // simulation
  if (mean) {
    {
      auto* gr = new TGraph(n, mean->time_.data(), mean->susceptible_.data());
      gr->SetTitle(" ");
      gr->SetLineColor(kBlue);
      gr->SetMarkerColor(kBlue);
      gr->SetLineWidth(2);
      mg.Add(gr, "3L");
    }
    {
      auto* gr = new TGraph(n, mean->time_.data(), mean->infected_.data());
      gr->SetTitle(" ");
      gr->SetLineColor(kRed);
      gr->SetMarkerColor(kRed);
      gr->SetLineWidth(2);
      mg.Add(gr, "3L");
    }
    {
      auto* gr = new TGraph(n, mean->time_.data(), mean->recovered_.data());
      gr->SetTitle("SIR sim mean");
      gr->SetLineColor(kGreen);
      gr->SetMarkerColor(kGreen);
      gr->SetLineWidth(2);
      mg.Add(gr, "3L");
    }
  }

  for (auto& rd : individual_rd) {
    {
      auto* gr = new TGraph(n, rd.time_.data(), rd.susceptible_.data());
      gr->SetTitle(" ");
      gr->SetLineColorAlpha(kBlue, 0.2);
      gr->SetMarkerColorAlpha(kBlue, 0.2);
      gr->SetLineWidth(1);
      mg.Add(gr, "L");
    }
    {
      auto* gr = new TGraph(n, rd.time_.data(), rd.infected_.data());
      gr->SetTitle(" ");
      gr->SetLineColorAlpha(kRed, 0.2);
      gr->SetMarkerColorAlpha(kRed, 0.2);
      gr->SetLineWidth(1);
      mg.Add(gr, "L");
    }
    {
      auto* gr = new TGraph(n, rd.time_.data(), rd.recovered_.data());
      gr->SetTitle("SIR single sim");
      gr->SetLineColorAlpha(kGreen, 0.2);
      gr->SetMarkerColorAlpha(kGreen, 0.2);
      gr->SetLineWidth(1);
      mg.Add(gr, "L");
    }
  }

  mg.SetMaximum(1.);

  FinalizePlot(&c, &mg, individual_rd.size(), Concat(folder, "/", filename),
               plot_legend);
}

// ---------------------------------------------------------------------------
inline void ResultDataToCsv(const ResultData& result,
                            const std::string& filename) {
  std::ofstream str(filename);
  str << "Time [h],Infected\n";
  for (uint64_t i = 0; i < result.infected_.size(); i++) {
    str << result.time_[i] << "," << result.infected_[i] << "\n";
  }
  str.close();
}

}  // namespace bdm

#endif  // EVALUATE_H_
