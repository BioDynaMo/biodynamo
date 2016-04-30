#ifndef SYNAPSE_PHYSICAL_SOMATIC_SPINE_H_
#define SYNAPSE_PHYSICAL_SOMATIC_SPINE_H_

#include <exception>
#include <string>

#include "synapse/excrescence.h"

namespace cx3d {
namespace synapse {

class BiologicalSomaticSpine;

class PhysicalSomaticSpine : public Excrescence {
 public:
  static std::shared_ptr<PhysicalSomaticSpine> create();

  static std::shared_ptr<PhysicalSomaticSpine> create(const std::shared_ptr<physics::PhysicalObject>& po,
                                               const std::array<double, 2>& origin, double length);

  PhysicalSomaticSpine();

  PhysicalSomaticSpine(const std::shared_ptr<physics::PhysicalObject>& po, const std::array<double, 2>& origin, double length);

  virtual ~PhysicalSomaticSpine();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual bool synapseWith(const std::shared_ptr<Excrescence>& other, bool create_physical_bond) override;

  virtual bool synapseWithSoma(const std::shared_ptr<Excrescence>& other_excrescence, bool create_phyiscal_bond)
      override;

  virtual bool synapseWithShaft(const std::shared_ptr<local_biology::NeuriteElement>& other_ne, double max_dis,
                                int nr_segments, bool create_phyiscal_bond) override;

  virtual std::shared_ptr<BiologicalSomaticSpine> getBiologicalSomaticSpine() const;

  virtual void setBiologicalSomaticSpine(const std::shared_ptr<BiologicalSomaticSpine>& spine);

 private:
  PhysicalSomaticSpine(const PhysicalSomaticSpine&) = delete;
  PhysicalSomaticSpine& operator=(const PhysicalSomaticSpine&) = delete;

  std::shared_ptr<BiologicalSomaticSpine> biological_spine_;
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_PHYSICAL_SOMATIC_SPINE_H_
