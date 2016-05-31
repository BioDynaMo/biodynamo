#include "synapse/biological_bouton.h"

#include "synapse/physical_bouton.h"

namespace bdm {
namespace synapse {

BiologicalBouton::BiologicalBouton() {
}

BiologicalBouton::~BiologicalBouton() {
}

StringBuilder& BiologicalBouton::simStateToJson(StringBuilder& sb) const {
  sb.append("{}");
  return sb;
}

PhysicalBouton* BiologicalBouton::getPhysicalBouton() const {
  return physical_bouton_;
}

void BiologicalBouton::setPhysicalBouton(PhysicalBouton* ps) {
  physical_bouton_ = ps;
}

}  // namespace synapse
}  // namespace bdm
