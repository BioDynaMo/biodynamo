#ifndef SYNAPSE_PHYSICAL_SPINE_H_
#define SYNAPSE_PHYSICAL_SPINE_H_

#include <exception>
#include <string>

#include "synapse/excrescence.h"

namespace cx3d {
namespace synapse {

class BiologicalSpine;

class PhysicalSpine : public Excrescence {
 public:
  PhysicalSpine() {
  }

  virtual ~PhysicalSpine() {
  }

  virtual void setBiologicalSpine(const std::shared_ptr<BiologicalSpine>& spine) {
    throw std::logic_error("PhysicalSpine::setBiologicalSpine must not be called");
  }

 private:
  PhysicalSpine(const PhysicalSpine&) = delete;
  PhysicalSpine& operator=(const PhysicalSpine&) = delete;
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_PHYSICAL_SPINE_H_
