#ifndef CELLS_CELL_H_
#define CELLS_CELL_H_

#include "sim_state_serializable.h"
#include "local_biology/soma_element.h"

namespace cx3d {
namespace cells {

class Cell : public SimStateSerializable {
 public:
  virtual ~Cell() {
  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override {
    throw std::logic_error("Cell::simStateToJson must not be called - Java must provide implementation");
  }

  virtual std::string toString() const {
    throw std::logic_error("Cell::toString must not be called - Java must provide implementation");
  }

  virtual std::shared_ptr<Cell> divide() const {
    throw std::logic_error("Cell::divide must not be called - Java must provide implementation");
  }

  virtual std::shared_ptr<local_biology::SomaElement> getSomaElement() const {
    throw std::logic_error("Cell::getSomaElement must not be called - Java must provide implementation");
  }
};

}  // cells
}  // cx3d

#endif  // CELLS_CELL_H_
