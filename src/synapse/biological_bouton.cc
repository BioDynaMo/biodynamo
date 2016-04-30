#include "synapse/biological_bouton.h"

#include "synapse/physical_bouton.h"

namespace cx3d {
namespace synapse {

std::shared_ptr<BiologicalBouton> BiologicalBouton::create() {
  return std::shared_ptr<BiologicalBouton>(new BiologicalBouton());
}

BiologicalBouton::BiologicalBouton() {
}

BiologicalBouton::~BiologicalBouton() {
}

bool BiologicalBouton::equalTo(const std::shared_ptr<BiologicalBouton>& other) const {
  return this == other.get();
}

StringBuilder& BiologicalBouton::simStateToJson(StringBuilder& sb) const {
  sb.append("{}");
  return sb;
}

std::shared_ptr<PhysicalBouton> BiologicalBouton::getPhysicalBouton() const {
  return physical_bouton_;
}

void BiologicalBouton::setPhysicalBouton(const std::shared_ptr<PhysicalBouton>& ps) {
  physical_bouton_ = ps;
}

}  // namespace synapse
}  // namespace cx3d
