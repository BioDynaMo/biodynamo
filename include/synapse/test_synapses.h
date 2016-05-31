#ifndef SYNAPSE_TEST_SYNAPSES_H_
#define SYNAPSE_TEST_SYNAPSES_H_

#include <memory>

#include "simulation/ecm.h"

namespace bdm {
namespace synapse {

class TestSynapses {
 public:
  static void extendExcressencesAndSynapseOnEveryNeuriteElement();

  static void extendExcressencesAndSynapseOnEveryNeuriteElement(double probability_to_synapse);
};

}  // namespace synapse
}  // namespace bdm

#endif  // SYNAPSE_TEST_SYNAPSES_H_
