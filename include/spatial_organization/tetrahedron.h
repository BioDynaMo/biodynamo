#ifndef SPATIAL_ORGANIZATION_TETRAHEDRON_H_
#define SPATIAL_ORGANIZATION_TETRAHEDRON_H_

#include <array>
#include <list>
#include <string>
#include <memory>
#include <stdexcept>

#include "spatial_organization/edge.h"

namespace bdm {
namespace spatial_organization {

template<class T> class SpaceNode;
template<class T> class Triangle3D;
template<class T> class OpenTriangleOrganizer;

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
class Tetrahedron : public std::enable_shared_from_this<Tetrahedron<T>> {
 public:
  /**
   * Creates a new Tetrahedron object and returns it within a <code>std::shared_ptr</code>
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
   * static std::shared_ptr<Tetrahedron> create(T&& ... all) {
   *   return std::shared_ptr<Tetrahedron>(new Tetrahedron(std::forward<T>(all)...));
   * }
   * </code>
   */
  static std::shared_ptr<Tetrahedron<T>> create(const std::shared_ptr<Triangle3D<T>>& one_triangle,
                                                SpaceNode<T>* fourth_point,
                                                const std::shared_ptr<OpenTriangleOrganizer<T>>& oto);

  static std::shared_ptr<Tetrahedron<T>> create(const std::shared_ptr<Triangle3D<T>>& triangle_a,
                                                const std::shared_ptr<Triangle3D<T>>& triangle_b,
                                                const std::shared_ptr<Triangle3D<T>>& triangle_c,
                                                const std::shared_ptr<Triangle3D<T>>& triangle_d, SpaceNode<T>* node_a,
                                                SpaceNode<T>* node_b, SpaceNode<T>* node_c, SpaceNode<T>* node_d);

  /**
   * Generates an initial tetrahedron for a new triangulation. A tetrahedron
   * is generated which has the four given nodes as endpoints and is adjacent
   * to four infinite tetrahedra.
   *
   * @param <T>
   *            The type of the user object stored in the incident nodes.
   * @param a
   *            The first incident node.
   * @param b
   *            The second incident node.
   * @param c
   *            The third incident node.
   * @param d
   *            The fourth incident node.
   * @param simple_oto
   *            simple open triangle organizer
   * @return A tetrahedron which is made out of the four points passed to this
   *         function. This tetrahedron will be neighbor to 4 infinite
   *         tetrahedra.
   */
  static std::shared_ptr<Tetrahedron<T>> createInitialTetrahedron(
      SpaceNode<T>* a, SpaceNode<T>* b, SpaceNode<T>* c, SpaceNode<T>* d,
      const std::shared_ptr<OpenTriangleOrganizer<T>>& simple_oto);

  virtual ~Tetrahedron();

  /**
   * Calculates the properties of this tetrahedron's circumsphere.
   */
  virtual void calculateCircumSphere();

  /**
   * Used to calculate the properties of this tetrahedron's circumsphere after
   * an endpoint has been moved. Originally used to increase the speed of
   * circumsphere calculations, but now uses the same functions as
   * {@link #calculateCircumSphere()} because the old method increased the
   * uncertainity of the circum_center_.
   *
   * In addition to calcualting the circumsphere, all incident triangles that are
   * incident to the moved node are informed about the movement.
   *
   * @param moved_node
   *            The node that was moved.
   */
  virtual void updateCirumSphereAfterNodeMovement(SpaceNode<T>* moved_node);

  /**
   * Determines whether a given point lies inside or outside the circumsphere
   * of this tetrahedron or lies on the surface of this sphere.
   *
   * @param point
   *            The point for which the orientation should be determined.
   * @return -1, if the point lies outside this tetrahedron's circumsphere, 1
   *         if it is inside the sphere and 0, if it lies on the surface of
   *         the sphere.
   */
  virtual int orientation(const std::array<double, 3>& point);

  /**
   * Determines whether a given point lies truly inside the circumsphere of
   * this tetrahedron
   *
   * @param point
   *            The point for which the orientation should be determined.
   * @return <code>true</code> if the distance of the point to the center of
   *         the circumsphere is smaller than the radius of the circumsphere
   *         and <code>false</code> otherwise.
   */
  virtual bool isTrulyInsideSphere(const std::array<double, 3>& point);

