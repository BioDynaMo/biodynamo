#ifndef SPATIAL_ORGANIZATION_TRIANGLE_3D_H_
#define SPATIAL_ORGANIZATION_TRIANGLE_3D_H_

#include <array>
#include <memory>
#include <string>

#include "spatial_organization/plane_3d.h"
#include "spatial_organization/exact_vector.h"
#include "spatial_organization/rational.h"

#ifdef TRIANGLE3D_DEBUG
#include "spatial_organization/debug/triangle_3d_debug.h"
#endif

namespace cx3d {
namespace spatial_organization {

template<class T> class Tetrahedron;
template<class T> class SpaceNode;

template<class T>
class Triangle3D : public Plane3D<T>, public std::enable_shared_from_this<Triangle3D<T>> {
 public:
#ifndef TRIANGLE3D_NATIVE
  Triangle3D()
      : Plane3D<T>(),
        adjacent_tetrahedra_(),
        nodes_(),
        circum_center_ { 0.0, 0.0, 0.0 },
        plane_updated_(false),
        circum_center_updated_(false),
        upper_side_positive_(true),
        connection_checked_(-1) {
  }
#endif

  /**
   * Creates a new Triangle3D object and returns it within a <code>std::shared_ptr</code>
   * @see Triangle3D(...)
   *
   * If functions return a std::shared_ptr of <code>*this</code> using
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
   * static std::shared_ptr<Triangle3D> create(T&& ... all) {
   *   return std::shared_ptr<Triangle3D>(new Triangle3D(std::forward<T>(all)...));
   * }
   * </code>
   */
  static std::shared_ptr<Triangle3D<T>> create(
      SpaceNode<T>* sn_1,
      SpaceNode<T>* sn_2,
      SpaceNode<T>* sn_3,
      const std::shared_ptr<Tetrahedron<T>>& tetrahedron_1,
      const std::shared_ptr<Tetrahedron<T>>& tetrahedron_2) {
#ifdef TRIANGLE3D_DEBUG
    std::shared_ptr<Triangle3D<T>> triangle(new Triangle3DDebug<T>(sn_1, sn_2, sn_3, tetrahedron_1, tetrahedron_2));
#else
    std::shared_ptr<Triangle3D<T>> triangle(
        new Triangle3D(sn_1, sn_2, sn_3, tetrahedron_1, tetrahedron_2));
#endif
    return triangle;
  }

  /**
   * Calculates the crossing point of three planes given in normal form.
   *
   * @param normals
   *            The normals of the three planes. <code>normals[i]</code>
   *            denotes the normal vector of the <code>i</code>th plane.
   * @param offsets
   *            The offsets of the three planes. Say, E0: n0.x == n[0].s,
   *            where s is a point on the plane. Then offset[0] = n[0].s.
   * @param normal_det
   *            The determinant of the Matrix normals
   * @param[OUT] result The cutting point of the three planes if there is any and the
   *         maximum vector possible if not.
   */
  static std::array<double, 3> calculate3PlaneXPoint(
      const std::array<std::array<double, 3>, 3>& normals, const std::array<double, 3>& offsets,
      double normal_det);

  /**
   * Calculates the crossing point of three planes given in normal form.
   *
   * @param normals
   *            The normals of the three planes. <code>normals[i]</code>
   *            denotes the normal vector of the <code>i</code>th plane.
   * @param offsets
   *            The offsets of the three planes. Say, E0: n0.x == n[0].s,
   *            where s is a point on the plane. Then offset[0] = n[0].s.
   * @return The cutting point of the three planes if there is any and the
   *         maximum vector possible if not.
   */
  static std::array<double, 3> calculate3PlaneXPoint(
      const std::array<std::array<double, 3>, 3>& normals, const std::array<double, 3>& offsets);

