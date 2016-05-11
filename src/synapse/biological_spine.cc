#include "synapse/biological_spine.h"

#include "synapse/physical_spine.h"

namespace cx3d {
namespace synapse {

BiologicalSpine::BiologicalSpine() {
}

BiologicalSpine::~BiologicalSpine() {
}

StringBuilder& BiologicalSpine::simStateToJson(StringBuilder& sb) const {
  sb.append("{}");
  return sb;
}

std::shared_ptr<PhysicalSpine> BiologicalSpine::getPhysicalSpine() const {
  return physical_spine_;
}

void BiologicalSpine::setPhysicalSpine(const std::shared_ptr<PhysicalSpine>& ps) {
  physical_spine_ = ps;
}

}  // namespace synapse
}  // namespace cx3d
