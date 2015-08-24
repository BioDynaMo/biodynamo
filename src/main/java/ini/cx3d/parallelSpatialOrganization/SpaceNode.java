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

package ini.cx3d.parallelSpatialOrganization;

import ini.cx3d.spatialOrganization.PositionNotAllowedException;
import ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.logging.Level;
import java.util.logging.Logger;

import static ini.cx3d.utilities.Matrix.*;

/**
 * This class is used to represent a node of a triangulation. Each node is
 * stores information about incident tetrahedra and edges (arbitrary amounts).  
 * 
 * @author Dennis Goehlsdorf
 * 
 * @param <T> The type of the user objects associated with each node.
 * TODO ensure this class is fully serializable
 */
public class SpaceNode<T> extends ManagedObject<T> {
	private static final long serialVersionUID = 4243833167511730757L;
	
	//Get a logger
	private static Logger theLogger = Logger.getLogger(SpaceNode.class.getName());	
	///////////////////////////////////////////////////////////////////
	/// Replace with config. file at some point.....
	///////////////////////////////////////////////////////////////////	
	{
		//theLogger.setLevel(Level.FINER);
		theLogger.setLevel(Level.FINEST);
	}
	///////////////////////////////////////////////////////////////////	
	
	
	/**
	 * A static list of all nodes that are part of the current triangulation. DO
	 * NOT USE! Exclusively used for debugging purposes. Needs to be removed.
	 */
	//public static LinkedList<SpaceNode> allNodes = new LinkedList<SpaceNode>();
	 public static LinkedList<SpaceNode> allNodes = null;

	/**
	 * Number of node movements performed during the current simulation which
	 * were processed by a flipping algorithm. Only for debugging purposes.
	 * Needs to be removed.
	 */
	public static int flipMovements = 0;

	/**
	 * Number of node movements performed during the current simulation where a
	 * delete & insert algorithm had to be applied. Only for debugging purposes.
	 * Needs to be removed.
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
	private LinkedList<ManagedObjectReference<T>> adjacentEdges = new LinkedList<ManagedObjectReference<T>>();
//	private LinkedList<SpatialOrganizationEdge<T>> adjacentEdges = new LinkedList<SpatialOrganizationEdge<T>>();

	/**
	 * A list of all tetrahedra incident to this node.
	 */
	private LinkedList<ManagedObjectReference<T>> adjacentTetrahedra = new LinkedList<ManagedObjectReference<T>>();
//	private LinkedList<Tetrahedron<T>> adjacentTetrahedra = new LinkedList<Tetrahedron<T>>();

	/**
	 * The volume associated with this SpaceNode.
	 */
	private double volume = 0;
	
	
//	/**
//	 * The unique address of this ManagedObject
//	 */
//	protected long address = 0;
//	
//	/**
//	 * The active objects that owns this ManagedObject
//	 */
//	protected SpatialOrganizationManager<T> som= null;
	
	/**
	 * Clears the list of all nodes and sets the static node counter to zero.
	 */
	public static void clear() {
		if (allNodes != null)
			allNodes.clear();
		IDCOUNTER = 0;
	}
	
	/**
	 * Copy constructor, creates a new SpaceNode identical to the one passed.
	 * @param origObj  the object to copy from
	 */
	SpaceNode( SpaceNode<T> origObj) {
		initTracker(origObj);
		this.content = origObj.content;
		this.position = origObj.position.clone();
		this.adjacentEdges = (LinkedList<ManagedObjectReference<T>>)origObj.adjacentEdges.clone();
		this.adjacentTetrahedra = (LinkedList<ManagedObjectReference<T>>)origObj.adjacentTetrahedra.clone();
		this.volume = origObj.volume;
		this.id = origObj.id;
		if (origObj.listeners != null)
			this.listeners = (LinkedList<SpatialOrganizationNodeMovementListener<T>>)origObj.listeners.clone();
		else 
			this.listeners = null;
		// For cleanliness, the reference should be cloned... but it's not necessary:
//		this.som = origObj.som;		
//		this.address = origObj.address;
	}
	
//	/**
//	 * Creates a new SpaceNode with at a given coordinate and associates it with
//	 * a user object.
//	 * 
//	 * @param position
//	 *            The position for this SpaceNode.
//	 * @param content
//	 *            The user object that should be associated with this SpaceNode.
//	 * @deprecated Specify SpatialOrganizationManager when initializing a ManagedObject!
//	 */
//	public SpaceNode(double[] position, T content) {
//		// TODO: oooooooh.... nasty type conversion. Hopefully, the default SOM is of the same type!
//		this(position, content, new ManagedObjectReference<T>((SpatialOrganizationManager<T>)SpatialOrganizationManager.getDefaultSOM()));
//	}
//	
	

//	/**
//	 * Creates a new SpaceNode with at a given coordinate and associates it with
//	 * a user object.
//	 * 
//	 * @param x
//	 *            The x-coordinate for this SpaceNode.
//	 * @param y
//	 *            The y-coordinate for this SpaceNode.
//	 * @param z
//	 *            The z-coordinate for this SpaceNode.
//	 * @param content
//	 *            The user object that should be associated with this SpaceNode.
//	 * @deprecated Specify SpatialOrganizationManager when initializing a ManagedObject!
//	 */
//	public SpaceNode(double x, double y, double z, T content) {
//		// TODO: oooooooh.... nasty type conversion. Hopefully, the default SOM is of the same type!
//		this(x,y,z,content, new ManagedObjectReference<T>((SpatialOrganizationManager<T>)SpatialOrganizationManager.getDefaultSOM()));
//	}
//	
	
