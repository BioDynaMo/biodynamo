#include <cmath>
#include "random.h"

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

namespace bdm {

TEST (Math, exp) {
  double x = -0.0028724514195400627;
  double result = exp(x);
  double expected = 0.99713167012178527;
  ASSERT_EQ(expected, result);
}

TEST (Random, all) {
  Random::setSeed(1L);
  ASSERT_TRUE(std::abs(0.7308781907032908 - Random::nextDouble()) < 1e-20);
  ASSERT_TRUE(std::abs(0.41008081149220166 - Random::nextDouble()) < 1e-20);
  for(int i = 0; i < 10000000; i++) Random::nextDouble();
  ASSERT_TRUE(std::abs(0.06991942722947553 - Random::nextDouble()) < 1e-20);
  ASSERT_TRUE(std::abs(-0.3648863101313806 - Random::nextGaussian(0, 1)) < 1e-20);
  for(int i = 0; i < 100000; i++) Random::nextGaussian(2, 3);
  ASSERT_TRUE(0.4512373907254288 - Random::nextGaussian(3, 4) < 1e-20);
  Random::setSeed(99L);
  ASSERT_TRUE(std::abs(0.7224575488195071 - Random::nextDouble()) < 1e-20);
  ASSERT_TRUE(std::abs(0.9246892004845302 - Random::nextGaussian(3, 4)) < 1e-20);
  auto noise = Random::nextNoise(100);
  ASSERT_TRUE(std::abs(73.40813456108145 - noise[0]) < 1e-20);
  ASSERT_TRUE(std::abs(53.51089851859078 - noise[1]) < 1e-20);
  ASSERT_TRUE(std::abs(-48.27938452667355 - noise[2]) < 1e-20);
}

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

}
  // namespace bdm
