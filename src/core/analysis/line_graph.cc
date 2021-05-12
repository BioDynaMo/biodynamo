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
#include <TLegend.h>
#include <TMultiGraph.h>
#include <TStyle.h>
#include "core/analysis/time_series.h"
#include "core/util/log.h"
#include "core/util/string.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
LineGraph::LineGraph(TimeSeries* ts, const std::string& title,
                     const std::string& xaxis_title,
                     const std::string& yaxis_title, bool legend, TStyle* style,
                     int width, int height)
    : ts_(ts), c_(new TCanvas()), mg_(new TMultiGraph()), s_(style) {
  if (s_) {
    s_->cd();
  }
  c_->SetCanvasSize(width, height);
  c_->SetGrid();
  mg_->SetTitle(Concat(title, ";", xaxis_title, ";", yaxis_title).c_str());
  if (legend) {
    l_ = new TLegend();
  }
}

// -----------------------------------------------------------------------------
TGraph* LineGraph::Add(const std::string& ts_name,
                       const std::string& legend_label,
                       const char* add_mg_options, short line_color,
                       float line_color_alpha, short line_style,
                       short line_width, short marker_color,
                       float marker_color_alpha, short marker_style,
                       float marker_size, short fill_color,
                       float fill_color_alpha, short fill_style) {
  auto it = id_tgraph_map_.find(ts_name);
  if (it != id_tgraph_map_.end()) {
    Log::Warning("LineGraph::Add", "Graph with id (", ts_name,
                 ") has already been added. Operation aborted.");
    return nullptr;
  }
  if (s_) {
    s_->cd();
  }

  if (!ts_->Contains(ts_name)) {
    Log::Warning("LineGraph::Add",
                 "The time series stored in this line graph does not contain "
                 "an entry for (",
                 ts_name, "). Operation aborted.");
    return nullptr;
  }
  const auto& xvals = ts_->GetXValues(ts_name);
  const auto& yvals = ts_->GetYValues(ts_name);
  TGraph* gr = new TGraph(xvals.size(), xvals.data(), yvals.data());
  gr->SetTitle(legend_label.c_str());
  gr->InvertBit(TGraph::EStatusBits::kNotEditable);
  gr->SetLineColorAlpha(line_color, line_color_alpha);
  gr->SetLineStyle(line_style);
  gr->SetLineWidth(line_width);
  gr->SetMarkerColorAlpha(marker_color, marker_color_alpha);
  gr->SetMarkerStyle(marker_style);
  gr->SetMarkerSize(marker_size);
  gr->SetFillColorAlpha(fill_color, fill_color_alpha);
  gr->SetFillStyle(fill_style);

  mg_->Add(gr, add_mg_options);
  if (l_ && legend_label != "") {
    l_->AddEntry(gr, legend_label.c_str());
  }
  id_tgraph_map_[ts_name] = gr;
  return gr;
}

// -----------------------------------------------------------------------------
void LineGraph::Draw(const char* mg_draw_option,
                     const char* canvas_draw_option) {
  Finalize(mg_draw_option);
  c_->Draw(canvas_draw_option);
}

// -----------------------------------------------------------------------------
void LineGraph::SetLegendPos(double x1, double y1, double x2, double y2) {
  l_->SetX1(x1);
  l_->SetY1(y1);
  l_->SetX2(x2);
  l_->SetY2(y2);
}

// -----------------------------------------------------------------------------
void LineGraph::SetLegendPosNDC(double x1, double y1, double x2, double y2) {
  l_->SetX1NDC(x1);
  l_->SetY1NDC(y1);
  l_->SetX2NDC(x2);
  l_->SetY2NDC(y2);
}

// -----------------------------------------------------------------------------
void LineGraph::SaveAs(const std::string& filenpath_wo_extension,
                       const std::vector<std::string>& extensions,
                       const char* mg_draw_option) {
  Finalize(mg_draw_option);
  for (auto& ext : extensions) {
    auto full_path = Concat(filenpath_wo_extension, ext);
    Log::Info("LineGraph::SaveAs",
              Concat("Saved LineGraph at: ", full_path).c_str());
    c_->SaveAs(full_path.c_str());
  }
}

// -----------------------------------------------------------------------------
TCanvas* LineGraph::GetTCanvas() { return c_; }

// -----------------------------------------------------------------------------
TMultiGraph* LineGraph::GetTMultiGraph() { return mg_; }

// -----------------------------------------------------------------------------
TLegend* LineGraph::GetTLegend() { return l_; }

// -----------------------------------------------------------------------------
TGraph* LineGraph::GetTGraph(const std::string& ts_name) {
  auto it = id_tgraph_map_.find(ts_name);
  if (it == id_tgraph_map_.end()) {
    return nullptr;
  }
  return it->second;
}

// -----------------------------------------------------------------------------
TStyle* LineGraph::GetTStyle() { return s_; }

// -----------------------------------------------------------------------------
void LineGraph::Finalize(const char* mg_draw_option) {
  if (s_) {
    s_->cd();
  }
  mg_->Draw(mg_draw_option);
  l_->Draw();
  c_->Update();
  c_->GetFrame()->SetBorderSize(12);
  gPad->Modified();
  gPad->Update();
  c_->Modified();
  c_->cd(0);
}

}  // namespace experimental
}  // namespace bdm
