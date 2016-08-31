#ifndef SYNAPSE_PHYSICAL_SOMATIC_SPINE_H_
#define SYNAPSE_PHYSICAL_SOMATIC_SPINE_H_

#include <exception>
#include <string>

#include "synapse/excrescence.h"
#include "synapse/biological_somatic_spine.h"

namespace bdm {
namespace synapse {

class PhysicalSomaticSpine : public Excrescence {
 public:
  using UPtr = std::unique_ptr<PhysicalSomaticSpine>;

  PhysicalSomaticSpine();

  PhysicalSomaticSpine(PhysicalObject* po, const std::array<double, 2>& origin, double length);

  virtual ~PhysicalSomaticSpine();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual bool synapseWith(Excrescence* other, bool create_physical_bond) override;

  virtual bool synapseWithSoma(Excrescence* other_excrescence, bool create_phyiscal_bond) override;

  virtual bool synapseWithShaft(NeuriteElement* other_ne, double max_dis, int nr_segments, bool create_phyiscal_bond)
      override;

  BiologicalSomaticSpine* getBiologicalSomaticSpine() const;

  void setBiologicalSomaticSpine(BiologicalSomaticSpine::UPtr spine);

 private:
  PhysicalSomaticSpine(const PhysicalSomaticSpine&) = delete;
  PhysicalSomaticSpine& operator=(const PhysicalSomaticSpine&) = delete;

  BiologicalSomaticSpine::UPtr biological_spine_ = nullptr;

  ClassDefOverride(PhysicalSomaticSpine, 1);
};

}  // namespace synapse
}  // namespace bdm

#endif  // SYNAPSE_PHYSICAL_SOMATIC_SPINE_H_
