#include "cell_clustering.h"

int main(int argc, const char** argv) {
  // auto options = bdm::DefaultSimulationOptionParser(argc, argv);
  return bdm::Simulate(std::stoi(argv[1]));
}