  /**
   * Calculates the crossing point of three planes given in normal form using
   * precise arithmetics.
   *
   * @param normals
   *            The normals of the three planes. <code>normals[i]</code>
   *            denotes the normal vector of the <code>i</code>th plane.
   * @param offsets
   *            The offsets of the three planes. Say, E0: n0.x == n[0].s,
   *            where s is a point on the plane. Then offset[0] = n[0].s.
   * @param normal_det
   *            The determinant of the Matrix normals
   * @return The cutting point of the three planes if there is any and the
   *         maximum vector possible if not.
   */
  static std::shared_ptr<ExactVector> calculate3PlaneXPoint(
      const std::array<std::shared_ptr<ExactVector>, 3>& normals,
      const std::array<std::shared_ptr<Rational>, 3>& offsets,
      const std::shared_ptr<Rational>& normal_det);

  virtual ~Triangle3D() {
  }

  /**
   * Compares this triangle to another triangle.
   *
   * @param other_triangle
   *            The other triangle.
   * @return <code>true</code>, if both triangles are incident to the same
   *         nodes.
   */
  virtual bool isSimilarTo(const std::shared_ptr<Triangle3D<T>>& other_triangle) const;

  /**
   * Returns the distance of the center of a circumsphere touching all points
   * of this triangle and the given 4th point and the center of the
   * circumcenter touching all three points of the triangle. The sign of the
   * returned value indicates whether the center of the circumsphere lies on
   * the upper side of the plane defined by this triangle.
   * <p>
   * This distance is not necessarily normalized!
   *
   * @param fourth_point
   *            The 4th point defining the circumsphere.
   * @return The signed Delaunay distance or {@link Double#MAX_VALUE} if it
   *         cannot be calculated.
   *
   * @see #calculateSDDistance(double[])
   */
  virtual double getSDDistance(const std::array<double, 3>& fourth_point) const;

  /**
   * Calculates the distance of the center of a circumsphere touching all
   * points of this triangle and the given 4th point and the center of the
   * circumcenter touching all three points of the triangle. This distance is
   * NOT normalized since the vector orthogonal is not normalized!
   *
   * @param fourth_point
   *            The 4th point defining the circumsphere.
   * @return The signed delaunay distance or -1 if it cannot be calculated.
   *
   */
  virtual std::shared_ptr<Rational> getSDDistanceExact(
      const std::array<double, 3>& fourth_point) const;

  /**
   * Calculates the center of the circumsphere around the three endpoints of this triangle and a
   * given fourth point.
   * <p>
   * <b>Currently, there is no function implemented that estimates the error of this function's
   * result. Therefore, the function {@link Tetrahedron#calculateCircumSphere()} should be preferred.</b>
   * @param fourth_point The fourth point defining the sphere.
   * @return The coordinate of the center of the circumsphere if it can be computed and <code>null</code> else.
   */
  virtual std::array<double, 3> calculateCircumSphereCenter(
      const std::array<double, 3>& fourth_point) const;

  /**
   * Calculates the center of the circumsphere around the three endpoints of this triangle and a
   * given fourth point if the circumcenter of this triangle is updated.
   * <p>
   * <b>Currently, there is no function implemented that estimates the error of this function's
   * result. Therefore, the function {@link Tetrahedron#calculateCircumSphere()} should be preferred.</b>
   * @param fourth_point The fourth point defining the sphere.
   * @return The coordinate of the center of the circumsphere if the circumcenter of
   *  this triangle was updated and <code>null</code> else.
   */
  virtual std::array<double, 3> calculateCircumSphereCenterIfEasy(
      const std::array<double, 3>& fourth_point) const;

  /**
   * This function informs the triangle that one of its incident nodes moved.
   * Therefore, the plane equation and the circumcircle will have to be recalculated.
   */
  virtual void informAboutNodeMovement();

  /**
   * Updates the plane equation for this triangle if and incident node has moved since the last
   * update.
   */
  virtual void updatePlaneEquationIfNecessary();

  /**
   * Updates all parameters of this triangle.
   */
  virtual void update();

  /**
   * {@inheritDoc}
   */
  int orientationExact(const std::array<double, 3>& point_1,
                       const std::array<double, 3>& point_2) const override;

