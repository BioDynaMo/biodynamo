/*
Copyright (C) 2009 Frédéric Zubler, Rodney J. Douglas,
Dennis Göhlsdorf, Toby Weston, Andreas Hauri, Roman Bauer,
Sabina Pfister & Adrian M. Whatley.

This file is part of CX3D.

CX3D is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CX3D is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CX3D.  If not, see <http://www.gnu.org/licenses/>.
*/

package ini.cx3d.spatialOrganization;

import java.util.Iterator;
import java.util.LinkedList;
import static ini.cx3d.utilities.Matrix.*;

/**
 * This class is used to represent a node of a triangulation. Each node is
 * stores information about incident tetrahedra and edges (arbitrary amounts).  
 * 
 * @author Dennis Goehlsdorf
 * 
 * @param <T> The type of the user objects associated with each node.
 */
/**
 * @author Dennis
 * 
 * @param <T>
 */
public class SpaceNode<T> implements SpatialOrganizationNode<T> {
	/**
	 * A static list of all nodes that are part of the current triangulation. DO
	 * NOT USE! Exclusively used for debugging purposes. Needs to be removed.
	 * TODO: remove!
	 */
//	public static LinkedList<SpaceNode> allNodes = new LinkedList<SpaceNode>();
	 public static LinkedList<SpaceNode> allNodes = null;

	/**
	 * Number of node movements performed during the current simulation which
	 * were processed by a flipping algorithm. Only for debugging purposes.
	 * TODO: remove!
	 */
	public static int flipMovements = 0;

	/**
	 * Number of node movements performed during the current simulation where a
	 * delete & insert algorithm had to be applied. Only for debugging purposes.
	 * Needs to be removed.
	 * TODO: remove!
	 */
	public static int deleteAndInsertMovements = 0;

	/**
	 * A static variable that is used to assign a unique number to each
	 * initialized flip process.
	 */
	private static int checkingIndex = 0;

	/**
	 * A static counter used to keep track of the number of created SpaceNodes.
	 */
	private static int IDCOUNTER = 0;

	/**
	 * The ID number of this SpaceNode.
	 */
	private int id = IDCOUNTER++;

	/**
	 * The user object associated with this SpaceNode.
	 */
	private T content = null;

	/**
	 * A list of listener objects that are called whenever this node is beeing
	 * moved.
	 */
	private LinkedList<SpatialOrganizationNodeMovementListener<T>> listeners = null;

	/**
	 * The coordinate of this SpaceNode.
	 */
	private double[] position = new double[3];

	// private ExtendedLinkedList<Edge<T>> adjacentEdges = new
	// ExtendedLinkedList<Edge<T>>();
	// private ExtendedLinkedList<Tetrahedron<T> > adjacentTetrahedra = new
	// ExtendedLinkedList<Tetrahedron<T> >();

	/**
	 * A list of all edges incident to this node.
	 */
	private LinkedList<SpatialOrganizationEdge<T>> adjacentEdges = new LinkedList<SpatialOrganizationEdge<T>>();

	/**
	 * A list of all tetrahedra incident to this node.
	 */
	private LinkedList<Tetrahedron<T>> adjacentTetrahedra = new LinkedList<Tetrahedron<T>>();

	/**
	 * The volume associated with this SpaceNode.
	 */
	private double volume = 0;

	/**
	 * Clears the list of all nodes and sets the static node counter to zero.
	 */
	public static void clear() {
		if (allNodes != null)
			allNodes.clear();
		checkingIndex = 0;
		IDCOUNTER = 0;
	}

	/**
	 * Creates a new SpaceNode with at a given coordinate and associates it with
	 * a user object.
	 * 
	 * @param position
	 *            The position for this SpaceNode.
	 * @param content
	 *            The user object that should be associated with this SpaceNode.
	 */
	public SpaceNode(double[] position, T content) {
		this.position = position;
		this.content = content;
		if (allNodes != null) 
			allNodes.add(this);
	}

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
	public SpaceNode(double x, double y, double z, T content) {
		this.position[0] = x;
		this.position[1] = y;
		this.position[2] = z;
		this.content = content;
		if (allNodes != null) 
			allNodes.add(this);
	}

	/**
	 * Returns a hash code for this SpaceNode, which is equal to its ID number.
	 * 
	 * @return The ID number of this node.
	 */
	public int hashCode() {
		return id;
	}

	/**
	 * Returns a string representation of this node.
	 * 
	 * @return A string containing the ID number of this SpaceNode.
	 */
	public String toString() {
		return "" + (this.id % 1000);
	}

	/**
	 * Adds a new edge to the list of incident edges.
	 * 
	 * @param newEdge
	 *            The edge to be added.
	 */
	public void addEdge(Edge<T> newEdge) {
		this.adjacentEdges.addFirst(newEdge);
	}

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
	protected SpatialOrganizationEdge<T> searchEdge(SpaceNode<T> oppositeNode) {
		for (SpatialOrganizationEdge<T> e : this.adjacentEdges) {
			if (e.getOpposite(this) == oppositeNode) return e;
		}
		return new Edge<T>(this, oppositeNode);
	}

	/**
	 * Removes a given edge from the list of incident edges.
	 * 
	 * @param edge
	 *            The edge to be removed.
	 */
	protected void removeEdge(Edge<T> edge) {
		adjacentEdges.remove(edge);
	}

	/**
	 * Removes a given tetrahedron from the list of incident tetrahedra.
	 * 
	 * @param tetrahedron
	 *            The tetrahedron to be remobed.
	 */
	protected void removeTetrahedron(Tetrahedron<T> tetrahedron) {
		adjacentTetrahedra.remove(tetrahedron);
	}

