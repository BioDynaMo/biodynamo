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
#ifndef CORE_ANALYSIS_LINE_GRAPH_H_
#define CORE_ANALYSIS_LINE_GRAPH_H_

#include <unordered_map>
#include <vector>
#include "core/util/root.h"

class TCanvas;
class TMultiGraph;

namespace bdm {
namespace experimental {

class TimeSeries;

class LineGraph {
 public:
  LineGraph(TimeSeries* ts, const std::string& title = "",
            const std::string& xaxis_title = "",
            const std::string& yaxis_title = "", int width = 350,
            int height = 250);

  void Add(const std::string& ts_name);

  void Finalize();
  void Draw(const char* option = "");

  TCanvas* GetTCanvas();
  TMultiGraph* GetTMultiGraph();

  // TODO SaveAs
 private:
  TimeSeries* ts_ = nullptr;
  TCanvas* c_ = nullptr;
  TMultiGraph* mg_ = nullptr;
};

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_ANALYSIS_TIME_SERIES_H_
