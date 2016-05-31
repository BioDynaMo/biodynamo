#ifndef SPATIAL_ORGANIZATION_PLANE_3D_H_
#define SPATIAL_ORGANIZATION_PLANE_3D_H_

#include <array>
#include <memory>

namespace bdm {
namespace spatial_organization {

template<class T> class SpaceNode;
template<class T> class Tetrahedron;

/**
 * Used to represent a plane in three dimensional space. Here, a plane is fully defined by
 * a normal vector and an offset value.
 * @param <T> The type of user objects associated with nodes in the current triangulation.
 */
template<class T>
class Plane3D {
 public:
  /**
   * The default constructor. Only used to avoid errors in child classes.
   */
  Plane3D();

  /**
   * Creates a plane from a given normal vector and a defined offset value.
   * @param normalVector The normal vector of the plane.
   * @param offset The offset of the plane.
   */
  Plane3D(const std::array<double, 3>& normal_vector, double offset);

  /**
   * Creates a plane from two direction vectors and one vector
   * pointing to a position on the plane.
   * @param direction_vector_1 The first direction vector. Must not be colinear to <code>directionVector2</code>.
   * @param directionVector2 The second direction vector. Must not be colinear to <code>directionVector1</code>.
   * @param position_vector The coordinate of a point on the plane.
   * @param normalize Defines whether or not the normal vector of this plane should be normalized or not.
   */
  Plane3D(const std::array<double, 3>& direction_vector_1, const std::array<double, 3>& direction_vector_2,
          const std::array<double, 3>& position_vector, bool normalize);

  /**
   * Creates a plane from two direction vectors and one vector
   * pointing to a position on the plane. The normal vector of the resulting plane will be
   * normalized if {@link #NORMALIZE} is set to <code>true</code>.
   * @param direction_vector_1 The first direction vector. Must not be colinear to <code>directionVector2</code>.
   * @param directionVector2 The second direction vector. Must not be colinear to <code>directionVector1</code>.
   * @param position_vector The coordinate of a point on the plane.
   */
  Plane3D(const std::array<double, 3>& direction_vector_1, const std::array<double, 3>& direction_vector_2,
          const std::array<double, 3>& position_vector);

  /**
   * Creates a plane from a set of nodes.
   * @param nodes An array of nodes which is expected to be of length 4. Three nodes from these 4 are used to
   * create the plane. <code>nonUsedNode</code> defines, which node will not be used.
   * @param nonUsedNode The node in <code>nodes</code> which will not become part of the newly created plane.
   * @param normalize Defines whether or not the normal vector of this plane should be normalized or not.
   */
  Plane3D(const std::array<SpaceNode<T>*, 4>& nodes, SpaceNode<T>* non_used_node, bool normalize);

  /**
   * Creates a plane from a set of nodes. The normal vector of the resulting plane will be
   * normalized if {@link #NORMALIZE} is set to <code>true</code>.
   * @param nodes An array of nodes which is expected to be of length 4. Three nodes from these 4 are used to
   * create the plane. <code>nonUsedNode</code> defines, which node will not be upublicsed.
   * @param nonUsedNode The node in <code>nodes</code> which will not become part of the newly created plane.
   */
  Plane3D(const std::array<SpaceNode<T>*, 4>& nodes, SpaceNode<T>* non_used_node);

  /**
   * Creates a plane from the endpoint of a tetrahedron. The normal vector of the resulting plane will be
   * normalized if {@link #NORMALIZE} is set to <code>true</code>.
   * @param tetra The tetrahedron which should be used to create an new plane. Three of the endpoints of
   * <code>tetra</code> will be used for the new plane/
   * @param nonUsedNode The node in <code>nodes</code> which will not become part of the newly created plane.
   */
  Plane3D(const std::shared_ptr<Tetrahedron<T>>& tetrahedron, SpaceNode<T>* non_used_node);

  virtual ~Plane3D();

