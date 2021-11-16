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

#ifndef PLOT_GRAPH_H_
#define PLOT_GRAPH_H_

#include <TAxis.h>
#include <TCanvas.h>
#include <TFrame.h>
#include <TGraph.h>

#include <atomic>
#include <vector>

template <typename T>
static void PlotGraph(std::vector<T> x, std::vector<T> y, std::string name) {
  assert(x.size() == y.size());
  int n = x.size();

  TCanvas *c = new TCanvas("c", name.c_str(), 200, 10, 700, 500);
  c->SetGrid();

  TGraph *gr = new TGraph(n, x.data(), y.data());
  gr->SetLineColor(2);
  gr->SetLineWidth(4);
  gr->SetTitle(name.c_str());
  gr->GetXaxis()->SetTitle("Timestep");
  gr->GetYaxis()->SetTitle("Activity");
  gr->Draw("ACP");

  // TCanvas::Update() draws the frame, after which one can change it
  c->Update();
  c->GetFrame()->SetBorderSize(12);
  c->Modified();
  c->SaveAs((name + ".jpg").c_str());
}

#endif  // PLOT_GRAPH_H_
