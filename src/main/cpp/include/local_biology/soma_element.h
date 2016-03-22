#ifndef LOCAL_BIOLOGY_SOMA_ELEMENT_H_
#define LOCAL_BIOLOGY_SOMA_ELEMENT_H_

#include <exception>
#include <string>

#include "cell_element.h"

namespace cx3d {
namespace local_biology {

class SomaElement : public CellElement {
 public:
  virtual ~SomaElement() {
  }
};

}  // namespace local_biology
}  // namespace cx3d

#endif  // LOCAL_BIOLOGY_SOMA_ELEMENT_H_
