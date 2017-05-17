#include <omp.h>
#include <unistd.h>
#include <cstdlib>

#include "cell.h"
#include "bdmAdaptor.h"

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

void simulate(Cell<Soa>& cells, size_t numberOfTimeSteps) {
  srand(42);
  for (size_t timeStep = 0; timeStep < numberOfTimeSteps; timeStep++) {
#pragma omp parallel for
    for (size_t i = 0; i < cells.size(); i++) {
      // The simulation operation is just a simple cell displacement and growth
      auto current_pos = cells[i].GetPosition();
      double r = -1.0 + static_cast <double> (rand()) /( static_cast <double> (RAND_MAX/(2)));
      std::array<double, 3> new_pos{(current_pos[0] + r),
                                    current_pos[1] + r, current_pos[2] + r};

      cells[i].SetPosition(new_pos);

      // auto current_dia = cells[i].GetDiameter();
      // cells[i].SetDiameter(current_dia + 1.0);
    }
    // timestep of 0.1
    double time = 0.1*timeStep;
    bdmAdaptor::CoProcess(cells, time, timeStep, timeStep == numberOfTimeSteps - 1);

    // Simulate the simulation overhead
    std::cout << "Simulating..." << std::endl;
    usleep(500000);
  }
}

int main(int args, char** argv) {
  if (args == 4) {
    size_t threads = 2;
    size_t cells_per_dim;
    size_t duration;

    std::istringstream(std::string(argv[1])) >> cells_per_dim;
    std::istringstream(std::string(argv[2])) >> duration;

#ifdef USE_CATALYST
  // initialize with a single script (for this demo)
  bdmAdaptor::Initialize(argv[3]);
#endif

    auto new_cells = initialize_cells(cells_per_dim);

    Cell<Soa>* cells = &new_cells;

    omp_set_num_threads(threads);
    simulate(*cells, duration);

#ifdef USE_CATALYST
  bdmAdaptor::Finalize();
#endif
  } else {
    print_usage();
  }
  return 0;
}
