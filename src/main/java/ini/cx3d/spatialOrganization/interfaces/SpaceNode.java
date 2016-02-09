package ini.cx3d.spatialOrganization.interfaces;

import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.spatialOrganization.PositionNotAllowedException;
import ini.cx3d.spatialOrganization.SpatialOrganizationEdge;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;
import ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener;

import java.util.AbstractSequentialList;
import java.util.LinkedList;

public interface SpaceNode<T> extends SpatialOrganizationNode<T> {
	@Override
	ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

	/**
	 * Returns a hash code for this SpaceNode, which is equal to its ID number.
	 *
	 * @return The ID number of this node.
	 */
	int hashCode();

	/**
	 * Returns a string representation of this node.
	 *
	 * @return A string containing the ID number of this SpaceNode.
	 */
	String toString();

	/**
	 * Adds a new edge to the list of incident edges.
	 *
	 * @param newEdge
	 *            The edge to be added.
	 */
	void addEdge(Edge newEdge);

	/**
	 * Searches for an edge which connects this SpaceNode with another
	 * SpaceNode.
	 *
	 * @param oppositeNode
	 *            The other node to which the required edge should be connected
	 *            to.
	 * @return An edge connecting this node and <code>oppositeNode</code>. If
	 *         such an edge didn't exist in the list of edges incident to this
	 *         node, a new edge is created.
	 */
	Edge searchEdge(SpaceNode oppositeNode);

	/**
	 * Removes a given edge from the list of incident edges.
	 *
	 * @param e
	 *            The edge to be removed.
	 */
	void removeEdge(Edge e);

	/**
	 * Removes a given tetrahedron from the list of incident tetrahedra.
	 *
	 * @param tetrahedron
	 *            The tetrahedron to be remobed.
	 */
	void removeTetrahedron(Tetrahedron tetrahedron);

	/**
	 * Returns an {@link Iterable} that allows to iterate over all edges
	 * incident to this node.
	 */
	AbstractSequentialList<SpatialOrganizationEdge<T>> getEdges();

	T getUserObject();

	/*
         * (non-Javadoc)
         *
         * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getNeighbors()
         */
	AbstractSequentialList<T> getNeighbors();

	/*
         * (non-Javadoc)
         *
         * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getPermanentListOfNeighbors()
         */
	AbstractSequentialList<T> getPermanentListOfNeighbors();

	/*
         * (non-Javadoc)
         *
         * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getVerticesOfTheTetrahedronContaining(double[])
         */
	Object[] getVerticesOfTheTetrahedronContaining(double[] position);

	/**
	 * Modifies the volume associated with this SpaceNode by a given value.
	 *
	 * @param change
	 *            The change value that will be added to the volume.
	 */
	void changeVolume(double change);

	/*
         * (non-Javadoc)
         *
         * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getVolume()
         */
	double getVolume();

	/*
         * (non-Javadoc)
         *
         * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getNewInstance(double[],
         *      java.lang.Object)
         */
	SpatialOrganizationNode<T> getNewInstance(double[] position,
											  T userObject);// throws PositionNotAllowedException;

	/**
	 * Sets the list of movement listeners attached to this node to a specified
	 * list.
	 *
	 * @param listeners
	 *            The movement listeners that are listening to this node's
	 *            movements.
	 */
	void setListenerList(
			AbstractSequentialList<SpatialOrganizationNodeMovementListener<T>> listeners);

	/*
         * (non-Javadoc)
         *
         * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#addSpatialOrganizationNodeMovementListener(ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener)
         */
	void addSpatialOrganizationNodeMovementListener(
			SpatialOrganizationNodeMovementListener<T> listener);

	/**
	 * @return The list of tetrahedra incident to this node.
	 */
	AbstractSequentialList<Tetrahedron> getAdjacentTetrahedra();

	void addAdjacentTetrahedron(Tetrahedron tetrahedron);

	/*
         * (non-Javadoc)
         *
         * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getPosition()
         */
	double[] getPosition();

	/**
	 * @return The identification number of this SpaceNode.
	 */
	int getId();

	/*
         * (non-Javadoc)
         *
         * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#remove()
         */
	void remove();

	/**
	 * Starting at a given tetrahedron, this function searches the triangulation
	 * for a tetrahedron which contains this node's coordinate.
	 *
	 * @param start
	 *            The starting tetrahedron.
	 * @return A tetrahedron which contains the position of this node.
	 * @throws PositionNotAllowedException
	 */
	Tetrahedron<T> searchInitialInsertionTetrahedron(Tetrahedron<T> start)
			throws PositionNotAllowedException;

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
	Tetrahedron<T> insert(Tetrahedron<T> start)
			throws PositionNotAllowedException;

	/**
	 * Restores the Delaunay property for the current triangulation after a movement of this node.
	 * If the triangulation remains a valid triangulation after a node movement, a sequence of
	 * 2->3 flips and 3->2 flips can often times restore the Delaunay property for the triangulation.
	 * This function first applies such a flip algorithm starting at all tetrahedra incident to this node.
	 * If the Delaunay property cannot be restored using this flip algorithm, a cleanup procedure is used,
	 * which removes all tetrahedra that cause a problem and then re-triangulates the resulted hole.
	 */
	void restoreDelaunay();

	/* (non-Javadoc)
         * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#moveFrom(double[])
         */
	void moveFrom(double[] delta) throws PositionNotAllowedException;

	/**
	 * Moves this node to a new position.
	 * @param newPosition The new coordinate for this node.
	 * @throws PositionNotAllowedException
	 */
	void moveTo(double[] newPosition) throws PositionNotAllowedException;

	/**
	 * Proposes a new position for a node that was moved to the same coordinate as this node.
	 * @return A coordinate which can be used to place the problematic node.
	 */
	double[] proposeNewPosition();

	/**
	 * Returns a list of all edges that are incident to this node.
	 * @return A list of edges.
	 */
	AbstractSequentialList<SpatialOrganizationEdge<T>> getAdjacentEdges();
}
