#include "cell.h"
#include "inline_vector.h"
#include "neighbor_nanoflann_op.h"
#include "neighbor_op.h"
#include "neighbor_pcl_op.h"
#include "neighbor_unibn_op.h"

#include <chrono>
#include <fstream>

#include <sys/stat.h>
#include <sys/types.h>

using std::ofstream;
using bdm::Cell;
using bdm::NeighborOp;
using bdm::NeighborNanoflannOp;
using bdm::NeighborPclOp;
using bdm::NeighborUnibnOp;
using bdm::Scalar;

template <typename T, typename Op>
void RunTest(T* cells, const Op& op, char filename[100], int threads) {
  // execute and time neighbor operation
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  op.Compute(cells, filename);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  if (print_terminal == 1) {
    std::cout << "op.Compute           = " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
              << "ms\n"
              << "=============================================\n\n";
  }

  ofstream outfile;
  outfile.open(filename, std::ofstream::out | std::ofstream::app);
  outfile << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << ",";
  outfile << threads << "\n";
  outfile.close();
}

int main(int args, char** argv) {
  if(args == 4) {
    int threads;
    std::istringstream(std::string(argv[2])) >> threads;
    omp_set_num_threads(threads);

    size_t cells_per_dim;
    std::istringstream(std::string(argv[1])) >> cells_per_dim;

    srand((unsigned int) time(NULL));

    auto cells = Cell<>::NewEmptySoa();
    cells.reserve(cells_per_dim * cells_per_dim * cells_per_dim);
    
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

    if (print_terminal == 1) {
      std::cout << "Created grid of [" << (cells_per_dim*cells_per_dim*cells_per_dim) << "] cells\n" << std::endl;
    }
    
    const char* dirname = argv[3];
    mkdir(dirname, 0700);

    char neighbor_op_file[100];
    strcpy(neighbor_op_file, dirname);
    strcat(neighbor_op_file, "/Innopolis.txt");

    char neighbor_nf_file[100];
    strcpy(neighbor_nf_file, dirname);
    strcat(neighbor_nf_file, "/NanoflannOp.txt");

    char neighbor_pcl_file[100];
    strcpy(neighbor_pcl_file, dirname);
    strcat(neighbor_pcl_file, "/PCL.txt");

    char neighbor_unibn_file[100];
    strcpy(neighbor_unibn_file, dirname);
    strcat(neighbor_unibn_file, "/UniBn.txt");

    RunTest(&cells, NeighborOp(700), neighbor_op_file, threads);
    RunTest(&cells, NeighborNanoflannOp(700), neighbor_nf_file, threads);
    RunTest(&cells, NeighborPclOp(700), neighbor_pcl_file, threads);
    RunTest(&cells, NeighborUnibnOp(700), neighbor_unibn_file, threads);
  } else {
    std::cout << "Error args: ./octree_bench <cells_per_dim> <num_treads> <dirname>" << std::endl;
  }

  return 0;
}