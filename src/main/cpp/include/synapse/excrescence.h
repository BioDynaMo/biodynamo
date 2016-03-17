#ifndef SYNAPSE_EXCRESCENCE_H_
#define SYNAPSE_EXCRESCENCE_H_

#include <exception>
#include <string>

#include "sim_state_serializable.h"

namespace cx3d {
namespace synapse {

class Excrescence : public SimStateSerializable {
 public:
  virtual ~Excrescence() {
  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override {
    throw std::logic_error("Excrescence::simStateToJson must not be called - Java must provide implementation");
  }
};

}  // namespace synapse
}  // namespace cx3d

#endif  // SYNAPSE_EXCRESCENCE_H_
