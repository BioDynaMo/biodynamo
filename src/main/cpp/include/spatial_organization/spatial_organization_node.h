#ifndef SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_H_
#define SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_H_

#include <array>

namespace cx3d {
namespace spatial_organization {

/**
 * Interface to define the basic properties of a node in the triangulation.
 *
 * @param <T> The type of user objects associated with each node in the triangulation.
 */
template<class T>
class SpatialOrganizationNode {
 public:
  virtual ~SpatialOrganizationNode() {
  }

  virtual std::array<double, 3> getPosition() const = 0;
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_H_
