#ifndef SPATIAL_ORGANIZATION_OPEN_TRIANGLE_ORGANIZER_H_
#define SPATIAL_ORGANIZATION_OPEN_TRIANGLE_ORGANIZER_H_

#include <list>
#include <stack>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "spatial_organization/simple_triangulation_node_organizer.h"
#include "spatial_organization/triangle_hash_key.h"

#ifdef OPENTRIANGLEORGANIZER_DEBUG
#include "spatial_organization/debug/open_triangle_organizer_debug.h"
#endif

namespace cx3d {
namespace spatial_organization {

class Rational;
template<class T> class SpaceNode;
template<class T> class Triangle3D;
template<class T> class Tetrahedron;
template<class T> class EdgeHashKey;
template<class T> struct EdgeHashKeyHash;
template<class T> struct EdgeHashKeyEqual;

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
class OpenTriangleOrganizer : public std::enable_shared_from_this<
    OpenTriangleOrganizer<T>> {
 public:
#ifndef OPENTRIANGLEORGANIZER_NATIVE
  OpenTriangleOrganizer()
      : shortest_distance_(0) {
  }
#endif
  virtual ~OpenTriangleOrganizer() {
  }

  static std::shared_ptr<OpenTriangleOrganizer<T>> create(
      int preferred_capacity,
      const std::shared_ptr<SimpleTriangulationNodeOrganizer<T>>& tno) {
#ifndef OPENTRIANGLEORGANIZER_DEBUG
    auto raw_ptr = new OpenTriangleOrganizer<T>(preferred_capacity, tno);
#else
    auto raw_ptr = new OpenTriangleOrganizerDebug<T>(preferred_capacity, tno);
#endif
    std::shared_ptr<OpenTriangleOrganizer<T>> s_ptr(raw_ptr);
    return s_ptr;
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
    auto tno = SimpleTriangulationNodeOrganizer<T>::create();
    return OpenTriangleOrganizer<T>::create(30, tno);
  }

  /**
   * Starts the recording of newly created tetrahedra.
   */
  virtual void recoredNewTetrahedra();

  /**
   * Returns the list of newly created tetrahedra.
   * @return The list of newly created tetrahedra, if recording was turned on before
   * (Use {@link #recoredNewTetrahedra()}) or <code>null</code> else.
   */
  virtual std::list<std::shared_ptr<Tetrahedron<T> > > getNewTetrahedra();

  /**
   * Returns an arbitrary tetrahedron which was created during the process of
   * triangulation.
   * @return A tetrahedron which was created during triangulation.
   */
  virtual std::shared_ptr<Tetrahedron<T>> getANewTetrahedron();

  /**
   * Removes one tetrahedron from the triangulation and possibly all adjacent tetrahedra that have the same circumsphere as the
   * first tetrahedron.
   * @param startingTetrahedron The first tetrahedron to remove.
   */
  virtual void removeAllTetrahedraInSphere(
      const std::shared_ptr<Tetrahedron<T>>& starting_tetrahedron);

  /**
   * Informs this open triangle organizer that a new
   * open triangle is available. In order to do so, the new open
   * triangle is added to the hashmap.
   * @param triangle The new open triangle.
   */
  virtual void putTriangle(const std::shared_ptr<Triangle3D<T>>& triangle);

  /**
   * Informs this open triangle organizer that an open triangle
   * is no longer available. In order to do so, the new open
   * triangle is removed from the hashmap.
   * @param triangle The triangle that should be removed.
   */
  virtual void removeTriangle(const std::shared_ptr<Triangle3D<T> >& triangle);

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
      const std::shared_ptr<SpaceNode<T>>& c);

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
      const std::shared_ptr<SpaceNode<T>>& c);

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
  virtual void triangulate();

  /**
   * Returns a String representation of this OpenTriangleOrganizer
   */
  virtual std::string toString() const;

  /**
   * Determines if two instances of this object are equal
   */
  virtual bool equalTo(const std::shared_ptr<OpenTriangleOrganizer<T>>& other) const;

 protected:
  /**
   * Determines if a triangle with the three specified endpoints is stored in
   * the hashmap for open triangles already.
   * @param a The first node incident to the searched triangle.
   * @param b The second node incident to the searched triangle.
   * @param c The third node incident to the searched triangle.
   * @return <code>true</code>, if an open triangle with the desired endpoints
   * is already stored in this triangle organizer.
   */
  virtual bool contains(const std::shared_ptr<SpaceNode<T>>& a,
                const std::shared_ptr<SpaceNode<T>>& b,
                const std::shared_ptr<SpaceNode<T>>& c) const;

  /**
   * Returns whether this triangle organizer is storing any more open triangles.
   * @return <code>true</code>, if the hashmap storing empty triangles is empty.
   */
  virtual bool isEmpty() const;

  OpenTriangleOrganizer(
      int preferredCapacity,
      const std::shared_ptr<SimpleTriangulationNodeOrganizer<T>>& tno);

