#ifndef SYNAPSE_PHYSICAL_BOUTON_H_
#define SYNAPSE_PHYSICAL_BOUTON_H_

#include <memory>

#include "synapse/excrescence.h"

namespace cx3d {
namespace synapse {

class BiologicalBouton;

class PhysicalBouton : public Excrescence {
 public:
  static std::shared_ptr<PhysicalBouton> create();

  static std::shared_ptr<PhysicalBouton> create(const std::shared_ptr<physics::PhysicalObject>& po,
                                                const std::array<double, 2>& origin, double length);

  PhysicalBouton();

  PhysicalBouton(const std::shared_ptr<physics::PhysicalObject>& po, const std::array<double, 2>& origin,
                 double length);

  virtual ~PhysicalBouton();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual bool synapseWith(const std::shared_ptr<Excrescence>& other, bool create_physical_bond) override;

  virtual bool synapseWithSoma(const std::shared_ptr<Excrescence>& other_excrescence, bool create_phyiscal_bond)
      override;

  virtual bool synapseWithShaft(const std::shared_ptr<local_biology::NeuriteElement>& other_ne, double max_dis,
                                int nr_segments, bool create_phyiscal_bond) override;

  virtual void setBiologicalBouton(const std::shared_ptr<BiologicalBouton>& bouton);

  virtual std::shared_ptr<BiologicalBouton> getBiologicalBouton() const;

 private:
  PhysicalBouton(const PhysicalBouton&) = delete;
  PhysicalBouton& operator=(const PhysicalBouton&) = delete;

  std::shared_ptr<BiologicalBouton> biological_bouton_;
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_PHYSICAL_BOUTON_H_
