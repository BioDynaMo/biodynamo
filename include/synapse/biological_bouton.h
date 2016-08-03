#ifndef SYNAPSE_BIOLOGIGICAL_BOUTON_H_
#define SYNAPSE_BIOLOGIGICAL_BOUTON_H_

#include <memory>
#include <exception>
#include <string>

#include "sim_state_serializable.h"

namespace bdm {
namespace synapse {

class PhysicalBouton;

class BiologicalBouton : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<BiologicalBouton>;

  BiologicalBouton(TRootIOCtor*) { }  // only used for ROOT I/O

  BiologicalBouton();

  ~BiologicalBouton();

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  PhysicalBouton* getPhysicalBouton() const;

  void setPhysicalBouton(PhysicalBouton* ps);

 private:
  BiologicalBouton(const BiologicalBouton&) = delete;
  BiologicalBouton& operator=(const BiologicalBouton&) = delete;

  PhysicalBouton* physical_bouton_ = nullptr;

  ClassDefOverride(BiologicalBouton, 1);
};

}  // namespace synapse
}  // namespace bdm

#endif  // SYNAPSE_BIOLOGIGICAL_BOUTON_H_
