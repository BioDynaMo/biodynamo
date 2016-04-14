#ifndef SYNAPSE_PHYSICAL_BOUTON_H_
#define SYNAPSE_PHYSICAL_BOUTON_H_

#include <exception>
#include <string>

#include "synapse/excrescence.h"

namespace cx3d {
namespace synapse {

class BiologicalBouton;

class PhysicalBouton : public Excrescence {
 public:
  PhysicalBouton() {
  }

  virtual ~PhysicalBouton() {
  }

  virtual void setBiologicalBouton(const std::shared_ptr<BiologicalBouton>& bouton) {
    throw std::logic_error("PhysicalBouton::setBiologicalBouton must not be called");
  }

 private:
  PhysicalBouton(const PhysicalBouton&) = delete;
  PhysicalBouton& operator=(const PhysicalBouton&) = delete;
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_PHYSICAL_BOUTON_H_
