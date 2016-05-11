#ifndef SYNAPSE_BIOLOGIGICAL_BOUTON_H_
#define SYNAPSE_BIOLOGIGICAL_BOUTON_H_

#include <memory>
#include <exception>
#include <string>

#include "sim_state_serializable.h"

namespace cx3d {
namespace synapse {

class PhysicalBouton;

class BiologicalBouton : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<BiologicalBouton>;

  BiologicalBouton();

  virtual ~BiologicalBouton();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual std::shared_ptr<PhysicalBouton> getPhysicalBouton() const;

  virtual void setPhysicalBouton(const std::shared_ptr<PhysicalBouton>& ps);

 private:
  BiologicalBouton(const BiologicalBouton&) = delete;
  BiologicalBouton& operator=(const BiologicalBouton&) = delete;

  std::shared_ptr<PhysicalBouton> physical_bouton_;
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_BIOLOGIGICAL_BOUTON_H_
