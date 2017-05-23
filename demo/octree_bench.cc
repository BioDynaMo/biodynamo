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
  if(args == 2) {
    omp_set_num_threads(omp_get_max_threads());
    srand((unsigned int) time(NULL));
    int num_cells;

    std::istringstream(std::string(argv[1])) >> num_cells;

    auto cells = Cell<>::NewEmptySoa();

    // create array of cells
    for (int i = 0; i < num_cells; i++) {
      double x = 64.0f * rand () / (RAND_MAX + 1.0f);
      double y = 64.0f * rand () / (RAND_MAX + 1.0f);
      double z = 64.0f * rand () / (RAND_MAX + 1.0f);
      cells.push_back(Cell<>({x, y, z}));
    }

    RunTest(&cells, NeighborOp(), "NeighborOp.txt");
    RunTest(&cells, NeighborNanoflannOp(), "NeighborNanoflannOp.txt");
    RunTest(&cells, NeighborPclOp(), "NeighborPclOp.txt");
    RunTest(&cells, NeighborUnibnOp(), "NeighborUnibnOp.txt");
  } else {
    std::cout << "Error: Specify the number of cells as the argument" << std::endl;
  }

  return 0;
}