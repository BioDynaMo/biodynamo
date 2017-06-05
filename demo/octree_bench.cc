#include "cell.h"
#include "inline_vector.h"
#include "neighbor_nanoflann_op.h"
#include "neighbor_op.h"
#include "neighbor_pcl_op.h"
#include "neighbor_unibn_op.h"

#include <chrono>
#include <fstream>

using std::ofstream;
using bdm::Cell;
using bdm::NeighborOp;
using bdm::NeighborNanoflannOp;
using bdm::NeighborPclOp;
using bdm::NeighborUnibnOp;
using bdm::Scalar;

template <typename T, typename Op>
void RunTest(T* cells, const Op& op, const char* filename) {
  // execute and time neighbor operation
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  op.Compute(cells);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  // std::cout << "op.Compute = " 
  //           << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
  //           << "us\n\n";

  ofstream outfile;
  outfile.open(filename, std::ofstream::out | std::ofstream::app);
  outfile << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "\n";
  outfile.close();
}

int main(int args, char** argv) {
  if(args == 3) {
    int threads;
    std::istringstream(std::string(argv[2])) >> threads;
    omp_set_num_threads(threads);

    size_t cells_per_dim;
    std::istringstream(std::string(argv[1])) >> cells_per_dim;

    srand((unsigned int) time(NULL));

    auto cells = Cell<>::NewEmptySoa();
    cells.reserve(cells_per_dim * cells_per_dim * cells_per_dim);

    // // create array of cells
    // size_t num_cells = cells_per_dim * cells_per_dim * cells_per_dim;
    // for (int i = 0; i < num_cells; i++) {
    //   double x = 3000.0f * rand () / (RAND_MAX + 1.0f);
    //   double y = 3000.0f * rand () / (RAND_MAX + 1.0f);
    //   double z = 3000.0f * rand () / (RAND_MAX + 1.0f);
    //   cells.push_back(Cell<>({x, y, z}));
    // }
    
    const double space = 20;
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

    std::cout << "Finished creating array of cells" << std::endl;

    RunTest(&cells, NeighborOp(700), "NeighborOp.txt");
    RunTest(&cells, NeighborNanoflannOp(700), "NeighborNanoflannOp.txt");
    RunTest(&cells, NeighborPclOp(700), "NeighborPclOp.txt");
    RunTest(&cells, NeighborUnibnOp(700), "NeighborUnibnOp.txt");
  } else {
    std::cout << "Error: Specify the number of cells as the argument" << std::endl;
  }

  return 0;
}