#ifndef SPATIAL_ORGANIZATION_EDGE_H_
#define SPATIAL_ORGANIZATION_EDGE_H_

#include <memory>
#include <string>
#include <list>

#include "spatial_organization/spatial_organization_edge.h"

namespace cx3d {
namespace spatial_organization {

template<class T> class SpaceNode;
template<class T> class Tetrahedron;

/**
 * This class is used to represent an edge in a triangulation. Each edge has two endpoint and
 * additionally stores links to all tetrahedra adjacent to it.
 *
 * @param <T> The type of the user objects stored in the endpoints of an edge.
 */
template<class T>
class Edge : public SpatialOrganizationEdge<T> {
 public:
  using SPtr = std::shared_ptr<Edge<T>>;

  /**
   * Creates a new Edge object and returns it within a <code>std::shared_ptr</code>
   * adds the newly created object to the given SpaceNodes a and b
   */
  static SPtr create(SpaceNode<T>* a, SpaceNode<T>* b) {
    SPtr edge(new Edge(a, b));
    if (edge->a_ != nullptr) {
      edge->a_->addEdge(edge);
    }
    if (edge->b_ != nullptr) {
      edge->b_->addEdge(edge);
    }
    return edge;
  }

  virtual ~Edge() {
  }

  /**
   * {@inheritDoc}
   */
  SpaceNode<T>* getOpposite(const SpaceNode<T>* node) const
      override;

  /**
   * {@inheritDoc}
   */
  T* getOppositeElement(T* first) const override;

  /**
   * {@inheritDoc}
   */
  T* getFirstElement() const override;

  /**
   * {@inheritDoc}
   */
  T* getSecondElement() const override;

  /**
   * {@inheritDoc}
   */
  double getCrossSection() const override;

  /**
   *  @return A string representation of this edge
   */
  virtual const std::string toString() const override;

  /**
   * Tests whether this edge is connecting a pair of points.
   * @param a The first node.
   * @param b The second node.
   * @return <code>true</code>, if this edge connects <code>a</code> and <code>b</code>.
   */
  virtual bool equals(SpaceNode<T>* a,
                      SpaceNode<T>* b) const;
  /**
   * Removes a tetrahedron from this edge's list of tetrahedra. If this edge is not incident to
   * any tetrahedra after the removal of the specified tetrahedron, the edge removes itself from
   * the triangulation by calling {@link #remove()}.
   * @param tetrahedron A tetrahedron incident to this edge which should be removed.
   */
  virtual void removeTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron);

  /**
   * Adds a tetrahedron to this edge's list of tetrahedra.
   * @param tetrahedron A tetrahedron incident to this edge which should be added.
   */
  virtual void addTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron);

  /**
   * Removes this edge from the triangulation. To do so, the two endpoints are informed
   * that the edge was removed.
   */
  virtual void remove();

  /**
   * Returns the list of incident tetrahedra.
   * @return The list of incident tetrahedra.
   */
  virtual std::list<std::shared_ptr<Tetrahedron<T>> > getAdjacentTetrahedra() const;

  /**
   * Changes the cross section area of this edge.
   * @param change The value by which the cross section area of this edge has changed.
   * At initialization, this area is set to zero and all tetrahedra that are registered as
   * incident tetrahedra increase the cross section area.
   */
  virtual void changeCrossSectionArea(double change);

 protected:
  /**
   * Initializes a new edge with two specified endpoints.
   * @param a The first endpoint of the new edge.
   * @param b The second endpoint of the new edge.
   */
  Edge(SpaceNode<T>* a, SpaceNode<T>* b);

 private:
  Edge() = delete;
  Edge(const Edge&) = delete;
  Edge& operator=(const Edge&) = delete;

  /**
   * The two endpoints of this edge.
   */
  SpaceNode<T>* a_ = nullptr;
  SpaceNode<T>* b_ = nullptr;

  /**
   * A list of all tetrahedra that are adjacent to this edge.
   */
  std::list<std::shared_ptr<Tetrahedron<T>> > adjacent_tetrahedra_;

  /**
   * Stores the cross section area associated with this edge.
   */
  double cross_section_area_;
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_EDGE_H_
