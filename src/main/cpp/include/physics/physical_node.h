#ifndef PHYSICS_PHYSICAL_NODE_H_
#define PHYSICS_PHYSICAL_NODE_H_

#include <string>
#include <stdexcept>

namespace cx3d {
namespace physics {

class PhysicalNode {
 public:
  virtual ~PhysicalNode(){
  }

  virtual std::string toString() const {
    throw std::logic_error(
            "PhysicalNode::toString must never be called - Java must provide implementation at this point");
  }
};

}  // namespace physics
}  // namespace cx3d

#endif  // PHYSICS_PHYSICAL_NODE_H_
