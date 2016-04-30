#include "synapse/biological_spine.h"

#include "synapse/physical_spine.h"

namespace cx3d {
namespace synapse {

std::shared_ptr<BiologicalSpine> BiologicalSpine::create() {
  return std::shared_ptr<BiologicalSpine>(new BiologicalSpine());
}

BiologicalSpine::BiologicalSpine() {
}

BiologicalSpine::~BiologicalSpine() {
}

bool BiologicalSpine::equalTo(const std::shared_ptr<BiologicalSpine>& other) const {
  return this == other.get();
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