  /**
   * Determines whether a given point lies truly inside the circumsphere of
   * this tetrahedron
   *
   * @param point
   *            The point for which the orientation should be determined.
   * @return <code>true</code> if the distance of the point to the center of
   *         the circumsphere is smaller or equal to the radius of the
   *         circumsphere and <code>false</code> otherwise.
   */
  virtual bool isInsideSphere(const std::array<double, 3>& point);

  /**
   * Returns a String representation of this tetrahedron
   */
  virtual std::string toString() const;

  /**
   * Determines if two instances of this object are equal
   */
  bool equalTo(const std::shared_ptr<Tetrahedron<T>>& other);

  /**
   * @return An array of triangles containing the 4 triangles incident to this tetrahedron.
   */
  std::array<std::shared_ptr<Triangle3D<T>>, 4> getAdjacentTriangles() const;

  /**
   * Determines whether a given node is an endpoint of this tetrahedron.
   * @param node The node of interest.
   * @return <code>true</code>, if the node is an endpoint.
   */
  bool isAdjacentTo(SpaceNode<T>* node) const;

  /**
   * Walks toward a specified point. If the point lies inside this
   * tetrahedron, this tetrahedron is returned. Otherwise, an adjacent
   * tetrahedron is returned that lies closer to the point.
   *
   * @param coordinate
   *            The coordinate that should be approximated.
   * @param triangle_order
   *        A small list containing a permutation of the number 0-3. This list is
   *            used to randomize the visibility walk implemented in
   * @return An adjacent tetrahedron that lies closer to the specified point
   *         than this tetrahedron, or this tetrahedron, if the point lies inside it.
   */
  std::shared_ptr<Tetrahedron<T>> walkToPoint(const std::array<double, 3>& coordinate,
                                              const std::array<int, 4>& triangle_order);
  /**
   * @return An array containing the nodes incident to this tetrahedron.
   */
  std::array<SpaceNode<T>*, 4> getAdjacentNodes() const;
//
// protected:
//
  /**
   * Determines the index of the edge connecting two given endpoints of the
   * tetrahedron in this tetrahedron's list of incident edges.
   *
   * @param node_number_1
   *            The index of the first endpoint of the edge.
   * @param node_number_2
   *            The index of the second endpoint of the edge.
   * @return A number between 0 and 5, giving the index of the edge of
   *         interest.
   */
  static int getEdgeNumber(int node_number_1, int node_number_2);

  /**
   * Removes two flat tetrahedra that have two common triangles.
   *
   * @param <T>
   *            The type of the user objects stored in the given tetrahedra.
   * @param tetrahedron_a
   *            The first flat tetrahedron.
   * @param tetrahedron_b
   *            The second flat tetrahedron.
   * @return A list of tetrahedra that were originally adjacent to either one
   *         of the two flat tetrahedra that were removed.
   */
  static std::list<std::shared_ptr<Tetrahedron<T>> > remove2FlatTetrahedra(
      const std::shared_ptr<Tetrahedron<T>>& tetrahedron_a, const std::shared_ptr<Tetrahedron<T>>& tetrahedron_b);

  /**
   * Performs a 2->3 Flip of two adjacent tetrahedra.
   * @param <T> The type of the user objects stored in the endpoints of the two tetrahedra.
   * @param tetrahedron_a The first tetrahedron to flip.
   * @param tetrahedron_b The second tetrahedron to flip.
   * @return An array of tetrahedra which were created during the process of flipping.
   */
  static std::array<std::shared_ptr<Tetrahedron<T>>, 3> flip2to3(const std::shared_ptr<Tetrahedron<T>>& tetrahedron_a,
                                                                 const std::shared_ptr<Tetrahedron<T>>& tetrahedron_b);

