#ifndef DEMO_CELL_GROWTH_H_
#define DEMO_CELL_GROWTH_H_

#include <omp.h>
#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>

#include "backend.h"
#include "cell.h"
#include "displacement_op.h"
#include "dividing_cell_op.h"
#include "exporter.h"
#include "neighbor_nanoflann_op.h"
#include "neighbor_op.h"
#include "resource_manager.h"
#include "scheduler.h"
#include "timing.h"
#include "timing_aggregator.h"
// #include <ittnotify.h>

using bdm::Cell;
using bdm::Scalar;
using bdm::Soa;
using bdm::Timing;
using bdm::TimingAggregator;
using bdm::ExporterFactory;

//namespace bdm {

  // void Execute(size_t cells_per_dim, size_t iterations, size_t threads,
  //  size_t repititions, TimingAggregator *statistic,
  //  bool with_export) {
  //   for (size_t r = 0; r < repititions; r++) {
  //     std::stringstream ss;
  //     ss << "measurement " << r << " - " << threads << " thread(s) - "
  //     << cells_per_dim << " cells per dim - " << iterations << " iteration(s)";
  //     statistic->AddDescription(ss.str());

  //     const double space = 20;

  //     auto cells = Cell<>::NewEmptySoa();
  //     cells.reserve(cells_per_dim * cells_per_dim * cells_per_dim);
  //     {
  //       Timing timing("Setup", statistic);
  //       for (size_t i = 0; i < cells_per_dim; i++) {
  //         for (size_t j = 0; j < cells_per_dim; j++) {
  //           for (size_t k = 0; k < cells_per_dim; k++) {
  //             Cell<Scalar> cell({i * space, j * space, k * space});
  //             cell.SetDiameter(30);
  //             cell.SetAdherence(0.4);
  //             cell.SetMass(1.0);
  //             cell.UpdateVolume();
  //             cells.push_back(cell);
  //           }
  //         }
  //       }
  //     }

  //   // iterate for all (time) steps
  //     ExporterFactory exp_fac;
  //     auto exp_basic = exp_fac.GenerateExporter<Cell<Soa>>("basic");
  //     auto exp_matlab = exp_fac.GenerateExporter<Cell<Soa>>("matlab");
  //     auto exp_neuroml = exp_fac.GenerateExporter<Cell<Soa>>("neuroml");
  //     auto exp_paraview = exp_fac.GenerateExporter<Cell<Soa>>("paraview");

  //     if (with_export) {
  //     //exporter.CreatePVDFile("Results4Paraview", iterations, 1.0);
  //       exp_paraview->CreatePVDFile("Results4Paraview", iterations, 1.0);
  //     }

  //     for (size_t i = 0; i < iterations; i++) {
  //       {
  //         Timing timing("Find Neighbors", statistic);
  //         bdm::NeighborOp op(700);
  //         op.Compute(&cells);
  //       }

  //     // __itt_resume();

  //       {
  //         Timing timing("Cell Growth", statistic);
  //         bdm::DividingCellOp biology;
  //         biology.Compute(&cells);
  //       }

  //     // __itt_pause();

  //       {
  //         Timing timing("Displacement", statistic);
  //         bdm::DisplacementOp op;
  //         op.Compute(&cells);
  //       }

  //       if (with_export) {
  //         Timing timing("Export", statistic);
  //         std::cout << "exporting now..." << std::endl;
  //         exp_basic->ToFile(cells, "FinalPositions.dat");
  //         exp_matlab->ToFile(cells, "FinalPositions.m");
  //         exp_neuroml->ToFile(cells, "FinalPositions.xml");
  //         exp_paraview->ToFile(cells, "Results4Paraview");
  //         exp_paraview->AddIteration();
  //       }
  //     }
  //   }
  // }

  // void Scaling(size_t cells_per_dim, size_t iterations, size_t repititions,
  //  TimingAggregator *statistic, bool with_export,
  //  const std::function<void(int &)> thread_inc =
  //                [](int &i) {  // NOLINT(runtime/references)
  //                  i *= 2;
  //                },
  //                const int max_threads = omp_get_max_threads()) {
  //   for (int i = 1; i <= max_threads; thread_inc(i)) {
  //     omp_set_num_threads(i);
  //     Execute(cells_per_dim, iterations, i, repititions, statistic, with_export);
  //   }
  // }

//}  // namespace bdm

#endif  // DEMO_CELL_GROWTH_H_