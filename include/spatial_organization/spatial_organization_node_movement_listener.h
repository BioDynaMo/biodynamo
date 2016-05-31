#ifndef SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_MOVEMENT_LISTENER_H_
#define SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_MOVEMENT_LISTENER_H_

#include <array>
#include <memory>

namespace bdm {
namespace spatial_organization {

// forward declaration
template<class T> class SpatialOrganizationNode;

template<class T>
class SpatialOrganizationNodeMovementListener {
 public:
  using UPtr = typename std::unique_ptr<SpatialOrganizationNodeMovementListener<T>>;

  SpatialOrganizationNodeMovementListener() {
  }

  virtual ~SpatialOrganizationNodeMovementListener() {
  }

  virtual UPtr getCopy() const = 0;

  virtual void nodeAboutToMove(const SpatialOrganizationNode<T>* node,
                               const std::array<double, 3>& planned_movement) = 0;

  virtual void nodeMoved(const SpatialOrganizationNode<T>* node) = 0;

  virtual void nodeAboutToBeRemoved(const SpatialOrganizationNode<T>* node) = 0;

  virtual void nodeRemoved(const SpatialOrganizationNode<T>* node) = 0;

  virtual void nodeAboutToBeAdded(const SpatialOrganizationNode<T>*, const std::array<double, 3>& planned_position,
                                  const std::array<T*, 4>& vertices_of_the_tetrahedron_containing_the_position) = 0;

  virtual void nodeAdded(const SpatialOrganizationNode<T>* node) = 0;

  /**
   * Returns a String representation of this SpatialOrganizationNodeMovementListener
   */
  virtual std::string toString() const = 0;
};

}  // namespace spatial_organization
}  // namespace bdm

#endif  // SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_MOVEMENT_LISTENER_H_
