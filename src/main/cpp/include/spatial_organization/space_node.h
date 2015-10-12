#ifndef SPATIAL_ORGANIZATION_SPACE_NODE_H_
#define SPATIAL_ORGANIZATION_SPACE_NODE_H_

#include <array>
#include <stdexcept>
#include <string>

#include "spatial_organization/spatial_organization_node.h"

namespace cx3d {
namespace spatial_organization {

using cx3d::spatial_organization::SpatialOrganizationNode;

/**
 * This class is used to represent a node of a triangulation. Each node is
 * stores information about incident tetrahedra and edges (arbitrary amounts).
 *
 * @param <T> The type of the user objects associated with each node.
 */
template<class T>
class SpaceNode : public SpatialOrganizationNode<T> {
 public:
  virtual ~SpaceNode() {
  }

  virtual std::array<double, 3> getPosition() const override {
    throw std::logic_error(
        "getPosition must never be called - Java must provide implementation at this point");
  }

  /**
   * @return The identification number of this SpaceNode.
   */
  virtual int getId() {
    throw std::logic_error(
        "getId must never be called - Java must provide implementation at this point");
  }

  /**
   * Returns a string representation of this node.
   */
  virtual std::string toString() {
    throw std::logic_error(
        "toString must never be called - Java must provide implementation at this point");
  }
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_SPACE_NODE_H_