 private:
#ifdef OPENTRIANGLEORGANIZER_NATIVE
  OpenTriangleOrganizer() = delete;
#endif
  OpenTriangleOrganizer(const OpenTriangleOrganizer&) = delete;
  OpenTriangleOrganizer& operator=(const OpenTriangleOrganizer&) = delete;

  /**
   * A hashmap to store open triangles that are created during any operation on the triangulation.
   * Used to keep track of which triangles are not open any more.
   */
  std::unordered_map<TriangleHashKey<T>, std::shared_ptr<Triangle3D<T>>, TriangleHashKeyHash<T>, TriangleHashKeyEqual<T>>map_;

  /**
   * Stores all tetrahedra that are created by this instance. No tetrahedra will
   * be recorded if this reference is <code>null</code>, which is the default.
   * Use {@link #recoredNewTetrahedra()} to start recording.
   */
  std::list<std::shared_ptr<Tetrahedron<T>>> new_tetrahedra_;

  /**
   * A node organizer used to keep track of all nodes that are adjacent to
   * any 'hole' in the triangulation.
   * These nodes are potential candidates for combination with open triangles.
   */
  std::shared_ptr<SimpleTriangulationNodeOrganizer<T>> tno_;

  /**
   * In addition to <code>map</code>, all open triangles are also stored in this stack.
   * Thereby, a fast extraction of any open triangle is guaranteed.
   */
  std::stack<std::shared_ptr<Triangle3D<T> > > open_triangles_;

  /**
   * Stores the shortest signed delaunay distance that was found for a given open
   * triangle.
   */
  double shortest_distance_;

  /**
   * A link to a tetrahedron that was created during the completion of an
   * incomplete triangulation.
   */
  std::shared_ptr<Tetrahedron<T>> a_new_tetrahedron_;  //fixme remove = null;

  /**
   * Returns an open triangle if there is any left. This function uses the stack {@link #openTriangles}
   * to search for open triangles.
   * @return An open triangle if there is still an open triangle left
   * and <code>null</code> else.
   */
  std::shared_ptr<Triangle3D<T>> getAnOpenTriangle();

  /**
   * Stores an edge onto a hashmap for edges. This function implements the
   * same functinoality as {@link #putTriangle(ini.cx3d.spatialOrganization.interfaces.Triangle3D)} but for edges.
   * @param a The first endpoint of the edge that should be placed on the hashmap.
   * @param a The second endpoint of the edge that should be placed on the hashmap.
   * @param oppositeNode A node lying on the non-open side of this edge.
   * @param oldOpenEdge If no pre-existing edge was found, this edge is returned.
   * @param map The hashmap on which the specified edge should be placed.
   * @return An instance of <code>EdgeHashKey</code> which points to the specified
   * two endpoints.
   */
  std::shared_ptr<EdgeHashKey<T>> putEdgeOnMap(
      const std::shared_ptr<SpaceNode<T>>& a,
      const std::shared_ptr<SpaceNode<T>>& b,
      const std::shared_ptr<SpaceNode<T>>& opposite_node,
      const std::shared_ptr<EdgeHashKey<T>>& old_open_edge,
      std::unordered_map<EdgeHashKey<T>, std::shared_ptr<EdgeHashKey<T>>, EdgeHashKeyHash<T>, EdgeHashKeyEqual<T> >& map);

  /**
   * Finds the node with lowest id in a list of nodes.
   * @param nodes The list of nodes.
   * @return The node with lowest id.
   */
  std::shared_ptr<SpaceNode<T>> findCenterNode(const std::list<std::shared_ptr<SpaceNode<T> > >& nodes);

  /**
   * Creates a two-dimensional triangulation for a set of points that all lie on one plane and on the
   * border of one circle.
   * @param sortedNodes A list of nodes, which is expected to be sorted by occurrence on the circle.
   * @param centerNode The node with the lowest id.
   * @param map A hashmap for instances of <code>EdgeHashKey</code>. Used to keep track of 'open edges'.
   * @param triangleList A list of triangles to which all the triangles created within this function will be
   * added.
   * @return An open edge on the convex hull of the triangulated circle.
   */
  std::shared_ptr<EdgeHashKey<T>> triangulateSortedCirclePoints(
      const std::list<std::shared_ptr<SpaceNode<T> > >& sorted_nodes,
      const std::shared_ptr<SpaceNode<T>>& center_node,
      std::unordered_map<EdgeHashKey<T>, std::shared_ptr<EdgeHashKey<T>>, EdgeHashKeyHash<T>, EdgeHashKeyEqual<T> >& map,
      std::list<std::shared_ptr<Triangle3D<T> > >& triangle_list);

