#ifndef SYNAPSE_CONNECTION_MAKER_H_
#define SYNAPSE_CONNECTION_MAKER_H_

#include <memory>

#include "simulation/ecm.h"

namespace cx3d {
namespace synapse {

class ConnectionMaker {
 public:
  static void extendExcressencesAndSynapseOnEveryNeuriteElement();

  static void extendExcressencesAndSynapseOnEveryNeuriteElement(double probability_to_synapse);
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_CONNECTION_MAKER_H_
