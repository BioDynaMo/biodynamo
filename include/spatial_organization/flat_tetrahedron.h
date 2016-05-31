#ifndef SPATIAL_ORGANIZATION_FLAT_TETRAHEDRON_H_
#define SPATIAL_ORGANIZATION_FLAT_TETRAHEDRON_H_

#include <memory>
#include <string>
#include <array>

#include "spatial_organization/tetrahedron.h"

namespace cx3d {
namespace spatial_organization {

template<class T> class SpaceNode;
template<class T> class Triangle3D;
template<class T> class OpenTriangleOrganizer;

/**
 * During the flip algorithm, it can happen that tetrahedra with no volume are created.
 * Since these have no volume and no circumsphere, they cannot simply be stored as normal
 * tetrahedra. This class extends the class {@link Tetrahedron} in such a way, that these
 * problems are handled.
 *
 * @param <T> The type of user objects associated with nodes in this triangulation.
 */
template<class T>
class FlatTetrahedron : public Tetrahedron<T> {  //, public std::enable_shared_from_this<FlatTetrahedron<T>> {
 public:
  /**
   * Creates a new FlatTetrahedron object and returns it within a <code>std::shared_ptr</code>
   * @see Edge(...)
   *
   * If functions return a std::shared_ptr of <code>this</code> using
   * <code>return shared_from_this();</code>, the following precondition must be met:
   * There must be at least one std::shared_ptr p that owns *this!
   * Calling <code>shared_from_this</code> on a non-shared object results in undefined behaviour.
   * http://mortoray.com/2013/08/02/safely-using-enable_shared_from_this/
   *
   * Therefore, all constructors are private and accessed through static factory methods that return
   * a std::shared_ptr.
   *
   * TODO(lukas) SWIG doesn't seem to support variadic templates and perfect forwarding system.
   * Once mapping to Java is not needed anymore, replace following create functions with:
   * <code>
   * template<typename ... T>
   * static std::shared_ptr<FlatTetrahedron> create(T&& ... all) {
   *   return std::shared_ptr<FlatTetrahedron>(new FlatTetrahedron(std::forward<T>(all)...));
   * }
   * </code>
   *
   * Constructs a new flat tetrahedron from a given triangle and a fourth point.
   * Missing triangles are created.
   *
   * @param one_triangle
   *            The triangle delivering 3 of the new tetrahedron's endpoints.
   * @param fourth_point
   *            The fourth endpoint of the new tetrahedron.
   * @param oto
   *            An organizer for open triangles which is used to keep track of
   *            newly created triangles.
   */

  static std::shared_ptr<Tetrahedron<T>> create(const std::shared_ptr<Triangle3D<T>>& one_triangle,
                                                SpaceNode<T>* fourth_point,
                                                const std::shared_ptr<OpenTriangleOrganizer<T>>& oto);

  /**
   * Creates a new flat tetrahedron from four triangles and four points.
   *
   * @param triangle_a
   *            The first triangle.
   * @param triangle_b
   *            The second triangle.
   * @param triangle_c
   *            The third triangle.
   * @param triangle_d
   *            The fourth triangle.
   * @param node_a
   *            The first point, must lie opposite to triangleA.
   * @param node_b
   *            The first point, must lie opposite to triangleB.
   * @param node_c
   *            The first point, must lie opposite to triangleC.
   * @param node_d
   *            The first point, must lie opposite to triangleD.
   */
  static std::shared_ptr<Tetrahedron<T>> create(const std::shared_ptr<Triangle3D<T>>& triangle_a,
                                                const std::shared_ptr<Triangle3D<T>>& triangle_b,
                                                const std::shared_ptr<Triangle3D<T>>& triangle_c,
                                                const std::shared_ptr<Triangle3D<T>>& triangle_d,
                                                SpaceNode<T>* node_a,
                                                SpaceNode<T>* node_b,
                                                SpaceNode<T>* node_c,
                                                SpaceNode<T>* node_d);

  virtual ~FlatTetrahedron();

  /**
   * Updates the circumsphere of this tetrahedron. Since a flat tetrahedron does not
   * have a real circumsphere, no work is performed in this function but the adjacent nodes are
   * informed about the movement of <code>movedNode</code>.
   * @param moved_node The node that was moved.
   */
  void updateCirumSphereAfterNodeMovement(SpaceNode<T>* moved_node) override;

  /**
   * Calculates the volume of this flat tetrahedron. Since the volume of a flat tetrahedron is
   * 0, the volume is simpley set to that value.
   */
  void calculateVolume() override;

  /**
   * Sets all the cross section areas associated with the incident edges to 0.
   */
  void updateCrossSectionAreas() override;

  /**
   * Computes the properties of the circumsphere around this tetrahedron.
   * Since a flat tetrahedron has no circumsphere, no work is performed in this function.
   */
  void calculateCircumSphere() override;

  /**
   * Returns whether this tetrahedron is flat or not.
   * @return <code>true</code> in any case, because all instances of this class are flat tetrahedra.
   */
  bool isFlat() const override;

  /**
   * Determines wether a given point lies inside or outside the circumsphere
   * of this tetrahedron or lies on the surface of this sphere. For a flat tetrahedron,
   * all points that do not lie in the same plane as the tetrahedron itself are defined
   * to lie outside the circumsphere. For points which lie in the same plane
   * as the tetrahedron,  the orientation is 1 if the point lies inside the circumcircle around
   * any of the incident triangles, 0 if it lies on any circumcircle and -1 else.
   *
   * @param point
   *            The point for which the orientation should be determined.
   * @return -1, if the point lies outside this tetrahedron's circumsphere, 1
   *         if it is inside the sphere and 0, if it lies on the surface of
   *         the sphere.
   */
  int orientation(const std::array<double, 3>& point) override;

  /**
   * {@inheritDoc}
   */
  bool isTrulyInsideSphere(const std::array<double, 3>& point) override;

  /**
   * {@inheritDoc}
   */
  bool isInsideSphere(const std::array<double, 3>& point) override;

  /**
   * {@inheritDoc}
   */
  bool isPointInConvexPosition(const std::array<double, 3>& point,
                               size_t connecting_triangle_number) const override;

  /**
   * {@inheritDoc}
   */
  int isInConvexPosition(const std::array<double, 3>& point, size_t connecting_triangle_number) const
      override;

 protected:
  FlatTetrahedron();

 private:
  FlatTetrahedron(const FlatTetrahedron&) = delete;
  FlatTetrahedron& operator=(const FlatTetrahedron&) = delete;
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_FLAT_TETRAHEDRON_H_