  /**
   * Given an order of nodes that all lie on the surface of one circle, this function searches for triangles in the triangulation that
   * do not match the standardized triangulation of this circle. In a standardized triangulation, every triangle is incident to two points
   * which are successors on the circle and to the one point with lowest ID.
   *
   * Triangles that violate the standardized triangulation are removed together with incident tetrahedra.
   * @param sortedNodes A list of nodes that lie on one circle. This list is expected to be sorted in terms of angular
   * neighborhoodship of nodes.
   */
  void removeForbiddenTriangles(const std::list<std::shared_ptr<SpaceNode<T> > >& sorted_nodes);

  /**
   * Sorts a list of nodes that all lie on one circle.
   * @param nodes The list of nodes that should be sorted.
   * @param startingEdge Any pre-existing edge.
   * @param centerNode The node with lowest ID.
   * @return A sorted list of nodes. The first node in this list will be the node with lowest ID, successive nodes are sorted
   * according to their occurrence on the circle.
   */
  std::list<std::shared_ptr<SpaceNode<T> > > sortCircleNodes(
      std::list<std::shared_ptr<SpaceNode<T> > >& nodes,
      std::shared_ptr<EdgeHashKey<T>> startingEdge, const std::shared_ptr<SpaceNode<T> >& center_node);

  /**
   * Creates a two-dimensional triangulation for a set of 4 or more points that lie in the same plane and on the border of the same circle.
   * @param similarDistanceNodes A list of nodes that have a similar 2D-signed delaunay distance to the  <code>startingEdge</code>.
   * @param startingEdge An initial edge which is part of the circle.
   * @param map A hashmap for instances of type <code>EdgeHashKey</code> which is used to keep track of open edges in
   * {@link #triangulatePointsOnSphere(LinkedList, LinkedList, ini.cx3d.spatialOrganization.interfaces.Triangle3D)}.
   * @param triangleList A list of triangles, which is used to store all triangles created in this function and
   * pass them to the calling function.
   * @return An open edge on the convex hull of the triangulated circle.
   */
  std::shared_ptr<EdgeHashKey<T>> triangulatePointsOnCircle(
      std::list<std::shared_ptr<SpaceNode<T> > >& similar_distance_nodes,
      const std::shared_ptr<EdgeHashKey<T>>& starting_edge,
      std::unordered_map<EdgeHashKey<T>, std::shared_ptr<EdgeHashKey<T>>, EdgeHashKeyHash<T>, EdgeHashKeyEqual<T> >& map,
      std::list<std::shared_ptr<Triangle3D<T> > >& triangle_list);  //todo order of parameters not conform with coding style - output param at the end

  /**
   * Creates a tetrahedralization for a set of 5 or more points that lie on the surface of one sphere.
   * @param nodes A list of nodes that have the same signed delaunay distance to <code>startingTriangle</code>.
   * @param onCircleNodes A list of nodes that lie on one common circle with <code>startingTriangle</code>.
   * @param startingTriangle An initial triangle which will be part of the triangulation.
   */
  void triangulatePointsOnSphere(
      std::list<std::shared_ptr<SpaceNode<T> > >& nodes,
      std::list<std::shared_ptr<SpaceNode<T> > >& on_circle_nodes,
      const std::shared_ptr<Triangle3D<T>>& starting_triangle);  //todo order of parameters not conform with coding style - output param at the end

  /**
   * Calculates the 2D-signed delaunay distance of a point to an edge.
   * The edge is here defined by the coordinates of its endpoints. This function uses
   * precise arithmetics to assure correctness of the result.
   *
   * <p><b>HAS NEVER BEEN USED NOR TESTED! USE AT YOUR OWN RISK!</b>`
   * @param av The coordinate of the first endpoint of the edge.
   * @param bv The coordinate of the second endpoint of the edge.
   * @param thirdPoint The point for which the signed delaunay distance should be calculated.
   * @return A rational number that contains the squared signed delaunay distance of the
   * specified edge to the given coordinate.
   */
  std::shared_ptr<Rational> calc2DSDDistanceExact(const std::array<double, 3>& av, const std::array<double, 3>& bv, const std::array<double, 3>& third_point);

  /**
   * Creates a triangle that can serve as an initial triangle for a triangulation.
   * Whenever an open triangle organizer is given a set of points but no open triangles,
   * the function {@link #triangulate()} calls this function to compute a starting triangle.
   * <p><b>HAS NEVER BEEN USED NOR TESTED! USE AT YOUR OWN RISK!</b>`
   *
   * @param rep A reporter object that serves to provide distance information to a
   * node organizer.
   */
  void createInitialTriangle();

  /**
   * Creates a new tetrahedron from a given triangle and a fourth node.
   * The result is stored in {@link #aNewTetrahedron} (and also in {@link #newTetrahedra} if this is
   * not null.
   * @param openTriangle One side of the new tetrahedron.
   * @param oppositeNode The fourth point of the tetrahedron.
   */
  void createNewTetrahedron(const std::shared_ptr<Triangle3D<T>>& openTriangle, const std::shared_ptr<SpaceNode<T>>& opposite_node);
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_OPEN_TRIANGLE_ORGANIZER_H_
