#ifndef SYNAPSE_BIOLOGIGICAL_SPINE_H_
#define SYNAPSE_BIOLOGIGICAL_SPINE_H_

#include <memory>
#include <exception>
#include <string>

#include "sim_state_serializable.h"

namespace cx3d {
namespace synapse {

class PhysicalSpine;

class BiologicalSpine : public SimStateSerializable {
 public:
  static std::shared_ptr<BiologicalSpine> create();

  BiologicalSpine();

  virtual ~BiologicalSpine();

  virtual bool equalTo(const std::shared_ptr<BiologicalSpine>& other) const;

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual std::shared_ptr<PhysicalSpine> getPhysicalSpine() const;

  virtual void setPhysicalSpine(const std::shared_ptr<PhysicalSpine>& ps);

 private:
  BiologicalSpine(const BiologicalSpine&) = delete;
  BiologicalSpine& operator=(const BiologicalSpine&) = delete;

  std::shared_ptr<PhysicalSpine> physical_spine_;
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_BIOLOGIGICAL_SPINE_H_