  /**
   * Performs a 3->2 Flip of two adjacent tetrahedra.
   * @param <T> The type of the user objects stored in the endpoints of the two tetrahedra.
   * @param tetrahedron_a The first tetrahedron to flip.
   * @param tetrahedron_b The second tetrahedron to flip.
   * @param tetrahedron_c The third tetrahedron to flip.
   * @return An array of tetrahedra which were created during the process of flipping.
   */
  static std::array<std::shared_ptr<Tetrahedron<T>>, 2> flip3to2(const std::shared_ptr<Tetrahedron<T>>& tetrahedron_a,
                                                                 const std::shared_ptr<Tetrahedron<T>>& tetrahedron_b,
                                                                 const std::shared_ptr<Tetrahedron<T>>& tetrahedron_c);

  /**
   * Extracts the user objects associated with the four endpoints of this
   * tetrahedron.
   *
   * @return An array of objects of type <code>T</code>.
   */
  std::array<T*, 4> getVerticeContents() const;

  /**
   * Returns whether this tetrahedron is infinite.
   *
   * @return <code>true</code>, if the tetrahedron is infinite (first
   *         endpoint is null).
   */
  bool isInfinite() const;

  /**
   * Returns whether this tetrahedron is a flat tetrahedron. Used to simplify
   * distinction between the two types <code>Tetrahedron</code> and
   * <code>FlatTetrahedron</code>.
   *
   * @return <code>false</code> for all instances of
   *         <code>Tetrahedron</code>.
   */
  virtual bool isFlat() const {
    return false;
  }

  /**
   * Changes the cross section area associated with one incident edge. Informs
   * incident edges if there is a change in their cross section area.
   *
   * @param number
   *            The index of the edge which cross section area should be
   *            changed.
   * @param new_value
   *            The new value for the cross section area of the specified
   *            edge.
   */
  void changeCrossSection(int number, double new_value);

  /**
   * Calculates all cross section areas of the six edges incident to this
   * tetrahedron.
   */
  virtual void updateCrossSectionAreas();

  /**
   * Calculates the volume_ of this tetrahedron and changes the stored value.
   * (The volume_ equals 1/6th of the determinant of 3 incident edges with a
   * common endpoint.)
   */
  virtual void calculateVolume();

  /**
   * Determines wether a given point lies inside or outside the circumsphere
   * of this tetrahedron or lies on the surface of this sphere. This function
   * uses precise arithmetics to calculate a reliable answer.
   *
   * @param position
   *            The position for which the orientation should be determined.
   * @return -1, if the point lies outside this tetrahedron's circumsphere, 1
   *         if it is inside the sphere and 0, if it lies on the surface of
   *         the sphere.
   */
  int orientationExact(const std::array<double, 3>& position) const;

  /**
   * Replaces one of the incident triangles of this tetrahedron. Automatically
   * exchanges the affected edges, too.
   *
   * @param old_triangle
   *            The triangle that should be replaced.
   * @param new_triangle
   *            The new trianlge.
   */
  void replaceTriangle(const std::shared_ptr<Triangle3D<T>>& old_triangle,
                       const std::shared_ptr<Triangle3D<T>>& new_triangle);

  /**
   * Determines which index a given node has in this tetrahedron's list of
   * endpoints.
   *
   * @param node
   *            The node of interest.
   * @return An index between 0 and 3.
   */
  int getNodeNumber(SpaceNode<T>* node) const;

  /**
   * Determines which index a given triangle has in this tetrahedron's list of
   * incident triangles.
   *
   * @param triangle
   *            The triangle of interest.
   * @return An index between 0 and 3.
   */
  int getTriangleNumber(const std::shared_ptr<Triangle3D<T>>& triangle) const;

  /**
   * Determines the edge that connects two endpoints of this tetrahedron.
   *
   * @param node_number_1
   *            The index of the first endpoint of the edge.
   * @param node_number_2
   *            The index of the second endpoint of the edge.
   * @return The edge connecting the two endpoints with the given indices.
   */
  Edge<T>* getEdge(int node_number_1, int node_number_2) const;

  /**
   * Determines the edge that connects two endpoints of this tetrahedron.
   *
   * @param a
   *            The first endpoint of the edge.
   * @param b
   *            The second endpoint of the edge.
   * @return The edge connecting the two given endpoints.
   */
  Edge<T>* getEdge(SpaceNode<T>* a, SpaceNode<T>* b) const;