  /**
   * Computes the orientation of a point to the circumcircle of this triangle.
   * This function does NOT test whether the given coordinate lies in the plane of this triangle.
   * It only compares the distance of the point to the circumcenter with the radius of the
   * circumcircle.
   * @param point The coordinate of interest.
   * @return 1, if the distance of <code>point</code> is smaller than the radius of the circumcircle, 0, if it is equal and 1, if
   * it is bigger.
   */
  virtual int circleOrientation(const std::array<double, 3>& point);

  /**
   * Given a tetrahedron which is incident to this triangle, this function returns the second tetrahedron incident to
   * this triangle.
   * @param incident_tetrahedron A tetrahedron incident to this triangle.
   * @return The tetrahedron opposite to <code>incidentTetrahedron</code> at this triangle.
   * @throws RuntimeException if <code>incidentTetrahedron</code> is not incident to this triangle.
   */
  virtual std::shared_ptr<Tetrahedron<T>> getOppositeTetrahedron(
      const std::shared_ptr<Tetrahedron<T>>& incident_tetrahedron) const;

  /**
   * Removes a given tetrahedron from the list of incident tetrahedra.
   * @param tetrahedron A tetrahedron incident to this triangle.
   */
  virtual void removeTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron);

  /**
   * Tests whether this triangle has an open side and whether a given coordinate
   * and the incident tetrahedron lie on opposite sides of the triangle.
   * @param point The coordinate of interest.
   * @return <code>true</code>, if this triangle has an open side and
   * if the given coordinate lies on the open side.
   */
  virtual bool isOpenToSide(const std::array<double, 3>& point);

  /**
   * This function detects on which side of the plane defined by this
   * triangle a given point lies. This side is then defined to be the
   * upper side of this triangle.
   * @param position A coordinate that defines the 'upper side' of this triangle
   *      A runtime exception is thrown if the given point lies in the plane
   *      defined by this triangle.
   */
  virtual void orientToSide(const std::array<double, 3>& position);

  /**
   * This function determines whether this triangle has a single open side.
   * If this is the case, the fourth point incident to the tetrahedron incident to this triangle
   * is defined to be on the lower side of the triangle, thereby defining the open side as the upper side.
   * If this triangle has either two open sides or no open sides, this function throws a RuntimeException.
   */
  virtual void orientToOpenSide();

  /**
   * Determines whether a given coordinate lies on the upper side of this triangle (which must be defined
   * beforehand, using either {@link #orientToOpenSide()} or {@link #orientToSide(double[])}.
   * @param point The coordinate of interest.
   * @return -1, if the coordinate lies on the lower side of the triangle, +1 if it lies on the upper
   *      side of the triangle an 0 if it lies in the plane.
   */
  virtual int orientationToUpperSide(const std::array<double, 3>& point) const;

  /**
   * Determines whether a given coordinate lies on the upper side of this triangle.
   * @param point The coordinate of interest.
   * @return <code>true</code>, if the given point lies on the upper side of this triangle or in the plane
   * defined by this triangle and <code>false</code> otherwise.
   * @see #orientToSide(double[])
   * @see #orientToOpenSide()
   */
  virtual bool onUpperSide(const std::array<double, 3>& point) const;

  /**
   * Determines whether a given coordinate lies truly on the upper side of this triangle.
   * @param point The coordinate of interest.
   * @return <code>true</code>, if the given point lies on the upper side of this triangle and <code>false</code> otherwise.
   * @see #orientToSide(double[])
   * @see #orientToOpenSide()
   */
  virtual bool trulyOnUpperSide(const std::array<double, 3>& point) const;

  /**
   * Calculates an sd-distance which could typically be expected from points lying in a distance
   * similar to the distance between the points of this triangle. Used to compute tolerance values in
   * {@link OpenTriangleOrganizer#triangulate()} but very imprecise! Needs to removed and replaced!
   * @return An unreliable double value.
   *
   */
  virtual double getTypicalSDDistance() const;

