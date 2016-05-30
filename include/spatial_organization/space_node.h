#ifndef SPATIAL_ORGANIZATION_SPACE_NODE_H_
#define SPATIAL_ORGANIZATION_SPACE_NODE_H_

#include <array>
#include <stdexcept>
#include <string>
#include <list>
#include <vector>
#include <memory>

#include "sim_state_serializable.h"
#include "spatial_organization/spatial_organization_node_movement_listener.h"
#include "spatial_organization/spatial_organization_node.h"
#include "spatial_organization/edge.h"

namespace cx3d {
namespace spatial_organization {

template<class T> class Triangle3D;
template<class T> class Tetrahedron;
template<class T> class OpenTriangleOrganizer;
template<class T> class SpatialOrganizationEdge;

using spatial_organization::SpatialOrganizationNodeMovementListener;

/**
 * This class is used to represent a node of a triangulation. Each node is
 * stores information about incident tetrahedra and edges (arbitrary amounts).
 *
 * @param <T> The type of the user objects associated with each node.
 */
template<class T>
class SpaceNode : public SpatialOrganizationNode<T>, public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<SpaceNode<T>>;

  static void reset();

  /**
   * Starting at a given tetrahedron, this function searches the triangulation
   * for a tetrahedron which contains a given coordinate.
   *
   * @param <T>
   *            The type of user objects that are associated to nodes in the
   *            current triangulation.
   * @param start
   *            The starting tetrahedron.
   * @param coordinate
   *            The coordinate of interest.
   * @return A tetrahedron which contains the position of this node.
   * @throws PositionNotAllowedException
   */
  static std::shared_ptr<Tetrahedron<T>> searchInitialInsertionTetrahedron(
      const std::shared_ptr<Tetrahedron<T>>& start,
      const std::array<double, 3>& coordinate);
//        throws PositionNotAllowedException {

  /**
   * Creates a new SpaceNode with at a given coordinate and associates it with
   * a user object.
   *
   * @param position
   *            The position for this SpaceNode.
   * @param content
   *            The user object that should be associated with this SpaceNode.
   */
  SpaceNode(const std::array<double, 3>& position,
            T* content);

  /**
   * Creates a new SpaceNode with at a given coordinate and associates it with
   * a user object.
   *
   * @param x
   *            The x-coordinate for this SpaceNode.
   * @param y
   *            The y-coordinate for this SpaceNode.
   * @param z
   *            The z-coordinate for this SpaceNode.
   * @param content
   *            The user object that should be associated with this SpaceNode.
   */
  SpaceNode(double x, double y, double z, T* content);

  virtual ~SpaceNode() {
  }

  virtual void addSpatialOrganizationNodeMovementListener(typename SpatialOrganizationNodeMovementListener<T>::UPtr listener)
          override;

  virtual std::list<Edge<T>* > getEdges() const override;  //TODO change back to SpatialOrganizationEdge afte porting has been finished

  virtual std::list<T*> getNeighbors() const override;

  virtual UPtr getNewInstance(const std::array<double, 3>& position, T* user_object) override;

  virtual std::list<T*> getPermanentListOfNeighbors() const override;

  virtual std::array<double, 3> getPosition() const override;

  virtual T* getUserObject() const override;

  //TODO clean up this hack after porting has been finished
  virtual std::array<T*, 4> getVerticesOfTheTetrahedronContaining(
      const std::array<double, 3>& position,
      std::array<int, 1>& returned_null) const override;

  virtual double getVolume() const override;

  virtual void moveFrom(const std::array<double, 3>& delta) override;  //fimxe throw(PositionNotAllowedException)

  virtual void remove() override;

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  /**
   * @return The list of tetrahedra incident to this node.
   */
  virtual std::list<std::shared_ptr<Tetrahedron<T>> > getAdjacentTetrahedra() const;

  /**
   * Adds an element to the list of adjacent tetrahedra incident to this node.
   */
  // todo rename to addTetrahedron to be consistent
  virtual void addAdjacentTetrahedron(
      const std::shared_ptr<Tetrahedron<T>>& tetrahedron);

