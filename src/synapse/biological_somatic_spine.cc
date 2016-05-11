#include "synapse/biological_somatic_spine.h"

#include "synapse/physical_somatic_spine.h"

namespace cx3d {
namespace synapse {

BiologicalSomaticSpine::BiologicalSomaticSpine() {
}

BiologicalSomaticSpine::~BiologicalSomaticSpine() {
}

StringBuilder& BiologicalSomaticSpine::simStateToJson(StringBuilder& sb) const {
  sb.append("{}");
  return sb;
}

std::shared_ptr<PhysicalSomaticSpine> BiologicalSomaticSpine::getPhysicalSomaticSpine() const {
  return physical_somatic_spine_;
}

void BiologicalSomaticSpine::setPhysicalSomaticSpine(const std::shared_ptr<PhysicalSomaticSpine>& ps) {
  physical_somatic_spine_ = ps;
}

}  // namespace synapse
}  // namespace cx3d
