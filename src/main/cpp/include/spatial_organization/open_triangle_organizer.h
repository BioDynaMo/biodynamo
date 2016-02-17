#ifndef SPATIAL_ORGANIZATION_OPEN_TRIANGLE_ORGANIZER_H_
#define SPATIAL_ORGANIZATION_OPEN_TRIANGLE_ORGANIZER_H_

#include <string>
#include <stdexcept>
#include <memory>

namespace cx3d {
namespace spatial_organization {

template<class T> class Triangle3D;
template<class T> class SpaceNode;
template<class T> class Tetrahedron;

/**
 * Whenever an incomplete delaunay triangulation is created for some reason (removement of
 * a node or during an initial triangulation), 'holes' in the triangulation need to be filled.
 *
 * This class provides the functionality to create complete triangulations from incomplete ones.
 * It keeps track of all triangles in the triangulation that are currently incident to only
 * one tetrahedron (=> 'open triangles') and can fill 'holes' in the triangulation. In order
 * to do so, it creates new tetrahedra by successively choosing an open triangle and a corresponding
 * node.
 *
 * @see #triangulate()
 *
 * @param <T> The type of user objects with the nodes in a given triangulation.
 */
template<class T>
class OpenTriangleOrganizer {
 public:
  virtual ~OpenTriangleOrganizer() {
  }

  /**
   * Returns a simple instance of <code>OpenTriangleOrganizer</code>. As a node
   * organizer, an instance of {@link SimpleTriangulationNodeOrganizer} is used
   * and the number of open triangles that will approximately be used is set
   * to 30.
   * @param <T> The type of user objects associated with nodes in the triangulation.
   * @return A new instance of <code>OpenTriangleOrganizer</code>.
   */
  static std::shared_ptr<OpenTriangleOrganizer<T>> createSimpleOpenTriangleOrganizer() {
    throw std::logic_error(
        "OpenTriangleOrganizer::createSimpleOpenTriangleOrganizer must never be called - Java must provide implementation at this point");
  }

  /**
   * Informs this open triangle organizer that a new
   * open triangle is available. In order to do so, the new open
   * triangle is added to the hashmap.
   * @param triangle The new open triangle.
   */
  virtual void putTriangle(const std::shared_ptr<Triangle3D<T>>& triangle) {
    throw std::logic_error(
        "OpenTriangleOrganizer::removeTriangle must never be called - Java must provide implementation at this point");
  }

  /**
   * Informs this open triangle organizer that an open triangle
   * is no longer available. In order to do so, the new open
   * triangle is removed from the hashmap.
   * @param triangle The triangle that should be removed.
   */
  virtual void removeTriangle(const std::shared_ptr<Triangle3D<T> >& triangle) {
    throw std::logic_error(
        "OpenTriangleOrganizer::removeTriangle must never be called - Java must provide implementation at this point");
  }

  /**
   * Searches for a triangle which is incident to three specified nodes.
   * If such an open triangle is not yet known to this
   * open triangle organizer, it is created and added to the
   * hashmap. Otherwise, it is removed from the hashmap (since it won't be
   * open any more soon).
   * @param a The first node incident to the requested triangle.
   * @param b The second node incident to the requested triangle.
   * @param c The third node incident to the requested triangle.
   * @return A triangle with the requested three endpoints.
   */
  virtual std::shared_ptr<Triangle3D<T>> getTriangle(
      const std::shared_ptr<SpaceNode<T>>& a,
      const std::shared_ptr<SpaceNode<T>>& b,
      const std::shared_ptr<SpaceNode<T>>& c) const {
    throw std::logic_error(
        "OpenTriangleOrganizer::getTriangle must never be called - Java must provide implementation at this point");
  }

  /**
   * Searches for a triangle which is incident to three specified nodes.
   * If this triangle does not exist yet, it is created and added to the
   * hashmap. In contrary to {@link #getTriangle(SpaceNode, SpaceNode, SpaceNode)},
   * the triangle is not removed from the hashmap if it was already part of the
   * hashmap.
   * @param a The first node incident to the requested triangle.
   * @param b The second node incident to the requested triangle.
   * @param c The third node incident to the requested triangle.
   * @return A triangle with the requested three endpoints.
   */
  virtual std::shared_ptr<Triangle3D<T>> getTriangleWithoutRemoving(
      const std::shared_ptr<SpaceNode<T>>& a,
      const std::shared_ptr<SpaceNode<T>>& b,
      const std::shared_ptr<SpaceNode<T>>& c) const {
    throw std::logic_error(
        "OpenTriangleOrganizer::getTriangleWithoutRemoving must never be called - Java must provide implementation at this point");
  }

  /**
   * Finishes an incomplete triangulation. Before calling this function the open triangle organizer
   * must be informed about all open triangles delimiting the non-triangulated volume in the
   * tetrahedralization.
   * <p>
   * This function will successively pick an open triangle and find the corresponding point for this
   * triangle. The triangle node organizer (given as argument to {@link #OpenTriangleOrganizer(int, AbstractTriangulationNodeOrganizer)})
   * will provide the order in which the candidate nodes are tested.
   * <p>
   * All tetrahedra created by this function will be reported in
   *
   */
  virtual void triangulate() {
    throw std::logic_error(
        "OpenTriangleOrganizer::triangulate must never be called - Java must provide implementation at this point");
  }

  /**
   * Returns a String representation of this OpenTriangleOrganizer
   */
  virtual std::string toString() const {
    throw std::logic_error(
        "OpenTriangleOrganizer::toString must never be called - Java must provide implementation at this point");
  }

  /**
   * Returns an arbitrary tetrahedron which was created during the process of
   * triangulation.
   * @return A tetrahedron which was created during triangulation.
   */
  virtual std::shared_ptr<Tetrahedron<T>> getANewTetrahedron() {
    throw std::logic_error(
        "OpenTriangleOrganizer::getANewTetrahedron must never be called - Java must provide implementation at this point");
  }

  /**
   * Removes one tetrahedron from the triangulation and possibly all adjacent tetrahedra that have the same circumsphere as the
   * first tetrahedron.
   * @param startingTetrahedron The first tetrahedron to remove.
   */
  virtual void removeAllTetrahedraInSphere(
      const std::shared_ptr<Tetrahedron<T>>& starting_tetrahedron) {
    throw std::logic_error(
        "OpenTriangleOrganizer::removeAllTetrahedraInSphere must never be called - Java must provide implementation at this point");
  }
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_OPEN_TRIANGLE_ORGANIZER_H_
