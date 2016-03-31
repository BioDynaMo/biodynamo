#ifndef SYNAPSE_EXCRESCENCE_H_
#define SYNAPSE_EXCRESCENCE_H_

#include <exception>
#include <string>

#include "sim_state_serializable.h"

namespace cx3d {

namespace physics {
class PhysicalObject;
}  // namespace physics

namespace synapse {

class Excrescence : public SimStateSerializable {
 public:
  virtual ~Excrescence() {
  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override {
    throw std::logic_error("Excrescence::simStateToJson must not be called - Java must provide implementation");
  }

  virtual std::array<double, 2> getPositionOnPO() const {
    throw std::logic_error("Excrescence::getPositionOnPO must not be called - Java must provide implementation");
  }

  virtual void setPositionOnPO(const std::array<double, 2>& position) const {
    throw std::logic_error("Excrescence::setPositionOnPO must not be called - Java must provide implementation");
  }

  virtual void setPo(const std::shared_ptr<physics::PhysicalObject>& po) {
    throw std::logic_error("Excrescence::setPo must not be called - Java must provide implementation");
  }
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_EXCRESCENCE_H_
