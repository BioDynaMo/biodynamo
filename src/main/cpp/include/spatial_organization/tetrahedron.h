#ifndef SPATIAL_ORGANIZATION_TETRAHEDRON_H_
#define SPATIAL_ORGANIZATION_TETRAHEDRON_H_

namespace cx3d {
namespace spatial_organization {

#include <array>
#include <string>
#include <memory>
#include <stdexcept>

template<class T> class Triangle3D;
template<class T> class SpaceNode;

/**
 * This class is used to represent a tetrahedron. Each instance saves references
 * to 4 incident nodes and 4 incident triangles. Additionally,
 * <code>Tetrahedron</code> stores information about the volume_ and the
 * circumsphere of the tetrahedron defined by the incident nodes.
 *
 * Tetrahedra can either be finite or infinite. In the latter case, the first
 * node incident to this tetrahedron is set to <code>null</code>, indicating
 * that the other three endpoints of this tetrahedron are part of the convex
 * hull of all points in the current triangulation.
 *
 * @param <T> The type of the user objects stored in endpoints of a tetrahedron.
 */
template<class T>
class Tetrahedron {
 public:
  virtual ~Tetrahedron() {
  }

  /**
   * Returns whether this tetrahedron is infinite.
   *
   * @return <code>true</code>, if the tetrahedron is infinite (first
   *         endpoint is null).
   */
  virtual bool isInfinite() {
    throw std::logic_error(
        "isInfinite must never be called - Java must provide implementation at this point");
  }

  /**
   * Returns the incident node opposite to a given triangle which is incident
   * to this tetrahedron.
   *
   * @param triangle
   *            An incident triangle of this tetrahedron.
   * @return The endpoint of this triangle that lies opposite to
   *         <code>triangle</code>.
   */
  virtual std::shared_ptr<SpaceNode<T> > getOppositeNode(
      const std::shared_ptr<Triangle3D<T>>& triangle) {
    throw std::logic_error(
        "getOppositeNode must never be called - Java must provide implementation at this point");
  }

  /**
   * @return An array containing the nodes incident to this tetrahedron.
   */
  virtual std::array<std::shared_ptr<SpaceNode<T> >, 4> getAdjacentNodes() {
    throw std::logic_error(
        "getAdjacentNodes must never be called - Java must provide implementation at this point");
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

#endif  // SPATIAL_ORGANIZATION_TETRAHEDRON_H_
