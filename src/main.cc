#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>

#include <omp.h>
// #include <ittnotify.h>

#include "backend.h"
#include "cell.h"
#include "daosoa.h"
#include "resource_manager.h"
#include "scheduler.h"
#include "timing.h"
#include "timing_aggregator.h"
#include "displacement_op.h"
#include "dividing_cell_op.h"

using bdm::Cell;
using bdm::daosoa;
using bdm::ResourceManager;
using bdm::Scheduler;
using bdm::ScalarBackend;
using bdm::Timing;
using bdm::TimingAggregator;
using bdm::VcBackend;

void execute(size_t cells_per_dim, size_t iterations, size_t threads,
             size_t repititions, TimingAggregator* statistic) {
  for (size_t r = 0; r < repititions; r++) {
    std::stringstream ss;
    ss << "measurement " << r << " - " << threads << " thread(s) - "
       << cells_per_dim << " cells per dim - " << iterations << " iteration(s)";
    statistic->AddDescription(ss.str());

    const unsigned space = 20;
    daosoa<Cell> cells(cells_per_dim * cells_per_dim * cells_per_dim);
    {
      Timing timing("Setup", statistic);
      for (size_t i = 0; i < cells_per_dim; i++) {
        for (size_t j = 0; j < cells_per_dim; j++) {
          for (size_t k = 0; k < cells_per_dim; k++) {
            // todo improve syntax
            Cell<ScalarBackend> cell(std::array<ScalarBackend::real_v, 3>{
                i * space, j * space, k * space});
            cell.SetDiameter(30);
            cell.SetAdherence(0.4);
            cell.SetMass(1.0);
            cell.UpdateVolume();
            cells.push_back(cell);
          }
        }
      }
    }

    {
      Timing timing("Find Neighbors", statistic);
      bdm::NeighborOp op(700);
      op.Compute(&cells);
    }

    // __itt_resume();

    {
      Timing timing("Cell Growth", statistic);
      bdm::DividingCellOp biology;
      for (size_t i = 0; i < iterations; i++) {
        biology.Compute(&cells);
      }
    }

    // __itt_pause();

    {
      Timing timing("Displacement", statistic);
      bdm::DisplacementOp op;
      for (size_t i = 0; i < iterations; i++) {
        op.Compute(&cells);
      }
    }
  }
}

void scaling(size_t cells_per_dim, size_t iterations, size_t repititions,
             TimingAggregator* statistic,
             const std::function<void(int&)> thread_inc = [](int& i) {
               i *= 2;
             }, const int max_threads = omp_get_max_threads()) {
  for (int i = 1; i <= max_threads; thread_inc(i)) {
    omp_set_num_threads(i);
    execute(cells_per_dim, iterations, i, repititions, statistic);
  }
}

int main(int args, char** argv) {
  std::cout << "Cell<VcBackend> size: " << sizeof(Cell<VcBackend>) << std::endl;

  TimingAggregator statistic;
  size_t repititions = 1;
  if (args >= 4) {
    size_t cells;
    size_t iterations;
    size_t threads;
    std::istringstream(std::string(argv[1])) >> cells;
    std::istringstream(std::string(argv[2])) >> iterations;
    std::istringstream(std::string(argv[3])) >> threads;
    if (args == 5) {
      std::istringstream(std::string(argv[4])) >> repititions;
    }
    omp_set_num_threads(threads);
    execute(cells, iterations, threads, repititions, &statistic);
  } else if (args >= 2 && std::string(argv[1]) == "--scaling") {
    if (args == 3) {
      std::istringstream(std::string(argv[2])) >> repititions;
    }
    scaling(128, 1, repititions, &statistic);
  } else if (args >= 2 && std::string(argv[1]) == "--detailed-scaling") {
    if (args == 3) {
      std::istringstream(std::string(argv[2])) >> repititions;
    }
    scaling(128, 1, repititions, &statistic, [](int& i) { i++; });
  } else {
    omp_set_num_threads(1);
    if (args == 2) {
      std::istringstream(std::string(argv[1])) >> repititions;
    }
    execute(8, 1e5, 1, repititions, &statistic);
  }
  std::cout << statistic << std::endl;
  return 0;
}