  /**
   * Determines the edge that connects two endpoints of this tetrahedron.
   *
   * @param a
   *            The first endpoint of the edge.
   * @param b
   *            The second endpoint of the edge.
   * @return A number between 0 and 5, giving the index of the edge of
   *         interest.
   */
  int getEdgeNumber(SpaceNode<T>* a, SpaceNode<T>* b) const;

  /**
   * Returns the incident triangle opposite to a given endpoint of this
   * tetrahedron.
   *
   * @param node
   *            An endpoint of this tetrahedron.
   * @return A reference to the triangle that lies opposite to
   *         <code>node</code>.
   */
  std::shared_ptr<Triangle3D<T>> getOppositeTriangle(SpaceNode<T>* node) const;

  /**
   * Returns the incident node opposite to a given triangle which is incident
   * to this tetrahedron.
   *
   * @param triangle
   *            An incident triangle of this tetrahedron.
   * @return The endpoint of this triangle that lies opposite to
   *         <code>triangle</code>.
   */
  SpaceNode<T>* getOppositeNode(const std::shared_ptr<Triangle3D<T>>& triangle) const;

  /**
   * Ret#include "spatial_organization/space_node.h"urns a reference to the triangle connecting this tetrahedron with
   * another one.
   *
   * @param tetrahedron
   *            An adjacent tetrahedron.
   * @return The triangle which is incident to this tetrahedron and
   *         <code>tetrahedron</code>.
   */
  std::shared_ptr<Triangle3D<T>> getConnectingTriangle(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) const;

  /**
   * Returns this index of the triangle connecting this tetrahedron with
   * another one.
   *
   * @param tetrahedron
   *            An adjacent tetrahedron.
   * @return An index between 0 and 3 which is the position of the triangle
   *         incident to this tetrahedron and <code>tetrahedron</code> in
   *         this tetrahedron's list of incident triangles.
   */
  int getConnectingTriangleNumber(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) const;

  /**
   * Returns the three incident triangles that are adjacent to a given
   * triangle.
   *
   * @param base
   *            A triangle which is incident to this tetrahedron.
   * @return An array of three triangles.
   */
  std::array<std::shared_ptr<Triangle3D<T>>, 3> getTouchingTriangles(const std::shared_ptr<Triangle3D<T>>& base) const;

  /**
   * Removes this tetrahedron from the triangulation. All the incident nodes,
   * edges and triangles are informed that this tetrahedron is being removed.
   *
   * !IMPORTANT!: No triangle organizer is informed about the removement of
   * this tetrahedron. A caller of this function must keep track of the new
   * open triangles itself!
   */
  void remove();

  /**
   * Determines whether a given coordinate lies in convex position, meaning
   * that the incident triangle with list index
   * <code>connectingTriangleNumver</code> is truly cut by a line connecting
   * the given coordinate and the endpoint of this tetrahedron that lies
   * opposite to the same triangle.
   *
   * @param point
   *            The coordinate that should be tested.
   * @param connecting_triangle_number
   *            The index of the triangle facing the coordinate.
   * @return <code>true</code>, if the given coordinate truly lies in
   *         convex position and <code>false</code> otherwise.
   */
  virtual bool isPointInConvexPosition(const std::array<double, 3>& point, size_t connecting_triangle_number) const;

  /**
   * Determines whether a given coordinate lies in convex position, meaning
   * that the incident triangle with list index
   * <code>connectingTriangleNumver</code> is cut by a line connecting the
   * given coordinate and the endpoint of this tetrahedron that lies opposite
   * to the same triangle.
   *
   * @param point
   *            The coordinate that should be tested.
   * @param connecting_triangle_number
   *            The index of the triangle facing the coordinate.
   * @return 1, if the given coordinate lies truly in convex position to this
   *         tetrahedron (meaning that a line connecting the node opposite to
   *         the specified triangle and the given coordinate would cut the
   *         inside of the specified triangle), 0 if the point lies on the
   *         border between convex positions and non-convex position, and -1
   *         if the point lies in a non-convex position.
   */
  virtual int isInConvexPosition(const std::array<double, 3>& point, size_t connecting_triangle_number) const;