  /**
   * Removes a given tetrahedron from the list of incident tetrahedra.
   *
   * @param tetrahedron
   *            The tetrahedron to be remobed.
   */
  virtual void removeTetrahedron(
      const std::shared_ptr<Tetrahedron<T>>& tetrahedron);

  /**
   * Moves this node to a new position.
   * @param new_position The new coordinate for this node.
   * @throws PositionNotAllowedException
   */
  virtual void moveTo(const std::array<double, 3>& new_position);

  /**
   * Modifies the volume associated with this SpaceNode by a given value.
   *
   * @param change
   *            The change value that will be added to the volume.
   */
  virtual void changeVolume(double change);

  /**
   * Adds a new edge to the std::list of adjacent edges.
   *
   * @param newEdge
   *            The edge to be added.
   */
  virtual void addEdge(const typename Edge<T>::SPtr& edge);

  /**
   * @return The identification number of this SpaceNode.
   */
  virtual int getId() const;

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
  virtual Edge<T>* searchEdge(
      SpaceNode<T>* opposite_node);

  /**
   * Removes a given edge from the std::list of incident edges.
   *
   * @param edge
   *            The edge to be removed.
   */
  virtual void removeEdge(Edge<T>* edge);

  /**
   * Starting at a given tetrahedron, this function searches the triangulation
   * for a tetrahedron which contains this node's coordinate.
   *
   * @param start
   *            The starting tetrahedron.
   * @return A tetrahedron which contains the position of this node.
   * @throws PositionNotAllowedException
   */
  virtual std::shared_ptr<Tetrahedron<T>> searchInitialInsertionTetrahedron(
      const std::shared_ptr<Tetrahedron<T>>& start);

  /**
   * Inserts this node into a triangulation. Given any tetrahedron which is part of the triangulation,
   * a stochastic visibility walk is performed in order to find a tetrahedron which contains the position of this node.
   * Starting from this tetrahedron, all tetrahedra that would contain this point in their
   * circumsphere are removed from the triangulation. Finally, the gap inside the triangulation which was created
   * is filled by creating a star-shaped triangulation.
   * @param start Any tetrahedron of the triangulation which will be used as a starting point for the
   * stochastic visibility walk.
   * @return A tetrahedron which was created while inserting this node.
   * @throws PositionNotAllowedException
   */
  virtual std::shared_ptr<Tetrahedron<T>> insert(
      const std::shared_ptr<Tetrahedron<T>>& start);

  /**
   * Restores the Delaunay property for the current triangulation after a movement of this node.
   * If the triangulation remains a valid triangulation after a node movement, a sequence of
   * 2->3 flips and 3->2 flips can often times restore the Delaunay property for the triangulation.
   * This function first applies such a flip algorithm starting at all tetrahedra incident to this node.
   * If the Delaunay property cannot be restored using this flip algorithm, a cleanup procedure is used,
   * which removes all tetrahedra that cause a problem and then re-triangulates the resulted hole.
   */
  virtual void restoreDelaunay();

  /**
   * Proposes a new position for a node that was moved to the same coordinate as this node.
   * @return A coordinate which can be used to place the problematic node.
   */
  virtual std::array<double, 3> proposeNewPosition();

  /**
   * Returns a string representation of this node.
   */
  virtual std::string toString() const override;

  /**
   * Determines if two instances of this object are equal
   */
  virtual bool equalTo(SpaceNode<T>* other);

 private:
  /**
   * Creates an unique identifier that is used while restoring the Delaunay property.
   * While running the flip algorithm, each triangle has to be tested whether it is still
   * locally Delaunay. In order to make sure that no triangle is tested more than once,
   * tested triangles are tagged with a checking index.
   * @return A unique index.
   */
  static int createNewCheckingIndex();

  SpaceNode() = delete;
  SpaceNode(const SpaceNode&);
  SpaceNode& operator=(const SpaceNode&) = delete;

  /**
   * A static variable that is used to assign a unique number to each
   * initialized flip process.
   */
  static int checking_index_;

  /**
   * A static counter used to keep track of the number of created SpaceNodes.
   */
  static int id_counter_;

  static std::vector<int> triangle_order_;

  /**
   * The ID number of this SpaceNode.
   */
  int id_;

  /**
   * The user object associated with this SpaceNode.
   */
  T* content_ = nullptr;

