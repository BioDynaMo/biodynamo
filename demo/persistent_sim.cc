#include <omp.h>

#include <Rtypes.h>
#include <TFile.h>

#include "backend.h"
#include "cell.h"
#include "daosoa.h"
#include "io_util.h"

#define ROOTFILE "simulation_001.root"
#define DAOSOA "test_cells"
#define IOHELPER "test_ioh"

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define RESET "\x1b[0m"

using bdm::Cell;
using bdm::daosoa;
using bdm::VcBackend;
using bdm::ScalarBackend;

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

daosoa<Cell> intialize_cells(size_t cells_per_dim) {
  daosoa<Cell> cells(cells_per_dim * cells_per_dim * cells_per_dim);
  const unsigned space = 20;
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell<VcBackend> cell(
            std::array<VcBackend::real_v, 3>{i * space, j * space, k * space});
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
void simulate(daosoa<Cell>& cells, size_t distance, std::tuple<T...>& ioh) {
  for (size_t d = std::get<1>(ioh); d < distance; d++) {
#pragma omp parallel for
    for (size_t i = 0; i < cells.vectors(); i++) {
      std::array<VcBackend::real_v, 3> current_pos = cells[i].GetPosition();
      std::array<VcBackend::real_v, 3> new_pos{(current_pos[0] + 1.0),
                                               current_pos[1], current_pos[2]};
      cells[i].SetPosition(new_pos);
    }

    std::cout << cells[0].GetPosition()[0] << std::endl;

    // Copy Vc elements to persistent intermediaries
    cells.PersistData();
    // Save the current simulation step
    std::get<1>(ioh) = d;

    // Write objects to file
    bdm::WritePersistentObject(ROOTFILE, DAOSOA, cells, "RECREATE");
    bdm::WritePersistentObject(ROOTFILE, IOHELPER, ioh, "UPDATE");

    // Simulate the simulation overhead
    usleep(500000);
  }
}

int main(int args, char** argv) {
  if (args == 3) {
    size_t threads = 2;
    size_t cells_per_dim;
    size_t distance;

    std::istringstream(std::string(argv[1])) >> cells_per_dim;
    std::istringstream(std::string(argv[2])) >> distance;

    // Using std::tuple to persist checkpoint variables (native ROOT support)
    // std::tuple is a variadic data structure, so customizable per simulation
    std::tuple<size_t, size_t>* ioh;

    daosoa<Cell>* cells;

    // Try to find persisted objects from ROOTFILE
    if ((bdm::GetPersistentObject(ROOTFILE, DAOSOA, cells) &&
         bdm::GetPersistentObject(ROOTFILE, IOHELPER, ioh)) &&
        std::get<0>(*ioh) == cells_per_dim) {
      std::cout << GREEN "Retrieved persistent state. Continuing..." RESET
                << std::endl;

      // Load back data from persistent intermediaries to Vc elements
      cells->LoadData();

      // todo make sure that this is necessary
      std::get<1>(*ioh)++;
    } else {
      cells = new daosoa<Cell>();
      *cells = intialize_cells(cells_per_dim);
      size_t init = 0;
      auto empty_ioh = std::make_tuple(cells_per_dim, init);
      ioh = &empty_ioh;
      std::cout << RED "No persistent state found. Starting anew..." RESET
                << std::endl;
    }
    omp_set_num_threads(threads);
    simulate(*cells, distance, *ioh);
  } else {
    print_usage();
  }
  return 0;
}