  /**
   * Returns the second tetrahedron that is incident to the incident triangle with index <code>number</code>.
   * @param number An index specifying a position in the list of triangles of this tetrahedron. The
   * corresponding triangle will be chosen to determine the adjacent tetrahedron.
   * @return An adjacent tetrahedron.
   */
  std::shared_ptr<Tetrahedron<T>> getAdjacentTetrahedron(int number);

  /**
   * Checks if a node may be moved to a given coordinate.
   * @param position The coordinate of interest.
   * @throws PositionNotAllowedException If the position is equal to any endpoint of this tetrahedron.
   */
  void testPosition(const std::array<double, 3>& position) const throw (std::exception);

  /**
   * When this tetrahedron is removed, there might still be references to
   * this tetrahedron.  Therefore, a flag is set to save that this tetrahedron was removed and
   * this can be read using this function.
   *
   * @return <code>true</code>, iff this tetrahedron is still part of the triangulation.
   */
  bool isValid() const;

  /**
   * Returns whether a given tetrahedron is adjacent to this tetrahedron.
   * @param other_tetrahedron The potential neighbor of this tetrahedron.
   * @return <code>true</code>, iff this tetrahedron is adjacent to <code>otherTetrahedron</code>.
   */
  bool isNeighbor(const std::shared_ptr<Tetrahedron<T>>& other_tetrahedron) const;

  /**
   * Given two nodes incident to this tetrahedron, this function returns
   * another endpoint. The returned endpoint is different from the result of
   * {@link #getSecondOtherNode(SpaceNode, SpaceNode)}.
   *
   * @param node_a
   *            A first incident node.
   * @param node_b
   *            A second incident node.
   * @return A third incident node.
   */
  SpaceNode<T>* getFirstOtherNode(SpaceNode<T>* node_a, SpaceNode<T>* node_b) const;

  /**
   * Given two nodes incident to this tetrahedron, this function returns
   * another endpoint. The returned endpoint is different from the result of
   * {@link #getFirstOtherNode(SpaceNode, SpaceNode)}.
   *
   * @param node_a
   *            A first incident node.
   * @param node_b
   *            A second incident node.
   * @return A third incident node.
   */
  SpaceNode<T>* getSecondOtherNode(SpaceNode<T>* node_a, SpaceNode<T>* node_b) const;

 protected:
  /**
   * Initialization is done within initializationHelper methods
   * Have a look at their documentation to understand why
   */
  Tetrahedron();

  /**
   * Initialization code that cannot be called inside the constructor, because it passes
   * a std::shared_ptr of itself to another function.
   * It does that using shared_from_this(). @see create function documentation for a more
   * detailed explanation of the requirements before calling shared_from_this()
   *
   * Constructs a new tetrahedron from a given triangle and a fourth point.
   * Missing triangles are created.
   *
   * @param one_triangle
   *            The triangle delivering 3 of the new tetrahedron's endpoints.
   * @param fourth_point
   *            The fourth endpoint of the new tetrahedron.
   * @param org
   *            An organizer for open triangles which is used to keep track of
   *            newly created triangles.
   */
  void initializationHelper(const std::shared_ptr<Triangle3D<T>>& one_triangle, SpaceNode<T>* fourth_point,
                            const std::shared_ptr<OpenTriangleOrganizer<T>>& oto);