  /**
   * A std::list of std::listener objects that are called whenever this node is beeing
   * moved.
   */
  std::vector<std::unique_ptr<SpatialOrganizationNodeMovementListener<T>>> listeners_;

  /**
   * The coordinate of this SpaceNode.
   */
  std::array<double, 3> position_;

  /**
   * A std::list of all edges incident to this node.
   */
  std::list<typename Edge<T>::SPtr > adjacent_edges_;

  /**
   * A std::list of all tetrahedra incident to this node.
   */
  std::list<std::shared_ptr<Tetrahedron<T>> > adjacent_tetrahedra_;

  /**
   * The volume associated with this SpaceNode.
   */
  double volume_;

  std::shared_ptr<Tetrahedron<T>> removeAndReturnCreatedTetrahedron();

  /**
   * A private function used inside {@link #insert(Tetrahedron)} to remove a given tetrahedron,
   * inform an open triangle organizer about a set of new open triangles and add all tetrahedrons
   * adjacent to the removed tetrahedron to a queue.
   * @param tetrahedron The tetrahedron that should be removed.
   * @param queue The queue which is used to keep track of candidates that might have to be removed.
   * @param oto The open triangle organizer that keeps track of all open triangles.
   */
  void processTetrahedron(std::shared_ptr<Tetrahedron<T>>& tetrahedron,
                          std::list<std::shared_ptr<Triangle3D<T>> >& queue,
                          std::shared_ptr<OpenTriangleOrganizer<T>>& oto);

  /**
   * Determines if the current triangulation would still be valid when this node would be moved to a
   * given coordinate. (This function does not check whether the Delaunay criterion would still be
   * fullfilled.)
   * @param newPosition The new coordinate to which this point should be moved to.
   * @return <code>true</code>, if the triangulation would not be corrupted by moving this node to
   * the specified position and <code>false</code>, if not.
   * @throws PositionNotAllowedException
   */
  bool checkIfTriangulationIsStillValid(
      const std::array<double, 3>& new_position);

  /**
   * Removes a tetrahedron from the triangulation and adds all adjacent tetrahedra to a linked list.
   * This function is used when the flip algorithm did not succeed in restoring the Delaunay property.
   * It removes a tetrahedron, adds all adjacent tetrahedra to a list and adds all incident nodes to a list.
   * Whenever open triangles are created, an open triangle organizer is informed.
   * @param tetrahedron_to_remove The tetrahedron that should be removed.
   * @param list The list of candidate tetrahedra which might have to be deleted.
   * @param node_list A list of nodes that keeps track of all nodes which were incident to
   * removed tetrahedra during a run of {@link #cleanUp(LinkedList)}.
   * @param oto An open triangle organizer which is used to keep track of open triangles.
   * @return <code>true</code>, if any of the two lists <code>list</code> of <code>nodeList</code>
   * were modified during this function call.
   */
  bool removeTetrahedronDuringCleanUp(
      std::shared_ptr<Tetrahedron<T>>& tetrahedron_to_remove,
      std::list<std::shared_ptr<Tetrahedron<T>> >& list,
      std::list<SpaceNode<T>* >& node_list,
      std::shared_ptr<OpenTriangleOrganizer<T>>& oto);
  /**
   * Restores the Delaunay criterion for  a section of the triangulation which
   * cannot be restored using a flip algorithm.
   * Whenever the flip algorithm fails to restore the Delaunay criterion, this function is called.
   * First, all tetrahedra that cause a problem are removed along with all adjacent tetrahedra that
   * have the same circumsphere. Then, the resulting hole in the triangulation is filled using
   * {@link OpenTriangleOrganizer#triangulate()}.
   * @param messed_up_tetrahedra A set of tetrahedra that are not Delaunay and cannot be replaced using a
   * flip algorithm.
   */
  void cleanUp(const std::list<std::shared_ptr<Tetrahedron<T>> >& messed_up_tetrahedra);

  /**
   * returns a permutation of the vector triangle_order_
   */
  static std::array<int, 4> generateTriangleOrder();

  /**
   * helper function used by function generateTriangleOrder
   */
  static int randomHelper(int i);
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_SPACE_NODE_H_
