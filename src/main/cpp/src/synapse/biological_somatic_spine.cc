#include "synapse/biological_somatic_spine.h"

#include "synapse/physical_somatic_spine.h"

namespace cx3d {
namespace synapse {

std::shared_ptr<BiologicalSomaticSpine> BiologicalSomaticSpine::create() {
  return std::shared_ptr<BiologicalSomaticSpine>(new BiologicalSomaticSpine());
}

BiologicalSomaticSpine::BiologicalSomaticSpine() {
}

BiologicalSomaticSpine::~BiologicalSomaticSpine() {
}

bool BiologicalSomaticSpine::equalTo(const std::shared_ptr<BiologicalSomaticSpine>& other) const {
  return this == other.get();
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
