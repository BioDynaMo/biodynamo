#include "backup_restore.h"
#include <omp.h>

int main(int argc, const char** argv) {
  omp_set_num_threads(1);
  return bdm::Simulate(argc, argv);
}
