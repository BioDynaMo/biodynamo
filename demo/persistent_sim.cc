#include <omp.h>
#include <unistd.h>

#include <Rtypes.h>
#include <TFile.h>

#include "cell.h"
#include "io_util.h"

#define ROOTFILE "simulation_001.root"
#define SOA "test_cells"
#define IOHELPER "test_ioh"

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define RESET "\x1b[0m"

using bdm::Cell;
using bdm::Soa;
using bdm::Scalar;

void print_usage() {
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

Cell<Soa> initialize_cells(size_t cells_per_dim) {
  const double space = 20;

  Cell<Soa> cells;
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

template <typename... T>
void simulate(Cell<Soa>& cells, size_t distance, std::tuple<T...>& ioh) {
  for (size_t d = std::get<1>(ioh); d < distance; d++) {
#pragma omp parallel for
    for (size_t i = 0; i < cells.size(); i++) {
      // The simulation operation is just a simple cell displacement and growth
      auto current_pos = cells[i].GetPosition();
      std::array<double, 3> new_pos{(current_pos[0] + 1.0),
                                    current_pos[1] + 1.0, current_pos[2] + 1.0};
      cells[i].SetPosition(new_pos);

      auto current_dia = cells[i].GetDiameter();
      cells[i].SetDiameter(current_dia + 1.0);
    }

    std::cout << "=======================================\n"
              << "cells[0].GetPosition() = [" << cells[0].GetPosition()[0]
              << ", " << cells[0].GetPosition()[1] << ", "
              << cells[0].GetPosition()[2] << "] " << std::endl

              << "cells[0].GetDiameter() = " << cells[0].GetDiameter()
              << std::endl;

    // Save the current simulation step
    std::get<1>(ioh) = d;

    // Write objects to file
    bdm::WritePersistentObject(ROOTFILE, SOA, cells, "RECREATE");
    bdm::WritePersistentObject(ROOTFILE, IOHELPER, ioh, "UPDATE");

    // Simulate the simulation overhead
    usleep(500000);
  }
}

bool check_final_state(Cell<Soa>& init_cells, Cell<Soa>& cells, size_t distance) {
  for (size_t i = 0; i < cells.size(); i++) {
    for (int j = 0; j < 3; j++)
      if (cells[i].GetPosition()[j] != (init_cells[i].GetPosition()[j] + distance))
        return false;
    if (cells[i].GetDiameter() != 30 + distance)
      return false;
  }
  return true;
}

int main(int args, char** argv) {
  if (args == 3) {
    size_t threads = 2;
    size_t cells_per_dim;
    size_t distance;

    std::istringstream(std::string(argv[1])) >> cells_per_dim;
    std::istringstream(std::string(argv[2])) >> distance;

    auto new_cells = initialize_cells(cells_per_dim);

    // Using std::tuple to persist checkpoint variables (native ROOT support)
    // std::tuple is a variadic data structure, so customizable per simulation
    std::tuple<size_t, size_t>* ioh;

    Cell<Soa>* cells;

    // Try to find persisted objects from ROOTFILE
    // NB: these *need* to be pointers
    if ((bdm::GetPersistentObject(ROOTFILE, SOA, cells) &&
         bdm::GetPersistentObject(ROOTFILE, IOHELPER, ioh)) &&
        std::get<0>(*ioh) == cells_per_dim) {
      std::cout << GREEN "Retrieved persistent state. Continuing..." RESET
                << std::endl;

      // todo make sure that this is necessary
      std::get<1>(*ioh)++;

      simulate(*cells, distance, *ioh);
      bool fin = check_final_state(new_cells, *cells, distance);

      if (fin)
        std::cout << GREEN "Resulting final state is correct" RESET
                  << std::endl;
      else
        std::cout << RED "Resulting final state is incorrect" RESET
                  << std::endl;
    } else {
      cells = &new_cells;

      auto empty_ioh = std::make_tuple(cells_per_dim, 0ul);
      ioh = &empty_ioh;

      std::cout << RED "No persistent state found. Starting anew..." RESET
                << std::endl;
      simulate(*cells, distance, *ioh);
    }
    omp_set_num_threads(threads);
  } else {
    print_usage();
  }
  return 0;
}
