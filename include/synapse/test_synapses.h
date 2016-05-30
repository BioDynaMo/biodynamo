#ifndef SYNAPSE_TEST_SYNAPSES_H_
#define SYNAPSE_TEST_SYNAPSES_H_

#include <memory>

#include "simulation/ecm.h"

namespace cx3d {
namespace synapse {

class TestSynapses {
 public:
  static void extendExcressencesAndSynapseOnEveryNeuriteElement();

  static void extendExcressencesAndSynapseOnEveryNeuriteElement(double probability_to_synapse);
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_TEST_SYNAPSES_H_
