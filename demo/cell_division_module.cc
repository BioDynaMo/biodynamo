#include "cell_division_module.h"

int main(int argc, const char** argv) {
  auto options = bdm::DefaultSimulationOptionParser(argc, argv);
  return bdm::Simulate(options);
}
