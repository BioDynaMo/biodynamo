#ifndef SPATIAL_ORGANIZATION_OPEN_TRIANGLE_ORGANIZER_H_
#define SPATIAL_ORGANIZATION_OPEN_TRIANGLE_ORGANIZER_H_

#include <string>
#include <stdexcept>
#include <memory>

namespace cx3d {
namespace spatial_organization {

template<class T> class Triangle3D;
template<class T> class SpaceNode;

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
   * Informs this open triangle organizer that an open triangle
   * is no longer available. In order to do so, the new open
   * triangle is removed from the hashmap.
   * @param triangle The triangle that should be removed.
   */
  virtual void removeTriangle(const std::shared_ptr<Triangle3D<T> >& triangle){
    throw std::logic_error(
            "SpaceNode::removeTriangle must never be called - Java must provide implementation at this point");
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
  virtual std::shared_ptr<Triangle3D<T>> getTriangle(const std::shared_ptr<SpaceNode<T>>& a,
                                                     const std::shared_ptr<SpaceNode<T>>& b,
                                                     const std::shared_ptr<SpaceNode<T>>& c) const {
    throw std::logic_error(
        "SpaceNode::getTriangle must never be called - Java must provide implementation at this point");
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
      const std::shared_ptr<SpaceNode<T>>& a, const std::shared_ptr<SpaceNode<T>>& b,
      const std::shared_ptr<SpaceNode<T>>& c) const {
    throw std::logic_error(
        "SpaceNode::getTriangleWithoutRemoving must never be called - Java must provide implementation at this point");
  }

  virtual std::string toString() const {
    throw std::logic_error(
            "SpaceNode::toString must never be called - Java must provide implementation at this point");
  }
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_OPEN_TRIANGLE_ORGANIZER_H_
