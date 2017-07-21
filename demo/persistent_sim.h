#ifndef DEMO_PERSISTANT_SIM_H_
#define DEMO_PERSISTANT_SIM_H_

#include <unistd.h>

#include "cell.h"
#include "io_util.h"

#define ROOTFILE "simulation_001.root"
#define SOA "test_cells"
#define IOHELPER "test_io_helper"
#define RUNVARS "test_rv"

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define RESET "\x1b[0m"

using bdm::Cell;
using bdm::Soa;
using bdm::Scalar;
using bdm::RuntimeVariables;

inline void PrintUsage() {
  std::cout
      << "USAGE\n"
      << "  ./persistent_sim #cells_per_dim #distance |\n"
      << "\nDESCRIPTION\n"
      << "  Creates a three dimensional grid of cells of size "
         "(#cells_per_dim)^3,\n"
      << "  and moves them unidirectional for the purpose of demonstrating "
         "data persistency\n"
      << "  Feel free to crash the simulation and restart it to experience "
         "some fault tolerance\n"
      << "\nOPTIONS\n"
      << "  --help\n"
      << "    Explains usage of this binary and its command line options\n"
      << std::endl;
}

inline Cell<Soa> InitializeCells(size_t cells_per_dim) {
  const double space = 20;

  auto cells = Cell<>::NewEmptySoa();
  cells.reserve(cells_per_dim * cells_per_dim * cells_per_dim);
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell<Scalar> cell({i * space, j * space, k * space});
        cell.SetDiameter(30);
        cell.SetAdherence(0.4);
        cell.SetMass(1.0);
        cell.UpdateVolume();
        cells.push_back(cell);
      }
    }
  }
  return cells;
}

template <typename Cells, typename... T>
inline void Simulate(Cells* cells, size_t distance,
                     std::tuple<T...>* io_helper) {
  for (size_t d = std::get<1>(*io_helper); d < distance; d++) {
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      // The simulation operation is just a simple cell displacement and growth
      auto&& cell = (*cells)[i];
      auto current_pos = cell.GetPosition();
      std::array<double, 3> new_pos{(current_pos[0] + 1.0),
                                    current_pos[1] + 1.0, current_pos[2] + 1.0};
      cell.SetPosition(new_pos);

      auto current_dia = cell.GetDiameter();
      cell.SetDiameter(current_dia + 1.0);
    }

    std::cout << "=======================================\n"
              << "cells[0].GetPosition() = [" << (*cells)[0].GetPosition()[0]
              << ", " << (*cells)[0].GetPosition()[1] << ", "
              << (*cells)[0].GetPosition()[2] << "] " << std::endl

              << "cells[0].GetDiameter() = " << (*cells)[0].GetDiameter()
              << std::endl;

    // Save the current simulation step
    std::get<1>(*io_helper) = d;

    // Write objects to file
    bdm::WritePersistentObject(ROOTFILE, SOA, *cells, "UPDATE");
    bdm::WritePersistentObject(ROOTFILE, IOHELPER, *io_helper, "UPDATE");

    // Simulate the simulation overhead
    usleep(500000);
  }
}

inline bool CheckFinalState(const Cell<Soa>& init_cells, const Cell<Soa>& cells,
                            size_t distance) {
  for (size_t i = 0; i < cells.size(); i++) {
    for (int j = 0; j < 3; j++) {
      if (cells[i].GetPosition()[j] !=
          (init_cells[i].GetPosition()[j] + distance)) {
        return false;
      }
    }
    if (cells[i].GetDiameter() != 30 + distance) {
      return false;
    }
  }
  return true;
}

inline int Run(int args, char** argv) {
  if (args != 3) {
    PrintUsage();
    return -1;
  }

  size_t cells_per_dim = 1;
  size_t distance = 10;

  std::istringstream(std::string(argv[1])) >> cells_per_dim;
  std::istringstream(std::string(argv[2])) >> distance;

  auto new_cells = InitializeCells(cells_per_dim);

  // Using std::tuple to persist checkpoint variables (native ROOT support)
  // std::tuple is a variadic data structure, so customizable per simulation
  std::tuple<size_t, size_t>* io_helper;

  Cell<Soa>* cells;

  RuntimeVariables rv;

  // Try to find persisted objects from ROOTFILE
  // NB: these *need* to be pointers
  if ((bdm::GetPersistentObject(ROOTFILE, SOA, cells) &&
       bdm::GetPersistentObject(ROOTFILE, IOHELPER, io_helper)) &&
      std::get<0>(*io_helper) == cells_per_dim) {
    std::cout << GREEN "Retrieved persistent state. Continuing..." RESET
              << std::endl;

    RuntimeVariables* rv_comp = nullptr;
    bdm::GetPersistentObject(ROOTFILE, RUNVARS, rv_comp);
    if (!(rv == *rv_comp)) {
      std::cout << "WARN: Running simulation on a different system!"
                << std::endl;
    }

    std::get<1>(*io_helper)++;

    Simulate(cells, distance, io_helper);
    bool fin = CheckFinalState(new_cells, *cells, distance);

    if (fin) {
      std::cout << GREEN "Resulting final state is correct" RESET << std::endl;
    } else {
      std::cout << RED "Resulting final state is incorrect" RESET << std::endl;
    }
  } else {
    cells = &new_cells;
    std::cout << "Diameter: " << (*cells)[0].GetDiameter() << std::endl;

    auto empty_io_helper = std::make_tuple(cells_per_dim, 0ul);
    io_helper = &empty_io_helper;

    bdm::WritePersistentObject(ROOTFILE, RUNVARS, rv, "RECREATE");

    std::cout << RED "No persistent state found. Starting anew..." RESET
              << std::endl;
    Simulate(cells, distance, io_helper);
  }
  return 0;
}

#endif  // DEMO_PERSISTANT_SIM_H_