  /**
   * Initializes the plane by computing a normal vector and an offset value from
   * two direction vectors and one position vector.
   * @param direction_vector_1 The first direction vector. Must not be colinear to <code>directionVector2</code>.
   * @param direction_vector_2 The second direction vector. Must not be colinear to <code>directionVector1</code>.
   * @param position_vector The coordinate of a point on the plane.
   * @param normalize Defines whether or not the normal vector of this plane should be normalized or not.
   */
  virtual void initPlane(const std::array<double, 3>& direction_vector_1,
                         const std::array<double, 3>& direction_vector_2, const std::array<double, 3>& position_vector,
                         bool normalize);

  /**
   * Reverts the orientation of this plane by switching the sign of all entries
   * in the normal vector and the offset value.
   */
  virtual void changeUpperSide();

  /**
   * Defines the upper side of this plane. The upper side is defined to be the one side
   * to which the normal vector points to.
   * @param point A coordinate on the upper side of the plane.
   */
  virtual void defineUpperSide(const std::array<double, 3>& point);

  /**
   * Computes the orientation of two coordinates relative this plane.
   * @param point_1 The first coordinate.
   * @param point_2 The second coordinate.
   * @return 1, if both points lie on the same side of the plane, -1, if the points
   * lie on opposite sides of the plane and 0, if either one of the points lies
   * on the plane.
   */
  virtual int orientation(const std::array<double, 3>& point_1, const std::array<double, 3>& point_2) const;

  /**
   * Returns whether or not two points lie on the same side of this plane.
   *
   * @param point_1 The first point.
   * @param point_2 The second point.
   * @return <code>true</code>, if both points lie on the same side of the
   *         plane and <code>false</code>, if they don't or if one of them
   *         lies in the plane.
   */
  virtual bool trulyOnSameSide(const std::array<double, 3>& point_1, const std::array<double, 3>& point_2);

  /**
   * Returns whether or not two points lie on different sides of this plane.
   *
   * @param point_1 The first point.
   * @param point_2 The second point.
   * @return <code>true</code>, if both points lie on different sides of the
   *         plane and <code>false</code>, if they don't or if one of them
   *         lies in the plane.
   */
  virtual bool trulyOnDifferentSides(const std::array<double, 3>& point_1, const std::array<double, 3>& point_2);

  /**
   * Returns whether or not two points lie on the same side of this plane.
   *
   * @param point_1 The first point.
   * @param point_2 The second point.
   * @return <code>true</code>, if both points lie on the same side of the
   *         plane or if any one of them lies in the plane
   *         and <code>false</code>, if they don't or if one of them
   *         lies in the plane.
   */
  virtual bool onSameSide(const std::array<double, 3>& point_1, const std::array<double, 3>& point_2) const;

  /**
   * @return The normal vector of this plane.
   */
  virtual std::array<double, 3> getNormalVector();

 protected:
  /**
   * Used to define whether or not normal vectors should be normalized to unit length.
   */
  static constexpr bool normalize_ = true;

  /**
   * The normal vector of this plane.
   */
  std::array<double, 3> normal_vector_;

  /**
   * The offset of this plane which is equal to the dot product of <code>normalVector</code>
   * with any coordinate on the plane.
   */
  double offset_;

  /**
   * Defines a tolerance intervall in which precise arithmetics are used.
   */
  double tolerance_;

  /**
   * A boolean to store whether or not the normal vector has changed.
   */
  bool normal_vector_updated_;

  /**
   * Computes the orientation of two coordinates relative this plane.
   * This function uses precise arithmetics to assure the result.
   * @param point_1 The first coordinate.
   * @param point_2 The second coordinate.
   * @return 1, if both points lie on the same side of the plane, -1, if the points
   * lie on opposite sides of the plane and 0, if either one of the points lies
   * on the plane.
   */
  virtual int orientationExact(const std::array<double, 3>& point_1, const std::array<double, 3>& point_2) const;

 private:
  Plane3D(const Plane3D&) = delete;
  Plane3D& operator=(const Plane3D&) = delete;
};

}  // namespace spatial_organization
}  // namespace bdm

#endif  // SPATIAL_ORGANIZATION_PLANE_3D_H_
