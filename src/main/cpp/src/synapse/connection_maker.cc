#include "synapse/connection_maker.h"

#include "local_biology/neurite_element.h"

namespace cx3d {
namespace synapse {

void ConnectionMaker::extendExcressencesAndSynapseOnEveryNeuriteElement(const std::shared_ptr<simulation::ECM>& ecm) {
  extendExcressencesAndSynapseOnEveryNeuriteElement(ecm, 1.0);
}

void ConnectionMaker::extendExcressencesAndSynapseOnEveryNeuriteElement(const std::shared_ptr<simulation::ECM>& ecm,
                                                                        double probability_to_synapse) {
  for (auto ne : ecm->getNeuriteElementList()) {
    if (ne->isAxon()) {
      ne->makeBoutons(2);
    } else {
      ne->makeSpines(5);
    }
  }
  for (auto ne : ecm->getNeuriteElementList()) {
    if (ne->isAxon()) {
      if (ecm->getRandomDouble1() < probability_to_synapse) {
        ne->synapseBetweenExistingBS(probability_to_synapse);
      }
    }
  }
}

}  // namespace synapse
}  // namespace cx3d