	/**
	 * Creates a new SpaceNode with at a given coordinate and associates it with
	 * a user object.
	 * 
	 * @param position
	 *            The position for this SpaceNode.
	 * @param content
	 *            The user object that should be associated with this SpaceNode.
	 * @param som
	 *            The SpatialOrganizationManager that owns this ManagedObject.
	 */
	public SpaceNode(double[] position, T content, CacheManager<T> cm, ManagedObjectReference<T> ref) {
		this(position[0],position[1],position[2],content,cm,ref);
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
	 * @param som
	 *            The SpatialOrganizationManager that owns this ManagedObject.
	 */
	public SpaceNode(double x, double y, double z, T content, CacheManager<T> cm, ManagedObjectReference<T> ref) {
		theLogger.finer("In SpaceNode --- Creating New Node\n");
		if (cm != null) {
			initTracker(cm,ref);
			cm.registerNewNode(this);
		}
		// this should only happen when the initial node is created:
		else {
			// no locking or anything else is necessary
			this.tracker = ref;
		}
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
		return "" + (this.getReference().getAddress() );//% 1000);
	}

	/**
	 * Adds a new edge to the list of incident edges.
	 * 
	 * @param newEdge
	 *            The edge to be added.
	 */
	public void addEdge(ManagedObjectReference<T> newEdge) {
		this.adjacentEdges.addFirst(newEdge);
	}

	/**
	 * Searches for an edge which connects this SpaceNode with another
	 * SpaceNode.
	 * 
	 * @param oppositeNode
	 *            The other node to which the required edge should be connected
	 *            to.
	 * @param som A SpatialOrganizationManager that would host a possibly newly created edge.           
	 * @return An edge connecting this node and <code>oppositeNode</code>. If
	 *         such an edge didn't exist in the list of edges incident to this
	 *         node, a new edge is created.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected Edge<T> searchEdge(SpaceNode<T> oppositeNode, SpatialOrganizationManager<T> som) throws NodeLockedException, ManagedObjectDoesNotExistException {
//		ManagedObjectReference<T> ref = (oppositeNode == null)?null:oppositeNode.getReference();
		for (Edge<T> e : this.getAdjacentEdges()) {
			if (e.getOpposite(this) == oppositeNode) return e;
		}
		return new Edge<T>(this, oppositeNode, som, tracker.getLockingCacheManager());
	}

	/**
	 * Removes a given edge from the list of incident edges.
	 * 
	 * @param edge
	 *            The edge to be removed.
	 */
	protected void removeEdge(Edge<T> edge) {
		adjacentEdges.remove(edge.getReference());
	}

	/**
	 * Removes a given tetrahedron from the list of incident tetrahedra.
	 * 
	 * @param tetrahedron
	 *            The tetrahedron to be remobed.
	 */
	protected void removeTetrahedron(Tetrahedron<T> tetrahedron) {
		adjacentTetrahedra.remove(tetrahedron.getReference());
	}

//	/**
//	 * Returns an {@link Iterable} that allows to iterate over all edges
//	 * incident to this node.
//	 */
//	public Iterable<SpatialOrganizationEdge<T>> getEdges() {
//		return adjacentEdges;
//		// return new Iterable<SpatialOrganizationEdge<T>>() {
//		// public Iterator<SpatialOrganizationEdge<T>> iterator() {
//		// final ExtendedLinkedListElement<Edge<T>> last =
//		// adjacentEdges.getLastElement();
//		// return new Iterator<SpatialOrganizationEdge<T>>() {
//		// ExtendedLinkedListElement<Edge<T>> position =
//		// adjacentEdges.getFirstElement();
//		// // ExtendedLinkedListElement<Edge<T>> next = findNext();
//		// // private ExtendedLinkedListElement<Edge<T>> findNext() {
//		// // next = position.getNext();
//		// // while (next != last &&
//		// (next.getContent().getOpposite(SpaceNode.this) == null))
//		// // next = next.getNext();
//		// // return next;
//		// // }
//		// public boolean hasNext() {
//		// // return next != last;
//		// return position.getNext() != last;
//		// }
//		//
//		// public SpatialOrganizationEdge<T> next() {
//		// // position = next;
//		// // findNext();
//		// position = position.getNext();
//		// return position.getContent();
//		// }
//		//
//		// public void remove() {
//		// throw new UnsupportedOperationException("This Iterator cannot be used
//		// to delete elements!");
//		// }
//		//					
//		// };
//		// }
//		// };
//	}

	public T getUserObject() {
		return content;
	}

	private Iterable<T> wrapEdgeListIntoNeighborNodeIterator(
			final LinkedList<Edge<T>> list) {
		return new Iterable<T>() {
			public Iterator<T> iterator() {
				final Iterator<Edge<T>> listIt =
						list.iterator();
				return new Iterator<T>() {
					public boolean hasNext() {
						return listIt.hasNext();
					}
					// TODO: Wrapping doesn't work any more - 
					// it must be determined whether all objects are accessible before 
					// returning the list. (maybe not, because only called by Fred => never
					// in a modifying process.
					public T next() {
						try {
							return listIt.next().getOpposite(SpaceNode.this)
									.getUserObject();
						} catch (NodeLockedException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
							throw new RuntimeException("wrapping the lists doesn't work any more! Solve this problem!");
						} catch (ManagedObjectDoesNotExistException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
							throw new RuntimeException("wrapping the lists doesn't work any more! Solve this problem!");
						}
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
	public Iterable<T> getNeighbors() throws NodeLockedException, ManagedObjectDoesNotExistException {
		return wrapEdgeListIntoNeighborNodeIterator(getAdjacentEdges());
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
	public Iterable<T> getPermanentListOfNeighbors() throws NodeLockedException, ManagedObjectDoesNotExistException {
		// return
		// wrapEdgeListIntoNeighborNodeIterator(((LinkedList<SpatialOrganizationEdge<T>>)adjacentEdges.clone()));
		LinkedList<T> ret = new LinkedList<T>();
		for (Edge<T> e : this.getAdjacentEdges()) {
			SpaceNode<T> opp = e.getOpposite(this);
			if (opp != null) ret.add(opp.getUserObject());
		}
		return ret;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getVerticesOfTheTetrahedronContaining(double[])
	 */
	public Object[] getVerticesOfTheTetrahedronContaining(double[] position) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (adjacentTetrahedra.isEmpty()) return null;
		// throw new RuntimeException("The point "+this+
		// " is not adjacent to any tetrahedra! Therefore, this point cannot
		// find a tetrahedron containing the position ("+
		// position[0]+"/"+position[1]+"/"+position[2]+")!");
		Tetrahedron<T> insertionTetrahedron = getFirstAdjacentTetrahedron();
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
	/**
	 * @param position
	 * @param userObject
	 * @param nodeReference
	 * @return
	 * @throws PositionNotAllowedException
	 * @throws NodeLockedException
	 * @throws ManagedObjectDoesNotExistException
	 */
	public SpaceNode<T> getNewInstance(double[] position,
			T userObject, ManagedObjectReference<T> nodeReference) throws PositionNotAllowedException, NodeLockedException, ManagedObjectDoesNotExistException {

		// create a new SpaceNode:
		SpaceNode<T> insertPoint = new SpaceNode<T>(position, userObject, tracker.getLockingCacheManager(), nodeReference);

		// the new instance should have the same listeners!
		insertPoint.setListenerList(this.listeners);

		// check if this point is capable of inserting a new point:
		if (adjacentTetrahedra.isEmpty()) {
			// enough nodes collected:
			if (adjacentEdges.size() == 2) {
				// collect the nodes:
				SpaceNode<T> a = tracker.organizeEdge(adjacentEdges.getFirst()).getOpposite(this);
				SpaceNode<T> b = tracker.organizeEdge(adjacentEdges.getLast()).getOpposite(this);
				// clear the edge lists:
				clearAdjacentEdges();
				a.clearAdjacentEdges();
				b.clearAdjacentEdges();
				// now create the first tetrahedron:
				Tetrahedron.createInitialTetrahedron(this, insertPoint, a, b, tracker.getLockingCacheManager());
			}
			else {
				new Edge<T>(this, insertPoint, getSupervisingSOM(), tracker.getLockingCacheManager());
				if (adjacentEdges.size() == 2) {
					new Edge<T>(tracker.organizeEdge(adjacentEdges.getLast()).getOpposite(this), insertPoint, getSupervisingSOM(), tracker.getLockingCacheManager());
				}
			}

		}
		else
		// insert point:
			insertPoint.insert(getFirstAdjacentTetrahedron());
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
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public LinkedList<Tetrahedron<T>> getAdjacentTetrahedra() throws NodeLockedException, ManagedObjectDoesNotExistException {
		LinkedList<Tetrahedron<T>> ret = new LinkedList<Tetrahedron<T>>();
		for (ManagedObjectReference<T> mor : adjacentTetrahedra) {
			ret.add(tracker.organizeTetrahedron(mor));
		}
		return ret;
	}
	
	public int getAdjacentTetrahedraCount() {
		return this.adjacentTetrahedra.size();
	}
	
	public ManagedObjectReference<T> getFirstAdjacentTetrahedronReference() {
		if (adjacentTetrahedra.isEmpty())
			return null;
		else
			return adjacentTetrahedra.getFirst();
	}
	
	public Tetrahedron<T> getFirstAdjacentTetrahedron() throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (adjacentTetrahedra.isEmpty())
			return null;
		else 
			return tracker.organizeTetrahedron(adjacentTetrahedra.getFirst());
	}
	
	/**
	 * Adds a Tetrahedron to this node's list of incident tetrahedra.
	 * @param tetrahedron The tetrahedron that should be added to the list.
	 * TODO: Change the type of the parameter tetrahedron to MOR
	 */
	protected void addAdjacentTetrahedron(Tetrahedron<T> tetrahedron) {
		this.adjacentTetrahedra.add(tetrahedron.getReference());
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
	public void remove() throws NodeLockedException, ManagedObjectDoesNotExistException {
		removeAndReturnCreatedTetrahedron();
	}

	/**
	 * Removes this SpaceNode from the triangulation and restores the gap in the
	 * triangulation by filling it with new triangles.
	 * 
	 * @return A new tetrahedron that was created while filling the created gap.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 * @see OpenTriangleOrganizer#triangulate()
	 */
	private Tetrahedron<T> removeAndReturnCreatedTetrahedron() throws NodeLockedException, ManagedObjectDoesNotExistException {
		invalidate();
		if (listeners != null) {
//	 commented out for the moment.
//	 TODO: find a way how to handle this!			
//			for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
//				listener.nodeAboutToBeRemoved(this);
		}
		// create an OpenTriangleOrganizer that will let every newly created MO decide which SOM it will be assigned to.
		OpenTriangleOrganizer<T> oto =
				OpenTriangleOrganizer.createSimpleOpenTriangleOrganizer(getLockingCacheManger());
		LinkedList<Tetrahedron<T>> messedUpTetrahedra = null;
		// Collect the triangles that are opened by removing the point and
		// remove the corresponding tetrahedra:
		for (Tetrahedron<T> tetrahedron : (LinkedList<Tetrahedron<T>>) getAdjacentTetrahedra()
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
//			 commented out for the moment.
//			 TODO: find a way how to handle this!			
//			for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
//				listener.nodeRemoved(this);
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
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public Tetrahedron<T> searchInitialInsertionTetrahedron(Tetrahedron<T> start)
			throws PositionNotAllowedException, NodeLockedException, ManagedObjectDoesNotExistException {
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
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public static <T> Tetrahedron<T> searchInitialInsertionTetrahedron(
			Tetrahedron<T> start, double[] coordinate)
			throws PositionNotAllowedException, NodeLockedException, ManagedObjectDoesNotExistException {
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
//		Triangle3D<T>[] triangles = current.getAdjacentTriangles();
//		SpaceNode<T>[] opposites = new SpaceNode[4];
//		for (int i = 0; i < opposites.length; i++) {
//			if (!triangles[i].isInfinite()) {
//				opposites[i] = triangles[i].getOppositeTetrahedron(current).getOppositeNode(triangles[i]);
//				if (opposites[i] != null) {
//					triangles[i].update();
//					if (triangles[i].orientation(opposites[i].getPosition(), coordinate) > 0) {
//						synchronized (current.getSOM().stopLock) {
//							System.out.println("the point doesn't lie on the inside of the selected tetrahedron!");
//							System.out.println("Orientation of inner tetrahedron: "+triangles[i].orientation(coordinate,current.getOppositeNode(triangles[i]).getPosition()));
//							if (!current.getSOM().checkTriangulation())
//								System.out.println("Triangulation problem!");
//							if (!current.tracker.getLockingCacheManager().checkTriangles())
//								System.out.println("Triangulation problem!");
//							current.walkToPoint(coordinate);
//							throw new RuntimeException("something's wrong...");
//							
//						}
//					}
//				}
//			}
//		}
		
		return current;
	}

	/**
	 * A private function used inside {@link #insert(Tetrahedron)} to remove a given tetrahedron, 
	 * inform an open triangle organizer about a set of new open triangles and add all tetrahedrons
	 * adjacent to the removed tetrahedron to a queue.
	 * @param tetrahedron The tetrahedron that should be removed.
	 * @param queue The queue which is used to keep track of candidates that might have to be removed.
	 * @param oto The open triangle organizer that keeps track of all open triangles.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	private void processTetrahedron(Tetrahedron<T> tetrahedron,
			LinkedList<Triangle3D<T>> queue, OpenTriangleOrganizer<T> oto) throws NodeLockedException, ManagedObjectDoesNotExistException {
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
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public Tetrahedron<T> insert(Tetrahedron<T> start)
			throws PositionNotAllowedException, NodeLockedException, ManagedObjectDoesNotExistException {
		Tetrahedron<T> insertionStart =
				searchInitialInsertionTetrahedron(start);

		if (listeners != null) {
			// tell the listeners that there will be a new node:
			Object[] verticeContents = insertionStart.getVerticeContents();
//			 commented out for the moment.
//			 TODO: find a way how to handle this!			
//			for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
//				listener.nodeAboutToBeAdded(this, position, verticeContents);
		}

		OpenTriangleOrganizer<T> oto =
				OpenTriangleOrganizer.createSimpleOpenTriangleOrganizer(getSOM(), getLockingCacheManger());
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
				ret = new Tetrahedron<T>(currentTriangle, this, oto, getSOM(), tracker.getLockingCacheManager());
		}
		// }
		// tell the listeners that the node was added:
		if (listeners != null) {
//			 commented out for the moment.
//			 TODO: find a way how to handle this!			
//			for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
//				listener.nodeAdded(this);
		}
		this.revalidate();
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
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	private boolean checkIfTriangulationIsStillValid(double[] newPosition)
			throws PositionNotAllowedException, NodeLockedException, ManagedObjectDoesNotExistException {
		for (Tetrahedron<T> tetrahedron : getAdjacentTetrahedra()) {
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
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	private boolean removeTetrahedronDuringCleanUp(
			Tetrahedron<T> tetrahedronToRemove,
			LinkedList<Tetrahedron<T>> list, LinkedList<SpaceNode<T>> nodeList,
			OpenTriangleOrganizer<T> oto) throws NodeLockedException, ManagedObjectDoesNotExistException {
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
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	private void cleanUp(LinkedList<Tetrahedron<T>> messedUpTetrahedra) throws NodeLockedException, ManagedObjectDoesNotExistException {
		LinkedList<Tetrahedron<T>> outerTetrahedra =
				new LinkedList<Tetrahedron<T>>();
		LinkedList<SpaceNode<T>> problemNodes = new LinkedList<SpaceNode<T>>();
		OpenTriangleOrganizer<T> oto =
				OpenTriangleOrganizer.createSimpleOpenTriangleOrganizer(getLockingCacheManger());
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
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public void restoreDelaunay() throws NodeLockedException, ManagedObjectDoesNotExistException {
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
	public void moveFrom(double[] delta) throws PositionNotAllowedException, NodeLockedException, ManagedObjectDoesNotExistException {
		moveTo(add(this.position, delta));
	}

	/**
	 * Moves this node to a new position.
	 * @param newPosition The new coordinate for this node.
	 * @throws PositionNotAllowedException
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public void moveTo(double[] newPosition) throws PositionNotAllowedException, NodeLockedException, ManagedObjectDoesNotExistException {

		if (checkIfTriangulationIsStillValid(newPosition)) {
			if (listeners != null) {
				double[] delta = subtract(newPosition, this.position);
//				 commented out for the moment.
//				 TODO: find a way how to handle this!			
//				for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
//					listener.nodeAboutToMove(this, delta);
			}
			flipMovements++;
			this.position = newPosition;
			restoreDelaunay();
			if (listeners != null) {
//				 commented out for the moment.
//				 TODO: find a way how to handle this!			
//				for (SpatialOrganizationNodeMovementListener<T> listener : listeners)
//					listener.nodeMoved(this);
			}
		}
		else {
			if (NewDelaunayTest.createOutput())
				NewDelaunayTest.out("Node must be deleted and reinserted!");
			deleteAndInsertMovements++;
			Tetrahedron<T> insertPosition =
					searchInitialInsertionTetrahedron(getFirstAdjacentTetrahedron(), newPosition);
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
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public double[] proposeNewPosition() throws NodeLockedException, ManagedObjectDoesNotExistException {
		double minDistance = Double.MAX_VALUE;
		double[] farthestAwayDiff = null;
		double maxDistance = Double.MIN_VALUE;
		for (Edge<T> edge : this.getAdjacentEdges()) {
			SpaceNode<T> otherNode = edge.getOpposite(this);
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
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public LinkedList<Edge<T>> getAdjacentEdges() throws NodeLockedException, ManagedObjectDoesNotExistException {
		LinkedList<Edge<T>> ret = new LinkedList<Edge<T>>();
		for (ManagedObjectReference<T> ref : adjacentEdges) {
			// TODO From my point of view, this is the WRONG procedure, since 
			// it will bypass the cache!
			ret.add(tracker.organizeEdge(ref));
		}
		return ret;
	}
	
	void clearAdjacentEdges() throws NodeLockedException, ManagedObjectDoesNotExistException {
		for (Edge<T> edge : getAdjacentEdges()) {
			edge.invalidate();
		}
		adjacentEdges.clear();
	}

//	/**
//	 * Organizes a local copy of this SpaceNode. The request is directed to this object's ManagedObjectTracker
//	 * which will then either request a copy from the assigned SpatialOrganizationManager or from the assigned 
//	 * CacheManager, if this object is a local copy. (In the latter case, however, this function will return <code>null</code>, 
//	 * because there may not be two local copies of one object.)
//	 * @return a local deep copy of the Managed Object if this object is not a local copy and <code>null</code> else.
//	 * @see ManagedObjectTracker#organizeNode(ManagedObjectReference)
//	 * @deprecated It doesn't really make any sense to use this function at all. Either,  a SpatialOrganizationManager will
//	 * create a local copy of an object which should be modified itself, or local copies of incident objects are requested.
//	 */
//	public SpaceNode<T> getLocalCopy() throws NodeLockedException, ManagedObjectDoesNotExistException {
//		return tracker.organizeNode(tracker.getReference());
//    }

	
//	/**
//	 * @return the SpatialOrganizationManager that hosts this Managed Object
//	 * @see ini.cx3d.parallelSpatialOrganization.ManagedObject#getSOM()
//	 */
//	public SpatialOrganizationManager getSOM() {
//		return som;
//	}
//	
//	/**
//	 * @return a reference to the Managed Object
//	 */
//	public ManagedObjectReference<T> getReference(){
//		NodeReference<T> ret = new NodeReference(som, address);
//		return ret; 
//		
//	}
	
//	/**
//	 * @param the current transaction ID
//	 */
//	public void setTransactionID(Integer transactionID){
//		this.transactionID = transactionID;
//		
//	}
	
	/**
	 * Assigns this ManagedObject to a CacheManager and locks the object.  
	 * For SpaceNode, the locking and the unlocking have to be synchronized. Besides that, the functionality of this function 
	 * remains the same as of {@link ManagedObject#lock(CacheManager)}.
	 * @param cm The CacheManager that requested this object.
	 * @return the CacheManager that owned this object before this function was called, or <code>null</code>,
	 * if the object was not locked before.
	 */
	public synchronized CacheManager<T> lock(CacheManager<T> cm) {
		return super.lock(cm);
	}
	
	/**
	 * Unlocks this ManagedObject. If this object was linked to a modifying process, this dependency is removed. 
	 * For SpaceNode, the locking and the unlocking have to be synchronized. Besides that, the functionality of this function 
	 * remains the same as of {@link ManagedObject#unlock()}.
	 */
	public synchronized void unlock() {
		super.unlock();
	}
	

}
