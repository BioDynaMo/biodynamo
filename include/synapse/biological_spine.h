#ifndef SYNAPSE_BIOLOGIGICAL_SPINE_H_
#define SYNAPSE_BIOLOGIGICAL_SPINE_H_

#include <memory>
#include <exception>
#include <string>

#include "sim_state_serializable.h"

namespace bdm {
namespace synapse {

class PhysicalSpine;

class BiologicalSpine : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<BiologicalSpine>;

  BiologicalSpine(TRootIOCtor*) { }  // only used for ROOT I/O

  BiologicalSpine();

  ~BiologicalSpine();

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  PhysicalSpine* getPhysicalSpine() const;

  void setPhysicalSpine(PhysicalSpine* ps);

 private:
  BiologicalSpine(const BiologicalSpine&) = delete;
  BiologicalSpine& operator=(const BiologicalSpine&) = delete;

  PhysicalSpine* physical_spine_ = nullptr;

  ClassDefOverride(BiologicalSpine, 1);
};

}  // namespace synapse
}  // namespace bdm

#endif  // SYNAPSE_BIOLOGIGICAL_SPINE_H_
