#ifndef SIM_STATE_SERIALIZABLE_H_
#define SIM_STATE_SERIALIZABLE_H_

#include "string_builder.h"

namespace cx3d {

using cx3d::StringBuilder;

/**
 * Classes that implement that interface serialize their simulation state to
 * json with as little implementation details as possible (e.g. state of locks
 * or which collection implementation has been used)
 */
class SimStateSerializable {
 public:
  virtual ~SimStateSerializable() {
  }

  /**
   * This function is called after the simulation has finished to serialize the
   * simulation outcome to Json.
   * @param sb Append Json to this StringBuilder
   * @return The received StringBuilder to enable function concatenation
   */
  virtual StringBuilder& simStateToJson(StringBuilder& sb) const = 0;  // NOLINT(runtime/references)
};

}  // namespace cx3d

#endif  // SIM_STATE_SERIALIZABLE_H_
