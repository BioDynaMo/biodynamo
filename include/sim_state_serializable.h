#ifndef SIM_STATE_SERIALIZABLE_H_
#define SIM_STATE_SERIALIZABLE_H_

#include <Rtypes.h>

#include "string_builder.h"

namespace bdm {

using bdm::StringBuilder;

/**
 * Classes that implement that interface serialize their simulation state to
 * json with as little implementation details as possible (e.g. state of locks
 * or which collection implementation has been used)
 */
class SimStateSerializable {
 public:
  virtual ~SimStateSerializable() {
  }

  //SimStateSerializable(TRootIOCtor*) { }  // only used for ROOT I/O

  /**
   * This function is called after the simulation has finished to serialize the
   * simulation outcome to Json.
   * @param sb Append Json to this StringBuilder
   * @return The received StringBuilder to enable function concatenation
   */
  virtual StringBuilder& simStateToJson(StringBuilder& sb) const = 0;  // NOLINT(runtime/references

  ClassDef(SimStateSerializable, 1);
};

}  // namespace bdm

#endif  // SIM_STATE_SERIALIZABLE_H_