	/**
	 * Returns an {@link Iterable} that allows to iterate over all edges
	 * incident to this node.
	 */
	public Iterable<SpatialOrganizationEdge<T>> getEdges() {
		return adjacentEdges;
		// return new Iterable<SpatialOrganizationEdge<T>>() {
		// public Iterator<SpatialOrganizationEdge<T>> iterator() {
		// final ExtendedLinkedListElement<Edge<T>> last =
		// adjacentEdges.getLastElement();
		// return new Iterator<SpatialOrganizationEdge<T>>() {
		// ExtendedLinkedListElement<Edge<T>> position =
		// adjacentEdges.getFirstElement();
		// // ExtendedLinkedListElement<Edge<T>> next = findNext();
		// // private ExtendedLinkedListElement<Edge<T>> findNext() {
		// // next = position.getNext();
		// // while (next != last &&
		// (next.getContent().getOpposite(SpaceNode.this) == null))
		// // next = next.getNext();
		// // return next;
		// // }
		// public boolean hasNext() {
		// // return next != last;
		// return position.getNext() != last;
		// }
		//
		// public SpatialOrganizationEdge<T> next() {
		// // position = next;
		// // findNext();
		// position = position.getNext();
		// return position.getContent();
		// }
		//
		// public void remove() {
		// throw new UnsupportedOperationException("This Iterator cannot be used
		// to delete elements!");
		// }
		//					
		// };
		// }
		// };
	}

	public T getUserObject() {
		return content;
	}

