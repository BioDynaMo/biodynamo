#ifndef SYNAPSE_BIOLOGICAL_SOMATIC_SPINE_H_
#define SYNAPSE_BIOLOGICAL_SOMATIC_SPINE_H_

#include <memory>
#include <exception>
#include <string>

#include "sim_state_serializable.h"

namespace bdm {
namespace synapse {

class PhysicalSomaticSpine;

class BiologicalSomaticSpine : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<BiologicalSomaticSpine>;

  BiologicalSomaticSpine(TRootIOCtor*) { }  // only used for ROOT I/O

  BiologicalSomaticSpine();

  ~BiologicalSomaticSpine();

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  PhysicalSomaticSpine* getPhysicalSomaticSpine() const;

  void setPhysicalSomaticSpine(PhysicalSomaticSpine* ps);

 private:
  BiologicalSomaticSpine(const BiologicalSomaticSpine&) = delete;
  BiologicalSomaticSpine& operator=(const BiologicalSomaticSpine&) = delete;

  PhysicalSomaticSpine* physical_somatic_spine_ = nullptr;

  ClassDefOverride(BiologicalSomaticSpine, 1);
};

}  // namespace synapse
}  // namespace bdm

#endif  // SYNAPSE_BIOLOGICAL_SOMATIC_SPINE_H_
