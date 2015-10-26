#ifndef SPATIAL_ORGANIZATION_SPACE_NODE_H_
#define SPATIAL_ORGANIZATION_SPACE_NODE_H_

#include <array>
#include <stdexcept>
#include <string>
#include <list>

#include "spatial_organization/spatial_organization_node.h"
#include "spatial_organization/spatial_organization_edge.h"

namespace cx3d {
namespace spatial_organization {

template<class T> class Tetrahedron;
template<class T> class Edge;

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

  /**
   * @return The list of tetrahedra incident to this node.
   */
  virtual std::list<std::shared_ptr<Tetrahedron<T>> > getAdjacentTetrahedra() const {
    throw std::logic_error(
        "getAdjacentTetrahedra must never be called - Java must provide implementation at this point");
  }

  /**
   * Adds an element to the list of adjacent tetrahedra incident to this node.
   */
  // todo rename to addTetrahedron to be consistent
  virtual void addAdjacentTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) {
    throw std::logic_error(
        "addAdjacentTetrahedron must never be called - Java must provide implementation at this point");
  }

  /**
   * Removes a given tetrahedron from the list of incident tetrahedra.
   *
   * @param tetrahedron
   *            The tetrahedron to be remobed.
   */
  virtual void removeTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) {
    throw std::logic_error(
        "addAdjacentTetrahedron must never be called - Java must provide implementation at this point");
  }

  virtual std::array<double, 3> getPosition() const override {
    throw std::logic_error(
        "getPosition must never be called - Java must provide implementation at this point");
  }

  virtual std::shared_ptr<T> getUserObject() const override {
    throw std::logic_error(
        "getUserObject must never be called - Java must provide implementation at this point");
  }

  /**
   * Modifies the volume associated with this SpaceNode by a given value.
   *
   * @param change
   *            The change value that will be added to the volume.
   */
  virtual void changeVolume(double change) {
    throw std::logic_error(
        "changeVolume must never be called - Java must provide implementation at this point");
  }

  /**
   * Adds a new edge to the std::list of adjacent edges.
   *
   * @param newEdge
   *            The edge to be added.
   */
  virtual void addEdge(const std::shared_ptr<Edge<T>>& edge) {
    throw std::logic_error(
        "addEdge must never be called - Java must provide implementation at this point");
  }

  /**
   * @return The identification number of this SpaceNode.
   */
  virtual int getId() const {
    throw std::logic_error(
        "getId must never be called - Java must provide implementation at this point");
  }

  /**
   * Searches for an edge which connects this SpaceNode with another
   * SpaceNode.
   *
   * @param oppositeNode
   *            The other node to which the required edge should be connected
   *            to.
   * @return An edge connecting this node and <code>oppositeNode</code>. If
   *         such an edge didn't exist in the std::list of edges incident to this
   *         node, a new edge is created.
   */
  virtual std::shared_ptr<Edge<T>> searchEdge(
      const std::shared_ptr<SpaceNode<T>>& opposite_node) const {
    throw std::logic_error(
        "searchEdge must never be called - Java must provide implementation at this point");
  }

  /**
   * Removes a given edge from the std::list of incident edges.
   *
   * @param edge
   *            The edge to be removed.
   */
  virtual void removeEdge(const std::shared_ptr<Edge<T>>& edge) {
    throw std::logic_error(
        "removeEdge must never be called - Java must provide implementation at this point");
  }

  /**
   * Returns a string representation of this node.
   */
  virtual std::string toString() const {
    throw std::logic_error(
        "toString must never be called - Java must provide implementation at this point");
  }
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_SPACE_NODE_H_