	private Iterable<T> wrapEdgeListIntoNeighborNodeIterator(
			final LinkedList<SpatialOrganizationEdge<T>> list) {
		return new Iterable<T>() {
			public Iterator<T> iterator() {
				final Iterator<SpatialOrganizationEdge<T>> listIt =
						list.iterator();
				return new Iterator<T>() {
					public boolean hasNext() {
						return listIt.hasNext();
					}

					public T next() {
						return listIt.next().getOpposite(SpaceNode.this)
								.getUserObject();
					}

					public void remove() {
						listIt.remove();
					}
				};
			}
		};
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getNeighbors()
	 */
	public Iterable<T> getNeighbors() {
		return wrapEdgeListIntoNeighborNodeIterator(adjacentEdges);
		// return new Iterable<T>() {
		// public Iterator<T> iterator() {
		// final ExtendedLinkedListElement<Edge<T>> last =
		// adjacentEdges.getLastElement();
		// return new Iterator<T>() {
		// ExtendedLinkedListElement<Edge<T>> position =
		// adjacentEdges.getFirstElement();
		// // ExtendedLinkedListElement<Edge<T>> next = getNext();
		// // private ExtendedLinkedListElement<Edge<T>> getNext() {
		// // ExtendedLinkedListElement<Edge<T>> own = position.getNext();
		// // while (own != last &&
		// (own.getContent().getOpposite(SpaceNode.this) == null))
		// // own = own.getNext();
		// // return own;
		// // }
		// public boolean hasNext() {
		// // return next != last;
		// return position.getNext() != last;
		// }
		//
		// public T next() {
		// // position = next;
		// // next = getNext();
		// position = position.getNext();
		// return
		// position.getContent().getOpposite(SpaceNode.this).getUserObject();
		// }
		//
		// public void remove() {
		// throw new UnsupportedOperationException("This Iterator cannot be used
		// to delete elements!");
		// }
		//					
		// };
		// }
		// };
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getPermanentListOfNeighbors()
	 */
	public Iterable<T> getPermanentListOfNeighbors() {
		// return
		// wrapEdgeListIntoNeighborNodeIterator(((LinkedList<SpatialOrganizationEdge<T>>)adjacentEdges.clone()));
		LinkedList<T> ret = new LinkedList<T>();
		for (SpatialOrganizationEdge<T> e : this.adjacentEdges) {
			SpatialOrganizationNode<T> opp = e.getOpposite(this);
			if (opp != null) ret.add(opp.getUserObject());
		}
		return ret;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getVerticesOfTheTetrahedronContaining(double[])
	 */
	public Object[] getVerticesOfTheTetrahedronContaining(double[] position) {
		if (adjacentTetrahedra.isEmpty()) return null;
		// throw new RuntimeException("The point "+this+
		// " is not adjacent to any tetrahedra! Therefore, this point cannot
		// find a tetrahedron containing the position ("+
		// position[0]+"/"+position[1]+"/"+position[2]+")!");
		Tetrahedron<T> insertionTetrahedron = adjacentTetrahedra.getFirst();
		if (insertionTetrahedron.isInfinite())
			insertionTetrahedron =
					insertionTetrahedron.getOppositeTriangle(null)
							.getOppositeTetrahedron(insertionTetrahedron);
		Tetrahedron<T> last = null;
		while ((insertionTetrahedron != last)
			&& (!insertionTetrahedron.isInfinite())) {
			last = insertionTetrahedron;
			try {
				insertionTetrahedron =
						insertionTetrahedron.walkToPoint(position);
			} catch (PositionNotAllowedException e) {
				insertionTetrahedron = last;
			}
		}
		if (insertionTetrahedron.isInfinite()) return null;
		Object[] ret = new Object[4];
		SpaceNode<T>[] nodes = insertionTetrahedron.getAdjacentNodes();
		for (int i = 0; i < nodes.length; i++) {
			if (nodes[i] != null) ret[i] = nodes[i].getUserObject();
		}
		return ret;
	}

	/**
	 * Modifies the volume associated with this SpaceNode by a given value.
	 * 
	 * @param change
	 *            The change value that will be added to the volume.
	 */
	protected void changeVolume(double change) {
		this.volume += change;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getVolume()
	 */
	public double getVolume() {
		return this.volume;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getNewInstance(double[],
	 *      java.lang.Object)
	 */
	public SpatialOrganizationNode<T> getNewInstance(double[] position,
			T userObject) throws PositionNotAllowedException {

		// create a new SpaceNode:
		SpaceNode<T> insertPoint = new SpaceNode<T>(position, userObject);

		// the new instance should have the same listeners!
		insertPoint.setListenerList(this.listeners);

		// check if this point is capable of inserting a new point:
		if (adjacentTetrahedra.isEmpty()) {
			// enough nodes collected:
			if (adjacentEdges.size() == 2) {
				// collect the nodes:
				SpaceNode<T> a =
						(SpaceNode<T>) adjacentEdges.getFirst().getOpposite(
								this), b =
						(SpaceNode<T>) adjacentEdges.getLast()
								.getOpposite(this);
				// clear the edge lists:
				adjacentEdges.clear();

				a.getAdjacentEdges().clear();
				b.getAdjacentEdges().clear();
				// now create the first tetrahedron:
				Tetrahedron.createInitialTetrahedron(this, insertPoint, a, b);
			}
			else {
				new Edge<T>(this, insertPoint);
				if (adjacentEdges.size() == 2)
					new Edge<T>((SpaceNode<T>) adjacentEdges.getLast()
							.getOpposite(this), insertPoint);
			}

		}
		else
		// insert point:
		insertPoint.insert(adjacentTetrahedra.getFirst());
		return insertPoint;
	}

	/**
	 * Sets the list of movement listeners attached to this node to a specified
	 * list.
	 * 
	 * @param listeners
	 *            The movement listeners that are listening to this node's
	 *            movements.
	 */
	public void setListenerList(
			LinkedList<SpatialOrganizationNodeMovementListener<T>> listeners) {
		this.listeners = listeners;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#addSpatialOrganizationNodeMovementListener(ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener)
	 */
	public void addSpatialOrganizationNodeMovementListener(
			SpatialOrganizationNodeMovementListener<T> listener) {
		if (listeners == null)
			listeners =
					new LinkedList<SpatialOrganizationNodeMovementListener<T>>();
		listeners.addLast(listener);
	}

	/**
	 * @return The list of tetrahedra incident to this node.
	 */
	public LinkedList<Tetrahedron<T>> getAdjacentTetrahedra() {
		return adjacentTetrahedra;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getPosition()
	 */
	public double[] getPosition() {
		return position;
	}

	/**
	 * @return The identification number of this SpaceNode.
	 */
	public int getId() {
		return id;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#remove()
	 */
	public void remove() {
		removeAndReturnCreatedTetrahedron();
	}

	/**
	 * Removes this SpaceNode from the triangulation and restores the gap in the
	 * triangulation by filling it with new triangles.
	 * 
	 * @return A new tetrahedron that was created while filling the created gap.
	 * @see OpenTriangleOrganizer#triangulate()
	 */
	private Tetrahedron<T> removeAndReturnCreatedTetrahedron() {
		if (listeners != null) {
			for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
				listener.nodeAboutToBeRemoved(this);
		}
		OpenTriangleOrganizer<T> oto =
				OpenTriangleOrganizer.createSimpleOpenTriangleOrganizer();
		LinkedList<Tetrahedron<T>> messedUpTetrahedra = null;
		// Collect the triangles that are opened by removing the point and
		// remove the corresponding tetrahedrons:
		for (Tetrahedron<T> tetrahedron : (LinkedList<Tetrahedron<T>>) adjacentTetrahedra
				.clone()) {
			if (tetrahedron.isValid()) {
				Triangle3D<T> oppositeTriangle =
						tetrahedron.getOppositeTriangle(this);
				oto.putTriangle(oppositeTriangle);
				Tetrahedron<T> oppositeTetrahedron =
						oppositeTriangle.getOppositeTetrahedron(tetrahedron);
				tetrahedron.remove();
				if ((oppositeTetrahedron != null)
					&& !oppositeTetrahedron.isInfinite()
					&& (oppositeTetrahedron.isInsideSphere(getPosition()))) {
					if (messedUpTetrahedra == null)
						messedUpTetrahedra = new LinkedList<Tetrahedron<T>>();
					messedUpTetrahedra.add(oppositeTetrahedron);
				}
			}
		}
		if (messedUpTetrahedra != null) {
			for (Tetrahedron<T> tetrahedron : messedUpTetrahedra) {
				if (tetrahedron.isValid())
					oto.removeAllTetrahedraInSphere(tetrahedron);
			}
		}
		oto.triangulate();
		if (allNodes != null) 
			allNodes.remove(this);
		if (listeners != null) {
			for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
				listener.nodeRemoved(this);
		}
		return oto.getANewTetrahedron();
	}

	/**
	 * Starting at a given tetrahedron, this function searches the triangulation
	 * for a tetrahedron which contains this node's coordinate.
	 * 
	 * @param start
	 *            The starting tetrahedron.
	 * @return A tetrahedron which contains the position of this node.
	 * @throws PositionNotAllowedException
	 */
	public Tetrahedron<T> searchInitialInsertionTetrahedron(Tetrahedron<T> start)
			throws PositionNotAllowedException {
		return searchInitialInsertionTetrahedron(start, this.getPosition());
	}

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
	public static <T> Tetrahedron<T> searchInitialInsertionTetrahedron(
			Tetrahedron<T> start, double[] coordinate)
			throws PositionNotAllowedException {
		Tetrahedron<T> current = start;
		if (current.isInfinite())
			current =
					current.getOppositeTriangle(null).getOppositeTetrahedron(
							current);
		Tetrahedron<T> last = null;
		while ((current != last) && (!current.isInfinite())) {
			last = current;
			current = current.walkToPoint(coordinate);
		}
		return current;
	}

	/**
	 * A private function used inside {@link #insert(Tetrahedron)} to remove a given tetrahedron, 
	 * inform an open triangle organizer about a set of new open triangles and add all tetrahedrons
	 * adjacent to the removed tetrahedron to a queue.
	 * @param tetrahedron The tetrahedron that should be removed.
	 * @param queue The queue which is used to keep track of candidates that might have to be removed.
	 * @param oto The open triangle organizer that keeps track of all open triangles.
	 */
	private void processTetrahedron(Tetrahedron<T> tetrahedron,
			LinkedList<Triangle3D<T>> queue, OpenTriangleOrganizer<T> oto) {
		tetrahedron.remove();
		for (int i = 0; i < 4; i++) {
			Triangle3D<T> currentTriangle =
					tetrahedron.getAdjacentTriangles()[i];
			if (currentTriangle.isCompletelyOpen())
				oto.removeTriangle(currentTriangle);
			else {
				queue.offer(currentTriangle);
				oto.putTriangle(currentTriangle);
			}
		}
	}

	// private Tetrahedron insertInOuterSpace(Tetrahedron start,
	// OpenTriangleOrganizer oto, LinkedList<Triangle3D>
	// convexHullTriangleQueue, LinkedList<Triangle3D> outerTriangles) {
	//		
	// LinkedList<Triangle3D> queue = new LinkedList<Triangle3D>();
	// if (start.isTrulyInsideSphere(this.position))
	// processTetrahedron(start, queue, oto);
	// // processTetrahedron(insertionStart, queue, oto);
	// while (!queue.isEmpty()) {
	// Triangle3D currentTriangle = queue.poll();
	// Tetrahedron oppositeTetrahedron =
	// currentTriangle.getOppositeTetrahedron(null);
	// if (oppositeTetrahedron != null) {
	// if (!oppositeTetrahedron.isInfinite())
	// convexHullTriangleQueue.offer(currentTriangle);
	// else {
	// if (oppositeTetrahedron.isTrulyInsideSphere(this.position))
	// processTetrahedron(oppositeTetrahedron, queue, oto);
	// else
	// outerTriangles.offer(currentTriangle);
	// }
	// }
	// }
	//		
	// Tetrahedron ret = null;
	// return ret;
	// }

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
	public Tetrahedron<T> insert(Tetrahedron<T> start)
			throws PositionNotAllowedException {
		Tetrahedron<T> insertionStart =
				searchInitialInsertionTetrahedron(start);

		if (listeners != null) {
			// tell the listeners that there will be a new node:
			Object[] verticeContents = insertionStart.getVerticeContents();
			for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
				listener.nodeAboutToBeAdded(this, position, verticeContents);
		}

		OpenTriangleOrganizer<T> oto =
				OpenTriangleOrganizer.createSimpleOpenTriangleOrganizer();
		LinkedList<Triangle3D<T>> queue = new LinkedList<Triangle3D<T>>();
		LinkedList<Triangle3D<T>> outerTriangles =
				new LinkedList<Triangle3D<T>>();

		processTetrahedron(insertionStart, queue, oto);
		// if (insertionStart.isInfinite() &&
		// insertionStart.getAdjacentTetrahedron(0).isInfinite()) {
		//			
		// }
		// else
		// {
		while (!queue.isEmpty()) {
			Triangle3D<T> currentTriangle = queue.poll();
			Tetrahedron<T> oppositeTetrahedron =
					currentTriangle.getOppositeTetrahedron(null);
			if ((oppositeTetrahedron != null)) {
				if (oppositeTetrahedron.isTrulyInsideSphere(this.position))
					processTetrahedron(oppositeTetrahedron, queue, oto);
				else outerTriangles.offer(currentTriangle);
			}
		}
		Tetrahedron<T> ret = null;
		// create a star-shaped triangulation:
		for (Triangle3D<T> currentTriangle : outerTriangles) {
			if (!currentTriangle.isCompletelyOpen())
				ret = new Tetrahedron<T>(currentTriangle, this, oto);
		}
		// }
		// tell the listeners that the node was added:
		if (listeners != null) {
			for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
				listener.nodeAdded(this);
		}

		return ret;
	}

	/**
	 * Determines if the current triangulation would still be valid when this node would be moved to a 
	 * given coordinate. (This function does not check whether the Delaunay criterion would still be 
	 * fullfilled.)
	 * @param newPosition The new coordinate to which this point should be moved to.
	 * @return <code>true</code>, if the triangulation would not be corrupted by moving this node to 
	 * the specified position and <code>false</code>, if not.
	 * @throws PositionNotAllowedException
	 */
	private boolean checkIfTriangulationIsStillValid(double[] newPosition)
			throws PositionNotAllowedException {
		for (Tetrahedron<T> tetrahedron : adjacentTetrahedra) {
			if (tetrahedron.isFlat()) return false;
			if (tetrahedron.isInfinite()) {
				Tetrahedron<T> innerTet = tetrahedron.getAdjacentTetrahedron(0);
				if (innerTet.getAdjacentTetrahedron(0).isInfinite()
					&& innerTet.getAdjacentTetrahedron(1).isInfinite()
					&& innerTet.getAdjacentTetrahedron(2).isInfinite()
					&& innerTet.getAdjacentTetrahedron(3).isInfinite())
					return true;
				else return false;
			}
			else {
				// check for each adjacent triangle if this node would remain on the same side:
				Triangle3D<T> triangle = tetrahedron.getOppositeTriangle(this);
				// if (! triangle.isInfinite()) {
				triangle.updatePlaneEquationIfNecessary();
				if (!triangle.trulyOnSameSide(this.getPosition(), newPosition)) {
					tetrahedron.testPosition(newPosition);
					return false;
				}
			}
		}
		return true;
	}

	/**
	 * Creates an unique identifier that is used while restoring the Delaunay property.
	 * While running the flip algorithm, each triangle has to be tested whether it is still 
	 * locally Delaunay. In order to make sure that no triangle is tested more than once, 
	 * tested triangles are tagged with a checking index. 
	 * @return A unique index.
	 */
	private static int createNewCheckingIndex() {
		checkingIndex = (checkingIndex + 1) % 2000000000;
		return checkingIndex;
	}

	// private void recurseForMessedUpTetrahedra(
	// Tetrahedron<T> startingTetrahedron,
	// LinkedList<Tetrahedron<T>> messedUpTetrahedra,
	// SpaceNode<T> problemNode, SpaceNode<T> lastOppositeNode,
	// int checkingIndex) {
	// // System.out.print("Testing tetrahedron "+startingTetrahedron+"...");
	// if (((startingTetrahedron.isAdjacentTo(problemNode)) && (lastOppositeNode
	// != null) &&
	// (startingTetrahedron.isInsideSphere(lastOppositeNode.getPosition()))) ||
	// ((!startingTetrahedron.isAdjacentTo(problemNode)) &&
	// (startingTetrahedron.isInsideSphere(problemNode.getPosition())))) {
	// // if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("node is
	// inside!");
	// messedUpTetrahedra.add(startingTetrahedron);
	// for (int i = 0; i < 4; i++) {
	// Triangle3D<T> triangle = startingTetrahedron.getAdjacentTriangles()[i];
	// if (!triangle.wasCheckedAlready(checkingIndex)) {
	// Tetrahedron<T> oppositeTetrahedron =
	// triangle.getOppositeTetrahedron(startingTetrahedron);
	// recurseForMessedUpTetrahedra(
	// oppositeTetrahedron,
	// messedUpTetrahedra,
	// problemNode,
	// startingTetrahedron .getAdjacentNodes()[i],
	// checkingIndex);
	// }
	// }
	// }
	// // else if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("node is
	// outside");
	// }

	/**
	 * Removes a tetrahedron from the triangulation and adds all adjacent tetrahedra to a linked list.
	 * This function is used when the flip algorithm did not succeed in restoring the Delaunay property.
	 * It removes a tetrahedron, adds all adjacent tetrahedra to a list and adds all incident nodes to a list.
	 * Whenever open triangles are created, an open triangle organizer is informed.
	 * @param tetrahedronToRemove The tetrahedron that should be removed.
	 * @param list The list of candidate tetrahedra which might have to be deleted.
	 * @param nodeList A list of nodes that keeps track of all nodes which were incident to 
	 * removed tetrahedra during a run of {@link #cleanUp(LinkedList)}.
	 * @param oto An open triangle organizer which is used to keep track of open triangles.
	 * @return <code>true</code>, if any of the two lists <code>list</code> of <code>nodeList</code> 
	 * were modified during this function call.
	 */
	private boolean removeTetrahedronDuringCleanUp(
			Tetrahedron<T> tetrahedronToRemove,
			LinkedList<Tetrahedron<T>> list, LinkedList<SpaceNode<T>> nodeList,
			OpenTriangleOrganizer<T> oto) {
		boolean ret = false;
		for (SpaceNode<T> node : tetrahedronToRemove.getAdjacentNodes()) {
			if ((node != null) && (!nodeList.contains(node))) {
				ret = true;
				nodeList.add(node);
			}
		}
		for (Triangle3D<T> adjacentTriangle : tetrahedronToRemove
				.getAdjacentTriangles()) {
			Tetrahedron<T> opposite =
					adjacentTriangle
							.getOppositeTetrahedron(tetrahedronToRemove);
			if ((opposite != null) && (!list.contains(opposite))) {
				list.add(opposite);
				// opposite.remove();
				ret = true;
			}
		}
		tetrahedronToRemove.remove();
		for (Triangle3D<T> currentTriangle : tetrahedronToRemove
				.getAdjacentTriangles()) {
			if (currentTriangle.isCompletelyOpen()) {
				// if (NewDelaunayTest.createOutput())
				// NewDelaunayTest.out("removing "+currentTriangle+" from the
				// open triangle list");
				oto.removeTriangle(currentTriangle);
			}
			else {
				// if (NewDelaunayTest.createOutput())
				// NewDelaunayTest.out("adding "+currentTriangle+" to the open
				// triangle list");
				oto.putTriangle(currentTriangle);
			}
		}
		return ret;
	}

	/**
	 * Restores the Delaunay criterion for  a section of the triangulation which 
	 * cannot be restored using a flip algorithm.
	 * Whenever the flip algorithm fails to restore the Delaunay criterion, this function is called.
	 * First, all tetrahedra that cause a problem are removed along with all adjacent tetrahedra that
	 * have the same circumsphere. Then, the resulting hole in the triangulation is filled using
	 * {@link OpenTriangleOrganizer#triangulate()}.
	 * @param messedUpTetrahedra A set of tetrahedra that are not Delaunay and cannot be replaced using a 
	 * flip algorithm.
	 */
	private void cleanUp(LinkedList<Tetrahedron<T>> messedUpTetrahedra) {
		LinkedList<Tetrahedron<T>> outerTetrahedra =
				new LinkedList<Tetrahedron<T>>();
		LinkedList<SpaceNode<T>> problemNodes = new LinkedList<SpaceNode<T>>();
		OpenTriangleOrganizer<T> oto =
				OpenTriangleOrganizer.createSimpleOpenTriangleOrganizer();
		if (NewDelaunayTest.createOutput())
			NewDelaunayTest.out("Cleaning up messed up tetrahedra: "
				+ messedUpTetrahedra.toString());
		for (Tetrahedron<T> tetrahedron : messedUpTetrahedra) {
			if (tetrahedron.isValid()) {
				removeTetrahedronDuringCleanUp(tetrahedron, outerTetrahedra,
						problemNodes, oto);
				if (outerTetrahedra.contains(tetrahedron))
					outerTetrahedra.remove(tetrahedron);
			}
		}
		boolean done = false;
		while (!done) {
			Tetrahedron<T> problemTetrahedron = null;
			for (Tetrahedron<T> outerTetrahedron : outerTetrahedra) {
				if (outerTetrahedron.isValid())
					for (SpaceNode<T> node : problemNodes) {
						if (!outerTetrahedron.isAdjacentTo(node)) {
							if (outerTetrahedron.isFlat()
								|| outerTetrahedron.isInsideSphere(node
										.getPosition())) {
								removeTetrahedronDuringCleanUp(
										outerTetrahedron, outerTetrahedra,
										problemNodes, oto);
								problemTetrahedron = outerTetrahedron;
								break;
							}
						}
					}
				if (problemTetrahedron != null) break;
			}
			if (problemTetrahedron != null)
				outerTetrahedra.remove(problemTetrahedron);
			else done = true;
		}
		oto.triangulate();
	}

	/**
	 * Restores the Delaunay property for the current triangulation after a movement of this node.
	 * If the triangulation remains a valid triangulation after a node movement, a sequence of
	 * 2->3 flips and 3->2 flips can often times restore the Delaunay property for the triangulation.
	 * This function first applies such a flip algorithm starting at all tetrahedra incident to this node.
	 * If the Delaunay property cannot be restored using this flip algorithm, a cleanup procedure is used,
	 * which removes all tetrahedra that cause a problem and then re-triangulates the resulted hole.
	 */
	public void restoreDelaunay() {
		LinkedList<Tetrahedron<T>> activeTetrahedra =
				new LinkedList<Tetrahedron<T>>();
		for (Tetrahedron<T> tetrahedron : getAdjacentTetrahedra()) {
			tetrahedron.updateCirumSphereAfterNodeMovement(this);
			activeTetrahedra.add(tetrahedron);

		}

		while (activeTetrahedra != null && !activeTetrahedra.isEmpty()) {
			int checkingIndex = createNewCheckingIndex();
			LinkedList<Tetrahedron<T>> problemTetrahedra =
					new LinkedList<Tetrahedron<T>>();
			LinkedList<Tetrahedron<T>> flatTetrahedra =
					new LinkedList<Tetrahedron<T>>();
			while (!activeTetrahedra.isEmpty()) {
				Tetrahedron<T> tetrahedron = activeTetrahedra.poll();
				if (tetrahedron.isValid()) {
					// if (tetrahedron.isInfinite()) {
					// if (NewDelaunayTest.createOutput())
					// NewDelaunayTest.out("Stop");
					// }
					for (int i = (tetrahedron.isInfinite() ? 1 : 0); i < 4; i++) {
						Triangle3D<T> triangleI =
								tetrahedron.getAdjacentTriangles()[i];
						// Check whether or not we already tested this
						// combination:
						if (!triangleI.wasCheckedAlready(checkingIndex)) {
							Tetrahedron<T> tetrahedronI =
									triangleI
											.getOppositeTetrahedron(tetrahedron);
							SpaceNode<T> nodeI =
									tetrahedronI.getOppositeNode(triangleI);
							// is there a violation of the Delaunay criterion?
							if ((nodeI != null)
								&& ((tetrahedron.isTrulyInsideSphere(nodeI
										.getPosition()) || (tetrahedron
										.isFlat() && tetrahedronI.isFlat())))) {
								Tetrahedron<T>[] newTetrahedra = null;
								// check if there is a neighboring tetrahedron
								// also violating the Delaunay criterion
								for (int j = (tetrahedron.isInfinite() ? 1 : 0); j < 4; j++) {
									if (i != j) {
										Triangle3D<T> triangleJ =
												tetrahedron
														.getAdjacentTriangles()[j];
										Tetrahedron<T> tetrahedronJ =
												triangleJ
														.getOppositeTetrahedron(tetrahedron);
										// is there also a violation of the
										// Delaunay
										// criterion between tetrahedronI and
										// tetrahedronJ?
										if (tetrahedronJ == null)
											if (NewDelaunayTest.createOutput())
												NewDelaunayTest
														.out("restoreDelaunay");
										if (tetrahedronJ
												.isNeighbor(tetrahedronI)) {
											SpaceNode<T> oppJ =
													tetrahedron
															.getAdjacentNodes()[j];
											SpaceNode<T> oppI =
													tetrahedron
															.getAdjacentNodes()[i];
											if (oppI != null && oppJ != null) {
												// Either all 3 tetrahedra are
												// flat & they are all neighbors
												// of each other,
												// or they are not flat but
												// their spheres include the
												// other tetrahedra's points
												if ((tetrahedron.isFlat()
													&& tetrahedronI.isFlat()
													&& tetrahedronJ.isFlat() && tetrahedronI != tetrahedronJ)
													|| (tetrahedronJ
															.isTrulyInsideSphere(oppJ
																	.getPosition()) && tetrahedronI
															.isTrulyInsideSphere(oppI
																	.getPosition()))) {
													newTetrahedra =
															Tetrahedron
																	.flip3to2(
																			tetrahedron,
																			tetrahedronI,
																			tetrahedronJ);
													break;
												}
												//											
												// if (tetrahedron.is)
												// if (tetrahedron.isFlat() ||
												// tetrahedronI.isFlat() ||
												// tetrahedronJ.isFlat()) {
												// if (tetrahedron )
												// }
												// if
												// (tetrahedronJ.isInsideSphere(oppJ.getPosition())
												// &&
												// tetrahedronI.isInsideSphere(oppI.getPosition()))
												// {
												// newTetrahedra =
												// Tetrahedron.flip3to2(tetrahedron,
												// tetrahedronI, tetrahedronJ);
												// }
												// else if {
												// newTetrahedra =
												// Tetrahedron.flip3to2(tetrahedron,
												// tetrahedronI, tetrahedronJ);
												// break;
												// }
											}
										}
									}
								} // for j
								// if no 3->2 flip was found, perform a 2->3
								// flip, if possible (convex!)
								if (newTetrahedra == null) {
									if (tetrahedron.isFlat()
										&& tetrahedronI.isFlat()
										&& tetrahedron.isAdjacentTo(nodeI))
										newTetrahedra =
												Tetrahedron
														.remove2FlatTetrahedra(
																tetrahedron,
																tetrahedronI);
									else if (!(tetrahedron.isFlat() || tetrahedronI
											.isFlat()))
										newTetrahedra =
												Tetrahedron.flip2to3(
														tetrahedron,
														tetrahedronI);
									// else
									// newTetrahedra =
									// Tetrahedron.flip2to3(tetrahedron,
									// tetrahedronI);
								}
								if (newTetrahedra != null) {
									for (Tetrahedron<T> tet : newTetrahedra) {
										activeTetrahedra.add(tet);
										if (tet.isFlat()) {
											flatTetrahedra.add(tet);
										}
									}
									break;
								}
								else {
									if (NewDelaunayTest.createOutput())
										NewDelaunayTest.out("Tetrahedrons "
											+ tetrahedron + " and "
											+ tetrahedronI
											+ " are messed up because of node "
											+ nodeI);
									problemTetrahedra.add(tetrahedron);
									problemTetrahedra.add(tetrahedronI);
									// LinkedList<Tetrahedron> list =
									// messedUpNodes.get(nodeI);
									// if (list == null) {
									// list = new LinkedList<Tetrahedron>();
									// messedUpNodes.put(nodeI, list);
									// }
									// if (!list.contains(tetrahedron))
									// list.add(tetrahedron);
									// list =
									// messedUpNodes.get(tetrahedron.getAdjacentNodes()[i]);
									// if (list == null) {
									// list = new LinkedList<Tetrahedron>();
									// messedUpNodes.put(tetrahedron.getAdjacentNodes()[i],
									// list);
									// }
									// if (!list.contains(tetrahedronI))
									// list.add(tetrahedronI);
									activeTetrahedra.add(tetrahedronI);
								}
							} // if nodeI inside sphere of tetrahedron
						} // if was checked before
					} // for i
					// if (messedUp && tetrahedron.isValid())
					// messedUpTetrahedra.offer(tetrahedron);
				} // if tetrahedron is valid
			} // while

			// special case: in some situation (like an octahedron), some false
			// tetrahedra might not have been removed. We solve this problem by
			// simply removing all invalid tetrahedra and triangulating the
			// holes
			LinkedList<Tetrahedron<T>> messedUpTetrahedra =
					new LinkedList<Tetrahedron<T>>();
			// check if there are flat tetrahedra left:
			for (Tetrahedron<T> flatTetrahedron : flatTetrahedra) {
				if (flatTetrahedron.isValid()
					&& !messedUpTetrahedra.contains(flatTetrahedron)) {
					for (Triangle3D<T> triangle : flatTetrahedron
							.getAdjacentTriangles()) {
						Tetrahedron<T> oppositeTetrahedron =
								triangle
										.getOppositeTetrahedron(flatTetrahedron);
						if (oppositeTetrahedron.isValid()
							&& !messedUpTetrahedra
									.contains(oppositeTetrahedron))
							messedUpTetrahedra.add(oppositeTetrahedron);
					}
					messedUpTetrahedra.add(flatTetrahedron);

				}
			}
			// filter for messedUpTetrahedra, that are still valid and still
			// messed up:
			for (Tetrahedron<T> tetrahedron : problemTetrahedra) {
				if (tetrahedron.isValid() && !tetrahedron.isFlat()
					&& !messedUpTetrahedra.contains(tetrahedron)) {
					for (Triangle3D<T> adjacentTriangle : tetrahedron
							.getAdjacentTriangles()) {
						Tetrahedron<T> oppositeTetrahedron =
								adjacentTriangle
										.getOppositeTetrahedron(tetrahedron);
						if (!oppositeTetrahedron.isInfinite()) {
							SpaceNode<T> oppositeNode =
									oppositeTetrahedron
											.getOppositeNode(adjacentTriangle);
							if (tetrahedron.isTrulyInsideSphere(oppositeNode
									.getPosition())) {
								messedUpTetrahedra.add(tetrahedron);
								break;
							}
						}
					}
				}
			}
			if (!messedUpTetrahedra.isEmpty()) cleanUp(messedUpTetrahedra);
			// // take care that all flat tetrahedra are being removed:
			// for (Tetrahedron flatTetrahedron : flatTetrahedra) {
			// if (flatTetrahedron.isValid() &&
			// !messedUpTetrahedra.contains(flatTetrahedron)) {
			// checkingIndex = createNewCheckingIndex();
			// messedUpTetrahedra.add(flatTetrahedron);
			// for (Triangle3D triangle :
			// flatTetrahedron.getAdjacentTriangles()) {
			// Tetrahedron tetrahedron =
			// triangle.getOppositeTetrahedron(flatTetrahedron);
			// for (Triangle3D triangle2 :
			// flatTetrahedron.getAdjacentTriangles()) {
			// Tetrahedron tetrahedron2 =
			// triangle2.getOppositeTetrahedron(flatTetrahedron);
			// if (tetrahedron.isNeighbor(tetrahedron2)) {
			// int connectionNumber =
			// tetrahedron2.getConnectingTriangleNumber(tetrahedron);
			// if (!messedUpTetrahedra.contains(tetrahedron))
			// recurseForMessedUpTetrahedra(tetrahedron, messedUpTetrahedra,
			// tetrahedron2.getAdjacentNodes()[connectionNumber],
			// tetrahedron2.getAdjacentNodes()[connectionNumber],
			// checkingIndex);
			// }
			//								
			// }
			// problemTetrahedra.add(triangle.getOppositeTetrahedron(flatTetrahedron));
			// }
			// if (NewDelaunayTest.createOutput())
			// NewDelaunayTest.out("restoreDelaunay: flat tetrahedron left
			// over!");
			//					
			// }
			// }
			//			
			// for (Tetrahedron tetrahedron : problemTetrahedra) {
			// if (tetrahedron.isValid() && !tetrahedron.isFlat()) {
			// checkingIndex = createNewCheckingIndex();
			// for (Triangle3D adjacentTriangle :
			// tetrahedron.getAdjacentTriangles()) {
			// Tetrahedron oppositeTetrahedron =
			// adjacentTriangle.getOppositeTetrahedron(tetrahedron);
			// if (!oppositeTetrahedron.isInfinite()) {
			// SpaceNode oppositeNode =
			// oppositeTetrahedron.getOppositeNode(adjacentTriangle);
			// if (tetrahedron.isTrulyInsideSphere(oppositeNode.getPosition()))
			// {
			// if (!messedUpTetrahedra.contains(tetrahedron))
			// recurseForMessedUpTetrahedra(tetrahedron, messedUpTetrahedra,
			// oppositeNode, oppositeNode, checkingIndex);
			// }
			// }
			// }
			// }
			// }
			//
			//			
			// OpenTriangleOrganizer oto = null;
			// for (Tetrahedron messedUpTetrahedron : messedUpTetrahedra) {
			// if (messedUpTetrahedron.isValid()) {
			//					
			// if (oto == null)
			// oto = OpenTriangleOrganizer.createSimpleOpenTriangleOrganizer();
			// messedUpTetrahedron.remove();
			// for (int i = 0; i < 4; i++) {
			// Triangle3D currentTriangle =
			// messedUpTetrahedron.getAdjacentTriangles()[i];
			// if (currentTriangle.isCompletelyOpen())
			// oto.removeTriangle(currentTriangle);
			// else {
			// oto.putTriangle(currentTriangle);
			// activeTetrahedra.add(currentTriangle.getOppositeTetrahedron(null));
			// }
			// }
			// }
			// }
			// if (oto != null) {
			// try {
			// oto.recoredNewTetrahedra();
			// oto.triangulate();
			// activeTetrahedra = oto.getNewTetrahedra();
			// }
			// catch (RuntimeException e) {
			// if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("\nMessed
			// up tetrahedra:");
			// for (Tetrahedron tet : messedUpTetrahedra) {
			// if (NewDelaunayTest.createOutput()) NewDelaunayTest.out(tet+"");
			// }
			// if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("\nOpen
			// triangles:");
			// for (Triangle3D triangle : oto.getOpenTriangles()) {
			// if (NewDelaunayTest.createOutput())
			// NewDelaunayTest.out(triangle+"");
			// }
			// throw e;
			// }
			// }
		}
	}

	/* (non-Javadoc)
	 * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#moveFrom(double[])
	 */
	public void moveFrom(double[] delta) throws PositionNotAllowedException {
		moveTo(add(this.position, delta));
	}

	/**
	 * Moves this node to a new position.
	 * @param newPosition The new coordinate for this node.
	 * @throws PositionNotAllowedException
	 */
	public void moveTo(double[] newPosition) throws PositionNotAllowedException {

		if (checkIfTriangulationIsStillValid(newPosition)) {
			if (listeners != null) {
				double[] delta = subtract(newPosition, this.position);
				for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
					listener.nodeAboutToMove(this, delta);
			}
			flipMovements++;
			this.position = newPosition;
			restoreDelaunay();
			if (listeners != null) {
				for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
					listener.nodeMoved(this);
			}
		}
		else {
			if (NewDelaunayTest.createOutput())
				NewDelaunayTest.out("Node must be deleted and reinserted!");
			deleteAndInsertMovements++;
			Tetrahedron<T> insertPosition =
					searchInitialInsertionTetrahedron(adjacentTetrahedra
							.getFirst(), newPosition);
			Tetrahedron<T> aNewTetrahedron =
					removeAndReturnCreatedTetrahedron();
			if (!insertPosition.isValid()) insertPosition = aNewTetrahedron;
			double[] oldPosition = position;
			this.position = newPosition;
			try {
				insert(insertPosition);
			} catch (PositionNotAllowedException e) {
				this.position = oldPosition;
				insert(insertPosition);
				throw e;
			}
			if (allNodes != null) 
				allNodes.add(this);
		}
	}

	/**
	 * Proposes a new position for a node that was moved to the same coordinate as this node.
	 * @return A coordinate which can be used to place the problematic node.
	 */
	public double[] proposeNewPosition() {
		double minDistance = Double.MAX_VALUE;
		double[] farthestAwayDiff = null;
		double maxDistance = Double.MIN_VALUE;
		for (SpatialOrganizationEdge<T> edge : this.adjacentEdges) {
			SpatialOrganizationNode<T> otherNode = edge.getOpposite(this);
			if (otherNode != null) {
				double[] diff = subtract(otherNode.getPosition(), position);
				double distance = dot(diff, diff);
				if (distance < minDistance) {
					minDistance = distance;
				}
				if (distance > maxDistance) {
					maxDistance = distance;
					farthestAwayDiff = diff;
				}
			}
			else if (maxDistance < Double.MAX_VALUE) {
				maxDistance = Double.MAX_VALUE;
				Tetrahedron<T> someAdjacentTetrahedron =
						(Tetrahedron<T>) ((Edge) edge).getAdjacentTetrahedra()
								.getFirst();
				if (someAdjacentTetrahedron == null) {
					if (NewDelaunayTest.createOutput())
						NewDelaunayTest.out("proposeNewPosition");
				}
				Triangle3D<T> triangle =
						someAdjacentTetrahedron.getAdjacentTriangles()[0];
				triangle.updatePlaneEquationIfNecessary();
				Tetrahedron<T> oppositeTetrahedron =
						triangle
								.getOppositeTetrahedron(someAdjacentTetrahedron);
				farthestAwayDiff = triangle.getNormalVector();
				if (!oppositeTetrahedron.isInfinite()) {
					double[] outerPosition =
							add(this.getPosition(), farthestAwayDiff);
					if (triangle.onSameSide(outerPosition, oppositeTetrahedron
							.getOppositeNode(triangle).getPosition()))
						farthestAwayDiff = scalarMult(-1, farthestAwayDiff);
				}
			}

		}
		return add(position, scalarMult(Math.sqrt(minDistance) * 0.5,
				normalize(farthestAwayDiff)));
	}

	/**
	 * Returns a list of all edges that are incident to this node.
	 * @return A list of edges.
	 */
	public LinkedList<SpatialOrganizationEdge<T>> getAdjacentEdges() {
		return adjacentEdges;
	}

}
