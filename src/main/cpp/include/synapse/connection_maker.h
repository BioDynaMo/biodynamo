#ifndef SYNAPSE_CONNECTION_MAKER_H_
#define SYNAPSE_CONNECTION_MAKER_H_

#include <memory>

#include "local_biology/neurite_element.h"
#include "physics/ecm.h"

namespace cx3d {
namespace synapse {

class ConnectionMaker {
 public:
  static void extendExcressencesAndSynapseOnEveryNeuriteElement(const std::shared_ptr<physics::ECM>& ecm);

  static void extendExcressencesAndSynapseOnEveryNeuriteElement(const std::shared_ptr<physics::ECM>& ecm,
                                                                double probability_to_synapse);
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_CONNECTION_MAKER_H_
