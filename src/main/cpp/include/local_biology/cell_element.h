#ifndef LOCAL_BIOLOGY_CELL_ELEMENT_H_
#define LOCAL_BIOLOGY_CELL_ELEMENT_H_

#include <exception>
#include <string>

#include "sim_state_serializable.h"

namespace cx3d {
namespace local_biology {

class CellElement : public SimStateSerializable {
 public:
  virtual ~CellElement() {
  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override {
    throw std::logic_error("CellElement::simStateToJson must not be called - Java must provide implementation");
  }
};

}  // namespace local_biology
}  // namespace cx3d

#endif  // LOCAL_BIOLOGY_CELL_ELEMENT_H_
