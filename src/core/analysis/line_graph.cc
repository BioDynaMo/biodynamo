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

#include "core/analysis/line_graph.h"
#include <TCanvas.h>
#include <TFrame.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include "core/analysis/time_series.h"
#include "core/util/string.h"

namespace bdm {
namespace experimental {

LineGraph::LineGraph(TimeSeries* ts, const std::string& title,
                     const std::string& xaxis_title,
                     const std::string& yaxis_title, int width, int height)
    : ts_(ts), c_(new TCanvas()), mg_(new TMultiGraph()) {
  c_->SetCanvasSize(width, height);
  c_->SetGrid();
  c_->SetRightMargin(0.05);
  c_->SetLeftMargin(0.2);
  c_->SetTopMargin(0.04);
  c_->SetBottomMargin(0.2);
  mg_->SetTitle(Concat(title, ";", xaxis_title, ";", yaxis_title).c_str());
}

void LineGraph::Add(const std::string& ts_name) {
  const auto& xvals = ts_->GetXValues(ts_name);
  const auto& yvals = ts_->GetYValues(ts_name);
  TGraph* gr = new TGraph(xvals.size(), xvals.data(), yvals.data());
  mg_->Add(gr, "L");
}

void LineGraph::Finalize() {
  mg_->Draw("A");
  c_->BuildLegend();

  c_->Update();
  c_->GetFrame()->SetBorderSize(12);
  gPad->Modified();
  gPad->Update();
  c_->Modified();
  c_->cd(0);
}

void LineGraph::Draw(const char* option) { c_->Draw(option); }

TCanvas* LineGraph::GetTCanvas() { return c_; }
TMultiGraph* LineGraph::GetTMultiGraph() { return mg_; }

}  // namespace experimental
}  // namespace bdm
