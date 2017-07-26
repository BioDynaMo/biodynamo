#include "backup_restore.h"
#include <omp.h>

int main(int argc, const char** argv) {
  omp_set_num_threads(1);
  auto options = bdm::DefaultSimulationOptionParser(argc, argv);
  return bdm::Simulate(options);
}
