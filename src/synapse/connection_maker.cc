#include "synapse/connection_maker.h"

namespace cx3d {
namespace synapse {

void ConnectionMaker::extendExcressencesAndSynapseOnEveryNeuriteElement() {
  extendExcressencesAndSynapseOnEveryNeuriteElement(1.0);
}

void ConnectionMaker::extendExcressencesAndSynapseOnEveryNeuriteElement(double probability_to_synapse) {
  for (auto ne : ECM::getInstance()->getNeuriteElementList()) {
    if (ne->isAxon()) {
      ne->makeBoutons(2);
    } else {
      ne->makeSpines(5);
    }
  }
  for (auto ne : ECM::getInstance()->getNeuriteElementList()) {
    if (ne->isAxon()) {
      if (Random::nextDouble() < probability_to_synapse) {
        ne->synapseBetweenExistingBS(probability_to_synapse);
      }
    }
  }
}

}  // namespace synapse
}  // namespace cx3d