  /**
   * Initialization code that cannot be called inside the constructor, because it passes
   * a std::shared_ptr of itself to another function.
   * It does that using shared_from_this(). @see create function documentation for a more
   * detailed explanation of the requirements before calling shared_from_this()
   *
   * Creates a new tetrahedron from four triangles and four points.
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
  void initializationHelper(const std::shared_ptr<Triangle3D<T>>& triangle_a,
                            const std::shared_ptr<Triangle3D<T>>& triangle_b,
                            const std::shared_ptr<Triangle3D<T>>& triangle_c,
                            const std::shared_ptr<Triangle3D<T>>& triangle_d, SpaceNode<T>* node_a,
                            SpaceNode<T>* node_b, SpaceNode<T>* node_c, SpaceNode<T>* node_d);

  /**
   * A small list containing a permutation of the number 0-3. This list is
   * used to randomize the visibility walk implemented in
   * {@link #walkToPoint(double[])}.
   */
  static std::array<int, 4> triangle_order_;
  /**
   * Contains references to the nodes incident to this tetrahedron.
   */
  std::array<SpaceNode<T>*, 4> adjacent_nodes_;
  /**
   * Contains references to the 6 edges incident to this tetrahedron.
   */
  std::array<Edge<T>*, 6> adjacent_edges_;
  /**
   * Contains references to the 4 triangles incident to this tetrahedron.
   */
  std::array<std::shared_ptr<Triangle3D<T>>, 4> adjacent_triangles_;
  /**
   * Saves for each incident edge this tetrahedron's contribution to the
   * cross-section area of that edge.
   */
  std::array<double, 6> cross_section_areas_;
  /**
   * The center of the circumsphere of this tetrahedron.
   */
  std::array<double, 3> circum_center_;

  bool circum_center_is_null_;

  /**
   * The square of the radius of this tetrahedron's circumsphere.
   */
  double squared_radius_;

  /**
   * Defines a tolerance_ intervall which is used in
   * {@link #orientation(double[])}. If the distance of a given point to the
   * center of the circumsphere of this tetrahedron has a difference to
   * <co#include "spatial_organization/space_node.h"de>squared_radius_</code> smaller than <code>tolerance_</code>,
   * exact mathematics are used to reliably calculate a decision whether that
   * point lies inside, outside or on the circumsphere.
   */
  double tolerance_;

  /**
   * The volume_ of this tetrahedron.
   */
  double volume_;

  /**
   * Determines whether this tetrahedron is still a part of the triangulation.
   */
  bool valid_;

 private:
  /**
   * Used during initialization to make sure that edges are created only once.
   */
  void registerEdges();

  /**
   * Changes the volume_ associated with this tetrahedron to a new value. The
   * incident nodes are informed about the volume_ change.
   *
   * @param new_volume
   *            The new volume_.
   */
  void changeVolume(double new_volume);

  /**
   * Calculates the vectors connecting the first endpoint of this tetrahedron
   * with the other three points.
   *
   * @return A two-dimensional array of length 3x3 and type double, containing
   *         three vectors.
   */
  std::array<std::array<double, 3>, 3> getPlaneNormals() const;

  /**
   * Finds the maximal absolute value in a two-dimensional array. Used for
   * tolerance_ interval calculations.
   *
   * @param values
   *            The array which should be searched.
   * @return The entry in <code>values</code> with the highest absolute
   *         value.
   */
  double maxAbs(const std::array<std::array<double, 3>, 3>& values) const;

  /**
   * Finds the maximal absolute value in four arrays. Used for tolerance_
   * interval calculations.
   *
   * @param values_1
   *            The first array to be searched.
   * @param values_2
   *            The second array to be searched.
   * @param values_3
   *            The third array to be searched.
   * @param values_4
   *            The fourth array to be searched.
   * @return The entry with the highest absolute value in any of the 4 given
   *         arrays.
   */
  double maxAbs(const std::array<double, 3>& values_1, const std::array<double, 3>& values_2,
                const std::array<double, 3>& values_3, const std::array<double, 3>& values_4) const;

  double multError2(double a, double a_err_2, double b, double b_err_2) const;

  double multError2(double a, double a_err_2, double b, double b_err_2, double c, double c_err_2) const;

  double addError2(double a_err_2, double b_err_2, double result) const;

  double addError2(double a_err_2, double b_err_2, double c_err_2, double result) const;

  /**
   * Calculates the center of the circumsphere of this tetrahedron and the
   * volume_. (The latter is simply convenient because the determinant needed
   * to calculate the volume_ is used anyways.)
   *
   * Along with the circumsphere calculation, an upper bound of the
   * uncertainty of this value is calculated.
   */
  void computeCircumsphereCenterAndVolume();

  /**
   * Calculates the radius of this tetrahedron's circumsphere.
   */
  void computeRadius();
};

}  // namespace spatial_organization
}  // namespace bdm

#endif  // SPATIAL_ORGANIZATION_TETRAHEDRON_H_
