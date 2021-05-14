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

#include <string>
#include <unordered_map>
#include <vector>
#include "core/util/root.h"

class TCanvas;
class TMultiGraph;
class TLegend;
class TGraph;
class TStyle;

namespace bdm {
namespace experimental {

class TimeSeries;

class LineGraph {
 public:
  /// \param style   This class does not take ownership of style
  LineGraph(const TimeSeries* ts, const std::string& title = "",
            const std::string& xaxis_title = "",
            const std::string& yaxis_title = "", bool legend = true,
            TStyle* style = nullptr, int width = 700, int height = 500);

  ~LineGraph();

  // TODO
  /// The following links provide further information for line, marker, and fill
  /// parameters: \see https://root.cern/doc/master/classTAttLine.html \see
  /// https://root.cern/doc/master/classTAttMarker.html \see
  /// https://root.cern/doc/master/classTAttFill.html
  TGraph* Add(const std::string& ts_name, const std::string& legend_name = "",
              const char* add_mg_options = "L", short line_color = 1,
              float line_color_alpha = 1.0, short line_style = 1,
              short line_width = 1, short marker_color = 1,
              float marker_color_alpha = 1.0, short marker_style = 1,
              float marker_size = 1, short fill_color = 0,
              float fill_color_alpha = 1.0, short fill_style = 1000);

  void SetLegendPos(double x1, double y1, double x2, double y2);
  /// NDC coordinates are a % of the canvas size:
  /// (0.5,0.5) is the middle of the canvas. (1,1) upper right corner,
  /// (0,0) bottom left corner.
  /// \ref
  /// https://root-forum.cern.ch/t/how-to-imagine-ndc-normalized-coord/24202
  void SetLegendPosNDC(double x1, double y1, double x2, double y2);

  void SetMultiGraphDrawOption(const std::string& s);

  void Draw(const char* canvas_draw_option = "");

  void SaveAs(const std::string& filenpath_wo_extension,
              const std::vector<std::string>& extensions);

  void Update();

  TCanvas* GetTCanvas();
  TMultiGraph* GetTMultiGraph();
  TLegend* GetTLegend();
  TGraph* GetTGraph(const std::string& ts_name);
  TStyle* GetTStyle();

 private:
  const TimeSeries* ts_ = nullptr;
  std::string mg_draw_option_ = "A";
  TCanvas* c_ = nullptr;
  TMultiGraph* mg_ = nullptr;
  TLegend* l_ = nullptr;
  TStyle* s_ = nullptr;
  std::unordered_map<std::string, TGraph*> id_tgraph_map_;
};

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_ANALYSIS_TIME_SERIES_H_
