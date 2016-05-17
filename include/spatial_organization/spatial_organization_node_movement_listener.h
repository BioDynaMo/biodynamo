#ifndef SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_MOVEMENT_LISTENER_H_
#define SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_MOVEMENT_LISTENER_H_

#include <list>
#include <array>
#include <memory>

namespace cx3d {
namespace spatial_organization {

// forward declaration
template<class T> class SpaceNode;

//TODO change all occurences to SpatialOrganizationNode once porting has been finished
template<class T>
class SpatialOrganizationNodeMovementListener {
 public:
  SpatialOrganizationNodeMovementListener(){

  }
  virtual ~SpatialOrganizationNodeMovementListener() {
  }

  virtual void nodeAboutToMove(const std::shared_ptr<SpaceNode<T>>& node,
                               const std::array<double, 3>& planned_movement) = 0;

  virtual void nodeMoved(const std::shared_ptr<SpaceNode<T> >& node) = 0;

  virtual void nodeAboutToBeRemoved(const std::shared_ptr<SpaceNode<T> >& node) = 0;

  virtual void nodeRemoved(const std::shared_ptr<SpaceNode<T> >& node) = 0;

  virtual void nodeAboutToBeAdded(
      const std::shared_ptr<SpaceNode<T>>& node, const std::array<double, 3>& planned_position,
                                  const std::array<T*, 4>& vertices_of_the_tetrahedron_containing_the_position) = 0;

  virtual void nodeAdded(const std::shared_ptr<SpaceNode<T> >& node) = 0;

  /**
   * Returns a String representation of this SpatialOrganizationNodeMovementListener
   */
  virtual std::string toString() const = 0;

  virtual bool equalTo(const std::shared_ptr<SpatialOrganizationNodeMovementListener>& other) const {
    return this == other.get();
  }
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_MOVEMENT_LISTENER_H_