  /**
   * Creates a string representation of this object.
   */
  virtual std::string toString() const;

  /**
   * Determines if two instances of this object are equal
   */
  virtual bool equalTo(const std::shared_ptr<Triangle3D<T>>& other) {
    return this == other.get();
  }

  /**
   * Tests whether this triangle has infinite size, meaning that it is
   * incident to <code>null</code>.
   *
   * @return <code>true</code>, if this tetrahedron is incident to a '<code>null</code>'-node.
   */
  virtual bool isInfinite() const;

  /**
   * @return A reference to the array storing the three endpoints of this
   * tetrahedron.
   */
  virtual std::array<SpaceNode<T>*, 3> getNodes() const;

  /**
   * Adds an incident tetrahedron to this triangle.
   *
   * @param tetrahedron A new tetrahedron which is incident to this triangle.
   */
  virtual void addTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron);

  /**
   * Returns whether this triangle has already been tested if it is locally Delaunay.
   * This function is used in a run of {@link SpaceNode#restoreDelaunay()} to
   * keep track of which triangles have already been tested.
   * <p>If this triangle was not tested yet, it is immediately marked as being tested.
   *
   * @param checking_index The unique identifier of the run of <code>restoreDelaunay</code>.
   * @return <code>true</code>, iff this triangle has already been tested for the
   * Delaunay property.
   */
  virtual bool wasCheckedAlready(int checking_index);

  /**
   * Returns whether this triangle is incident to a given tetrahedron.
   * @param tetrahedron a tetrahedron that might be incident to this triangle.
   * @return <code>true</code>, iff the tetrahedron is incident to this triangle.
   */
  virtual bool isAdjacentTo(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) const;

  /**
   * Returns whether this triangle is incident to a given node.
   *
   * @param node A node that might be incident to this triangle.
   * @return <code>true</code>, iff the node is incident to this triangle.
   */
  virtual bool isAdjacentTo(SpaceNode<T>* node) const;

  /**
   * Tests if this triangle is not incident to any tetrahedron.
   * @return <code>true</code>, iff this triangle has no incident tetrahedra.
   */
  virtual bool isCompletelyOpen() const;

  /**
   * Tests if this triangle is incident to two tetrahedra.
   * @return <code>true</code>, iff this triangle has two incident tetrahedra.
   */
  virtual bool isClosed() const;

 protected:
  /**
   * Creates a new triangle from three nodes and two tetrahedra.
   *
   * @param sn_1
   *            The first incident node.
   * @param sn_2
   *            The second incident node.
   * @param sn_3
   *            The third incident node.
   * @param tetrahedron_1
   *            The first incident tetrahedron.
   * @param tetrahedron_2
   *            The second incident tetrahedron.
   */
  Triangle3D(SpaceNode<T>* sn_1, SpaceNode<T>* sn_2,
             SpaceNode<T>* sn_3,
             const std::shared_ptr<Tetrahedron<T>>& tetrahedron_1,
             const std::shared_ptr<Tetrahedron<T>>& tetrahedron_2);

  /**
   * Computes the normal vector for the plane equation of this triangle. The
   * normal vector is calculated using precise arithmetics and therefore, the
   * result is given as an instance of <code>ExactVector</code>.
   *
   * @return The normal vector for this triangle.
   */
  virtual std::shared_ptr<ExactVector> getExactNormalVector() const;

  /**
   * Internal function that is called whenever the normal vector of this
   * triangle is changed.
   * @param new_normal_vector The new normal vector.
   */
  virtual void updateNormalVector(const std::array<double, 3>& new_normal_vector);

 private:
#ifdef TRIANGLE3D_NATIVE
  Triangle3D() = delete;
#endif
  Triangle3D(const Triangle3D&) = delete;
  Triangle3D& operator=(const Triangle3D&) = delete;

  /**
   * The two tetrahedra that are incident to this triangle.
   */
  std::array<std::weak_ptr<Tetrahedron<T>>, 2> adjacent_tetrahedra_;

