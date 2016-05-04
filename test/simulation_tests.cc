#include "gtest/gtest.h"

#include "dividing_cell_test.h"
#include "dividing_module_test.h"
#include "intracellular_diffusion_test.h"
#include "membrane_contact_test.h"
#include "neurite_chemo_attraction_test.h"
#include "random_branching_module_test.h"
#include "simple_synapse_test.h"
#include "small_network_test.h"
#include "soma_clustering_test.h"
#include "soma_random_walk_module_test.h"
#include "figure_5_test.h"
#include "figure_9_test.h"

namespace cx3d {

TEST_F (DividingCellTest, simulation) {
  run();
}

TEST_F (DividingModuleTest, simulation) {
  run();
}

TEST_F (IntracellularDiffusionTest, simulation) {
  run();
}

TEST_F (MembraneContactTest, simulation) {
  run();
}

TEST_F (NeuriteChemoAttractionTest, simulation) {
  run();
}

TEST_F (RandomBranchingModuleTest, simulation) {
  run();
}

TEST_F (SimpleSynapseTest, simulation) {
  run();
}

TEST_F (SmallNetworkTest, simulation) {
  run();
}

TEST_F (SomaClusteringTest, simulation) {
  run();
}

TEST_F (SomaRandomWalkModuleTest, simulation) {
  run();
}

TEST_F (Figure5Test, simulation) {
  run();
}

TEST_F (Figure9Test, simulation) {
  run();
}

}  // namespace cx3d
