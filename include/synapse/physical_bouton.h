#ifndef SYNAPSE_PHYSICAL_BOUTON_H_
#define SYNAPSE_PHYSICAL_BOUTON_H_

#include <memory>

#include "synapse/excrescence.h"
#include "biological_bouton.h"

namespace bdm {
namespace synapse {

class BiologicalBouton;

class PhysicalBouton : public Excrescence {
 public:
  using UPtr = std::unique_ptr<PhysicalBouton>;

  PhysicalBouton();

  PhysicalBouton(PhysicalObject* po, const std::array<double, 2>& origin, double length);

  virtual ~PhysicalBouton();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual bool synapseWith(Excrescence* other, bool create_physical_bond) override;

  virtual bool synapseWithSoma(Excrescence* other_excrescence, bool create_phyiscal_bond) override;

  virtual bool synapseWithShaft(NeuriteElement* other_ne, double max_dis, int nr_segments, bool create_phyiscal_bond)
      override;

  void setBiologicalBouton(BiologicalBouton::UPtr bouton);

  BiologicalBouton* getBiologicalBouton() const;

 private:
  PhysicalBouton(const PhysicalBouton&) = delete;
  PhysicalBouton& operator=(const PhysicalBouton&) = delete;

#ifdef __ROOTCLING__
  BiologicalBouton* biological_bouton_ = nullptr;
#else
  BiologicalBouton::UPtr biological_bouton_ = nullptr;
#endif

  ClassDefOverride(PhysicalBouton, 1);
};

}  // namespace synapse
}  // namespace bdm

#endif  // SYNAPSE_PHYSICAL_BOUTON_H_
