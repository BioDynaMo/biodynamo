#ifndef SYNAPSE_BIOLOGIGICAL_SOMATIC_SPINE_H_
#define SYNAPSE_BIOLOGIGICAL_SOMATIC_SPINE_H_

#include <memory>
#include <exception>
#include <string>

#include "sim_state_serializable.h"

namespace cx3d {
namespace synapse {

class PhysicalSomaticSpine;

class BiologicalSomaticSpine : public SimStateSerializable {
 public:
  static std::shared_ptr<BiologicalSomaticSpine> create();

  BiologicalSomaticSpine();

  virtual ~BiologicalSomaticSpine();

  virtual bool equalTo(const std::shared_ptr<BiologicalSomaticSpine>& other) const;

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual std::shared_ptr<PhysicalSomaticSpine> getPhysicalSomaticSpine() const;

  virtual void setPhysicalSomaticSpine(const std::shared_ptr<PhysicalSomaticSpine>& ps);

 private:
  BiologicalSomaticSpine(const BiologicalSomaticSpine&) = delete;
  BiologicalSomaticSpine& operator=(const BiologicalSomaticSpine&) = delete;

  std::shared_ptr<PhysicalSomaticSpine> physical_somatic_spine_;
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_BIOLOGIGICAL_SOMATIC_SPINE_H_
