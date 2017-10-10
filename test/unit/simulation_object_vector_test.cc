#include "simulation_object_vector_test.h"

namespace bdm {
namespace simulation_object_vector_test_internal {

TEST(SimulationObjectVector, All) { RunTest(); }

TEST(SimulationObjectVector, InitializeSuccessors) { RunInitializeTest(); }

}  // namespace simulation_object_vector_test_internal
}  // namespace bdm
