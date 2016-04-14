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
  enum Type {
    kSpine = 0,
    kBouton = 1,
    kSomaticSpine = 2,
    kShaft = 3
  };

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

  virtual std::shared_ptr<Excrescence> getEx() const {
    throw std::logic_error("Excrescence::getEx must not be called - Java must provide implementation");
  }

  virtual int getType() const {
    throw std::logic_error("Excrescence::getType must not be called - Java must provide implementation");
  }

  virtual std::array<double, 3> getProximalEnd() const {
    throw std::logic_error("Excrescence::getProximalEnd must not be called - Java must provide implementation");
  }

  virtual double getLength() const {
    throw std::logic_error("Excrescence::getLength must not be called - Java must provide implementation");
  }

  virtual bool synapseWith(const std::shared_ptr<Excrescence>& other, bool create_physical_bond) const {
      throw std::logic_error("Excrescence::getLength must not be called - Java must provide implementation");
    }
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_EXCRESCENCE_H_
