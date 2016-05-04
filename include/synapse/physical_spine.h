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
  static std::shared_ptr<PhysicalSpine> create();

  static std::shared_ptr<PhysicalSpine> create(const std::shared_ptr<physics::PhysicalObject>& po,
                                               const std::array<double, 2>& origin, double length);

  PhysicalSpine();

  PhysicalSpine(const std::shared_ptr<physics::PhysicalObject>& po, const std::array<double, 2>& origin, double length);

  virtual ~PhysicalSpine();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual bool synapseWith(const std::shared_ptr<Excrescence>& other, bool create_physical_bond) override;

  virtual bool synapseWithSoma(const std::shared_ptr<Excrescence>& other_excrescence, bool create_phyiscal_bond)
      override;

  virtual bool synapseWithShaft(const std::shared_ptr<local_biology::NeuriteElement>& other_ne, double max_dis,
                                int nr_segments, bool create_phyiscal_bond) override;

  virtual std::shared_ptr<BiologicalSpine> getBiologicalSpine() const;

  virtual void setBiologicalSpine(const std::shared_ptr<BiologicalSpine>& spine);

 private:
  PhysicalSpine(const PhysicalSpine&) = delete;
  PhysicalSpine& operator=(const PhysicalSpine&) = delete;

  std::shared_ptr<BiologicalSpine> biological_spine_;
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_PHYSICAL_SPINE_H_
