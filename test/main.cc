#include <string>

#include "gtest/gtest.h"

#include "base_simulation_test.h"

using std::string;

int main(int argc, char **argv) {
    // manual processing for parameter updateSimStateReferenceFiles
    string param ="updateSimStateReferenceFiles";
    for(int i = 0; i < argc; i++) {
        if(!param.compare(argv[i])) {
            cx3d::BaseSimulationTest::update_sim_state_reference_file_ = true;
        }
    }

    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}