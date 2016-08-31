#ifndef SYNAPSE_PHYSICAL_SPINE_H_
#define SYNAPSE_PHYSICAL_SPINE_H_

#include <exception>
#include <string>

#include "synapse/excrescence.h"
#include "biological_spine.h"

namespace bdm {
namespace synapse {

class BiologicalSpine;

class PhysicalSpine : public Excrescence {
 public:
  using UPtr = std::unique_ptr<PhysicalSpine>;

  PhysicalSpine();

  PhysicalSpine(PhysicalObject* po, const std::array<double, 2>& origin, double length);

  virtual ~PhysicalSpine();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual bool synapseWith(Excrescence* other, bool create_physical_bond) override;

  virtual bool synapseWithSoma(Excrescence* other_excrescence, bool create_phyiscal_bond) override;

  virtual bool synapseWithShaft(NeuriteElement* other_ne, double max_dis, int nr_segments, bool create_phyiscal_bond)
      override;

  BiologicalSpine* getBiologicalSpine() const;

  void setBiologicalSpine(BiologicalSpine::UPtr spine);

 private:
  PhysicalSpine(const PhysicalSpine&) = delete;
  PhysicalSpine& operator=(const PhysicalSpine&) = delete;

  BiologicalSpine::UPtr biological_spine_ = nullptr;

  ClassDefOverride(PhysicalSpine, 1);
};

}  // namespace synapse
}  // namespace bdm

#endif  // SYNAPSE_PHYSICAL_SPINE_H_