  /**
   * The three nodes that are incident to this triangle.
   */
  std::array<SpaceNode<T>*, 3> nodes_;

  /**
   * The coordinate of this triangle's circumcenter.
   */
  std::array<double, 3> circum_center_;

  /**
   * Stores whether the plane equation has been updated since the last change
   * of any endpoint.
   */
  bool plane_updated_;

  /**
   * Stores whether the circumcenter is up to date.
   */
  bool circum_center_updated_;

  /**
   * Defines the upper side of this triangle. A point is said to be on the
   * upper side iff its dot product with the normal vector of the plane
   * equation minus the offset is a positive number nad
   * <code>upperSidePositive</code> is <code>true</code> or vice versa.
   */
  bool upper_side_positive_;

  /**
   * Used to remember during the flip algorithm whether this triangle has
   * already been tested to be locally Delaunay.
   */
  int connection_checked_;

  /**
   * Calculates the exact circumcenter for a triangle defined by three
   * position vectors and one normal vector.
   * @param points The coordinates of the points of the triangle.
   * @param normal_vector A normal vector of the triangle.
   * @return The center of the circumcircle around the given triangle.
   */
  static std::shared_ptr<ExactVector> calculateCircumCenterExact(
      const std::array<std::shared_ptr<ExactVector>, 3>& points,
      const std::shared_ptr<ExactVector>& normal_vector);

  /**
   * Calculates the distance of the center of a circumsphere touching all
   * points of this triangle and the given 4th point and the center of the
   * circumcenter touching all three points of the triangle.
   * <p>
   * This distance is not necessarily normalized!
   *
   * @param fourth_point
   *            The 4th point defining the circumsphere.
   * @return The signed delaunay distance or -1 if it cannot be calculated.
   */
  virtual double calculateSDDistance(const std::array<double, 3>& fourth_point) const;

  /**
   * Computes the normal vector for a plane equation given by three position
   * vectors. The normal vector is calculated using precise arithmetics and
   * therefore, the result is given as an instance of <code>ExactVector</code>.
   *
   * @param points
   *            The endpoints of the triangle for which the normal vector
   *            should be calculated, given in exact representation.
   * @return The normal vector for the triangle defined by the three
   *         coordinates.
   */
  virtual std::shared_ptr<ExactVector> calculateExactNormalVector(
      const std::array<std::shared_ptr<ExactVector>, 3>& points) const;

  /**
   * Returns the distance of the center of a circumsphere touching all points
   * of this triangle and the given 4th point and the center of the
   * circumcenter touching all three points of the triangle. The sign of the
   * returned value indicates whether the center of the circumsphere lies on
   * the upper side of the plane defined by this triangle.
   * <p>
   * This function uses precise arithmetics to assure reliability of the
   * result.
   * <p>
   * The calculated distance is not normalized! It can therefore only be used
   * to compare this signed delaunay distance with the delaunay distance of
   * the same triangle to another coordinate.
   *
   * @param points
   *            An array of four vectors. The first three points are expected
   *            to be the three endpoints of the triangle and the fourth is
   *            expected to be the point to which the Delaunay distance should
   *            be calculated.
   * @param normal_vector
   *            A normal vector for the plane defined by the three endpoints
   *            of the triangle.
   * @return The signed delaunay distance or {@link Long#MAX_VALUE} if it
   *         cannot be calculated.
   */
  virtual std::shared_ptr<Rational> calculateSDDistanceExact(
      const std::array<std::shared_ptr<ExactVector>, 4>& points,
      const std::shared_ptr<ExactVector>& normal_vector) const;

  /**
   * Recomputes the center of the circumsphere of this triangle if any incident node moved since the last change.
   */
  virtual void updateCircumCenterIfNecessary();

  /**
   * @return An array of three instances of <code>ExactVector</code> which contain the
   * coordinates of this triangle's endpoints as rational numbers.
   */
  virtual std::array<std::shared_ptr<ExactVector>, 3> getExactPositionVectors() const;
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_TRIANGLE_3D_H_
