#include <string>

#include "gtest/gtest.h"

#include "base_simulation_test.h"

using std::string;

int main(int argc, char **argv) {
  // manual processing for additional parameters
  string update_references = "--update-references";
  string disable_assertions = "--disable-assertions";
  for (int i = 0; i < argc; i++) {
    if (!update_references.compare(argv[i])) {
      bdm::BaseSimulationTest::update_sim_state_reference_file_ = true;
    } else if (!disable_assertions.compare(argv[i])) {
      bdm::BaseSimulationTest::disable_assertions_ = true;
    }
  }

  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
