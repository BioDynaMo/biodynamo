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

#ifndef HERTZ_INTERACTION_H_
#define HERTZ_INTERACTION_H_

#include <omp.h>
#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include "TAxis.h"
#include "TCanvas.h"
#include "TGaxis.h"
#include "TGraph.h"
#include "TH1.h"
#include "TLegend.h"
#include "TLine.h"
#include "TPad.h"
#include "TStyle.h"
#include "biodynamo.h"
#include "core/environment/uniform_grid_environment.h"
#include "core/interaction_force.h"
#include "core/operation/mechanical_forces_op.h"
#include "custom_ops.h"
#include "extended_hertz_force.h"
#include "moving_cell.h"
#include "sim_param.h"

namespace bdm {

using experimental::TimeSeries;

inline std::vector<double> ExtractNumbersFromLine(const std::string& line) {
  std::string sanitized = line;
  for (auto& c : sanitized) {
    if (!(std::isdigit(c) || c == '-' || c == '+' || c == '.' || c == 'e' ||
          c == 'E')) {
      c = ' ';
    }
  }

  std::stringstream ss(sanitized);
  std::vector<double> values;
  double value;
  while (ss >> value) {
    values.push_back(value);
  }
  return values;
}

inline bool PlotDistanceAndForceFromCsv(const std::string& position_csv,
                                        const std::string& force_csv) {
  std::ifstream pos_in(position_csv);
  std::ifstream force_in(force_csv);
  if (!pos_in.is_open() || !force_in.is_open()) {
    return false;
  }

  std::vector<double> time_distance;
  std::vector<double> distance;
  std::string line;
  while (std::getline(pos_in, line)) {
    auto values = ExtractNumbersFromLine(line);
    if (values.size() < 7) {
      continue;
    }
    // CSV layout: t, cell2(x,y,z), cell1(x,y,z)
    const double x1 = values[1];
    const double y1 = values[2];
    const double z1 = values[3];
    const double x2 = values[4];
    const double y2 = values[5];
    const double z2 = values[6];
    const double dist = std::sqrt(
        (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2));
    time_distance.push_back(values[0]);
    distance.push_back(dist);
  }

  std::vector<double> time_force;
  std::vector<double> force_magnitude;
  while (std::getline(force_in, line)) {
    auto values = ExtractNumbersFromLine(line);
    // CSV layout: t, (Fx, Fy, Fz, 0)
    if (values.size() < 4) {
      continue;
    }
    const double fx = values[1];
    const double fy = values[2];
    const double fz = values[3];
    const double fm = std::sqrt(fx * fx + fy * fy + fz * fz) * 1e12;
    time_force.push_back(values[0]);
    force_magnitude.push_back(fm);
  }

  if (time_distance.empty() || time_force.empty()) {
    return false;
  }

  const double dist_min = 8.0;
  const double dist_max = 12.0;

  const auto [force_min_it, force_max_it] =
      std::minmax_element(force_magnitude.begin(), force_magnitude.end());
  double force_max = *force_max_it;
  double force_min_pos = std::numeric_limits<double>::max();
  for (auto fm : force_magnitude) {
    if (fm > 0 && fm < force_min_pos) {
      force_min_pos = fm;
    }
  }
  if (force_min_pos == std::numeric_limits<double>::max()) {
    force_min_pos = 1e-6;
  }
  double force_min = 0.8 * force_min_pos;
  force_max = 1.2 * force_max;
  if (force_max <= force_min) {
    force_max = force_min * 10.0;
  }

  const double x_min = std::min(time_distance.front(), time_force.front());
  const double x_max = std::max(time_distance.back(), time_force.back());

  auto* canvas = new TCanvas("c_hertz", "Hertz Interaction", 1100, 700);
  gStyle->SetOptStat(0);

  auto* pad_distance = new TPad("pad_distance", "pad_distance", 0, 0, 1, 1);
  pad_distance->SetGrid();
  pad_distance->SetLeftMargin(0.12);
  pad_distance->SetRightMargin(0.14);
  pad_distance->Draw();
  pad_distance->cd();

  auto* distance_graph = new TGraph(static_cast<int>(time_distance.size()),
                                    time_distance.data(), distance.data());
  distance_graph->SetTitle(
      "Cell Distance and Force vs Time;Time index;Distance between centers "
      "[#mum]");
  distance_graph->SetLineColor(kBlue + 1);
  distance_graph->SetLineWidth(3);
  distance_graph->GetXaxis()->SetLimits(x_min, x_max);
  distance_graph->GetYaxis()->SetTitleColor(kBlue + 1);
  distance_graph->GetYaxis()->SetLabelColor(kBlue + 1);
  distance_graph->GetYaxis()->SetRangeUser(dist_min, dist_max);
  distance_graph->Draw("AL");

  auto* touching_line = new TLine(x_min, 10.0, x_max, 10.0);
  touching_line->SetLineStyle(2);
  touching_line->SetLineColor(kBlue + 2);
  touching_line->SetLineWidth(2);
  touching_line->Draw("SAME");

  canvas->cd();
  auto* pad_force = new TPad("pad_force", "pad_force", 0, 0, 1, 1);
  pad_force->SetFillStyle(4000);
  pad_force->SetFrameFillStyle(0);
  pad_force->SetLeftMargin(pad_distance->GetLeftMargin());
  pad_force->SetRightMargin(pad_distance->GetRightMargin());
  pad_force->SetTopMargin(pad_distance->GetTopMargin());
  pad_force->SetBottomMargin(pad_distance->GetBottomMargin());
  pad_force->SetLogy();
  pad_force->Draw();
  pad_force->cd();

  auto* force_graph = new TGraph(static_cast<int>(time_force.size()),
                                 time_force.data(), force_magnitude.data());
  force_graph->SetLineColor(kRed + 1);
  force_graph->SetLineWidth(3);
  auto* force_frame =
      pad_force->DrawFrame(x_min, force_min, x_max, force_max, "");
  force_frame->GetXaxis()->SetLabelSize(0);
  force_frame->GetXaxis()->SetTickLength(0);
  force_frame->GetYaxis()->SetLabelSize(0);
  force_frame->GetYaxis()->SetTickLength(0);
  force_frame->SetLineColor(0);
  force_frame->SetMarkerColor(0);
  force_graph->Draw("L SAME");

  pad_distance->cd();
  auto* axis_right = new TGaxis(x_max, dist_min, x_max, dist_max, force_min,
                                force_max, 510, "+LG");
  axis_right->SetTitle("Force magnitude [pN]");
  axis_right->SetTitleOffset(1.8);
  axis_right->SetLineColor(kRed + 1);
  axis_right->SetLabelColor(kRed + 1);
  axis_right->SetTitleColor(kRed + 1);
  axis_right->SetMoreLogLabels(true);
  axis_right->Draw();

  auto* legend = new TLegend(0.16, 0.78, 0.55, 0.9);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
  legend->AddEntry(distance_graph, "Distance between cell centers", "l");
  legend->AddEntry(touching_line, "2 radii = 10 #mum", "l");
  legend->AddEntry(force_graph, "Force magnitude [pN]", "l");
  legend->Draw();

  canvas->SaveAs("distance_force_vs_time.png");
  canvas->SaveAs("distance_force_vs_time.pdf");
  return true;
}

inline int Simulate(int argc, const char** argv) {
  // Set number of threads for OpenMP to 1 to avoid non-deterministic behavior
  // due to the fact that we track the positions of the cells at every time step
  // and thus have a data race
  omp_set_dynamic(0);
  omp_set_num_threads(1);

  // Adding space edge of but to be used in larger use case.
  auto set_param = [](Param* param) {
    param->use_progress_bar = true;
    param->bound_space = Param::BoundSpaceMode::kOpen;
    param->min_bound = -2000;
    param->max_bound = 2000;
    param->export_visualization = false;
    param->visualization_interval = 1;
    param->statistics = false;
    param->simulation_time_step = 0.0001;
    param->simulation_max_displacement = 100;  // 3 is the default value
    param->random_seed = 1234;  // Fixed seed for reproducible results
  };

  // Before we create a simulation we have to tell BioDynaMo about
  // the new parameters.
  Param::RegisterParamGroup(new SimParam());

  // Create a new simulation
  Simulation simulation(argc, argv, set_param);
  auto* ctxt = simulation.GetExecutionContext();  // Get the execution context
  auto* scheduler = simulation.GetScheduler();    // Get the scheduler
  auto* param = simulation.GetParam();            // Get the parameters
  const auto* sparam = param->Get<SimParam>();  // Get the simulation parameters

  Moving_cell* cell1 = new Moving_cell(sparam->cell1_position);
  Moving_cell* cell2 = new Moving_cell(sparam->cell2_position);

  real_t const cell_volume =
      4. / 3. * M_PI * pow(sparam->cell_diam / 2., 3);  // um^3
  real_t const cell_density = pow(10, -15);  // 1000 kg/m^3 = 10^-15kg/um^3
  real_t const cell_mass = cell_volume * cell_density;
  int number_of_cells = 2;

  cell1->SetDiameter(sparam->cell_diam);
  cell2->SetDiameter(sparam->cell_diam);
  cell1->SetMass(cell_mass);
  cell2->SetMass(cell_mass);

  cell1->SetId(0);
  cell2->SetId(1);

  ctxt->AddAgent(cell1);  // put the created cell in our cells structure
  ctxt->AddAgent(cell2);  // put the created cell in our cells structure

  // Custom force module
  auto* custom_force = new ExtendedHertzForce();
  auto* mech_op = scheduler->GetOps("mechanical forces")[0];
  auto* force_implementation = mech_op->GetImplementation<MechanicalForcesOp>();
  force_implementation->SetInteractionForce(custom_force);
  mech_op->frequency_ = 1;

  size_t time_steps =
      static_cast<size_t>(sparam->total_time / param->simulation_time_step);

  std::vector<std::vector<Double3>> cell_positions(number_of_cells);
  auto* track_pos_op = NewOperation("track_position");
  track_pos_op->GetImplementation<TrackPosition>()->positions_ =
      &cell_positions;
  track_pos_op->frequency_ =
      1;  // every 1 -> timestep 0.1 min, every 10 -> timestep 0.01 min, every
          // 100 -> timestep 0.001 min
  scheduler->ScheduleOp(track_pos_op);

  std::vector<Double4> forces;
  auto* track_force_op = NewOperation("track_force");
  track_force_op->GetImplementation<TrackForce>()->forces_ = &forces;
  track_force_op->GetImplementation<TrackForce>()->agent1_uid_ =
      cell2->GetUid();
  track_force_op->GetImplementation<TrackForce>()->agent2_uid_ =
      cell1->GetUid();
  track_force_op->GetImplementation<TrackForce>()->interaction_force_ =
      custom_force;
  track_force_op->frequency_ =
      1;  // every 1 -> timestep 0.1 min, every 10 -> timestep 0.01 min, every
          // 100 -> timestep 0.001 min
  scheduler->ScheduleOp(track_force_op);

  // Run simulation
  simulation.GetScheduler()->Simulate(time_steps);
  std::cout << "Simulation completed successfully!" << std::endl;

  std::ofstream position_file;
  if (!position_file.is_open()) {
    position_file.open("cell_positions.csv");
  }

  // Add initial positions
  position_file << 0 << "\t " << sparam->cell2_position << "\t "
                << sparam->cell1_position << std::endl;

  for (size_t j = 0; j < cell_positions[0].size(); j++) {  // time points
    position_file << j + 1;                                // time point
    for (size_t i = 0; i < cell_positions.size(); i++) {
      position_file << "\t " << cell_positions[i][j];
    }
    position_file << std::endl;
  }

  position_file.close();

  std::ofstream force_file;
  if (!force_file.is_open()) {
    force_file.open("cell_forces.csv");
  }

  for (size_t j = 0; j < forces.size(); j++) {  // time points
    force_file << j + 1 << "\t " << forces[j];
    force_file << std::endl;
  }

  force_file.close();

  if (!PlotDistanceAndForceFromCsv("cell_positions.csv", "cell_forces.csv")) {
    std::cout << "Could not generate ROOT plot from CSV files." << std::endl;
  }

  return 0;
}

}  // namespace bdm

#endif  // HERTZ_INTERACTION_H_
