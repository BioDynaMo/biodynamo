#ifndef SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_H_
#define SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_H_

#include <string>
#include <array>
#include <memory>
#include <vector>

#include "sim_state_serializable.h"
#include "string_builder.h"
#include "spatial_organization/spatial_organization_node_movement_listener.h"

namespace bdm {
namespace spatial_organization {

template<class T> class SpatialOrganizationEdge;

/**
 * Interface to define the basic properties of a node in the triangulation.
 *
 * @param <T> The type of user objects associated with each node in the triangulation.
 */
template<class T>
class SpatialOrganizationNode : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<SpatialOrganizationNode<T>>;

  virtual ~SpatialOrganizationNode() {
  }

  virtual void addSpatialOrganizationNodeMovementListener(
      typename SpatialOrganizationNodeMovementListener<T>::UPtr listener) = 0;

  /**
   * Returns a list that allows to iterate over all edges
   * incident to this node.
   */
  virtual std::vector<SpatialOrganizationEdge<T>*> getEdges() const = 0;

  virtual std::vector<T*> getNeighbors() const = 0;

  virtual std::unique_ptr<SpatialOrganizationNode<T>> getNewInstance(const std::array<double, 3>& position,
                                                                     T* user_object) = 0;

  virtual std::vector<T*> getPermanentListOfNeighbors() const = 0;

  virtual std::array<double, 3> getPosition() const = 0;

  virtual T* getUserObject() const = 0;

  virtual std::array<T*, 4> getVerticesOfTheTetrahedronContaining(const std::array<double, 3>& position,
                                                                  bool& returned_null) const = 0;

  virtual double getVolume() const = 0;

  virtual void moveFrom(const std::array<double, 3>& delta) = 0;

  virtual void remove() = 0;

  virtual std::string toString() const =0;

 private:
  ClassDefOverride(SpatialOrganizationNode, 1);
};

}  // namespace spatial_organization
}  // namespace bdm

#endif  // SPATIAL_ORGANIZATION_SPATIAL_ORGANIZATION_NODE_H_
