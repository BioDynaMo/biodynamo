#ifndef PHYSICAL_NODE_H_
#define PHYSICAL_NODE_H_

#include <string>
#include <stdexcept>

namespace cx3d {

class PhysicalNode {
 public:
  virtual ~PhysicalNode(){
  }

  virtual std::string toString() const {
    throw std::logic_error(
            "PhysicalNode::toString must never be called - Java must provide implementation at this point");
  }
};

}  // namespace cx3d

#endif  // PHYSICAL_NODE_H_
