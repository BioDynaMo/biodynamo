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

//TEST (DividingCellTest, simulation) {
//  cx3d::DividingCellTest simulation;
//  simulation.run();
//}
//
//TEST (DividingModuleTest, simulation) {
//  cx3d::DividingModuleTest simulation;
//  simulation.run();
//}

TEST (IntracellularDiffusionTest, simulation) {
  cx3d::IntracellularDiffusionTest simulation;
  simulation.run();
}

//TEST (MembraneContactTest, simulation) {
//  cx3d::MembraneContactTest simulation;
//  simulation.run();
//}
//
//TEST (NeuriteChemoAttractionTest, simulation) {
//  cx3d::NeuriteChemoAttractionTest simulation;
//  simulation.run();
//}
//
//TEST (RandomBranchingModuleTest, simulation) {
//  cx3d::RandomBranchingModuleTest simulation;
//  simulation.run();
//}
//
//TEST (SimpleSynapseTest, simulation) {
//  cx3d::SimpleSynapseTest simulation;
//  simulation.run();
//}
//
//TEST (SmallNetworkTest, simulation) {
//  cx3d::SmallNetworkTest simulation;
//  simulation.run();
//}
//
//TEST (SomaClusteringTest, simulation) {
//  cx3d::SomaClusteringTest simulation;
//  simulation.run();
//}
//
//TEST (SomaRandomWalkModuleTest, simulation) {
//  cx3d::SomaRandomWalkModuleTest simulation;
//  simulation.run();
//}
//
//TEST (Figure5Test, simulation) {
//  cx3d::Figure5Test simulation;
//  simulation.run();
//}
//
//TEST (Figure9Test, simulation) {
//  cx3d::Figure9Test simulation;
//  simulation.run();
//}

