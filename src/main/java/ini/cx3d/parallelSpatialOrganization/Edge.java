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

import java.util.LinkedList;

/**
 * This class is used to represent an edge in a triangulation. Each edge has two endpoint and 
 * additionally stores links to all tetrahedra adjacent to it.
 * 
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of the user objects stored in the endpoints of an edge.
 */
public class Edge<T> extends ManagedObject<T>  {
//	/**
//	 * Used to assign a unique identification number to each initialized edge.
//	 */
//	private static int IDCOUNTER = 0;
//	
//	/**
//	 * The identification number of an edge. 
//	 */
//	private int id = IDCOUNTER++;

	private static final long serialVersionUID = -5204616022400943707L;

	/**
	 * The two endpoints of this edge.
	 */
	private ManagedObjectReference<T> a, b;
//	private SpaceNode<T> a, b;
	
	/**
	 * A list of all tetrahedra that are adjacent to this edge. 
	 */
	private LinkedList<ManagedObjectReference<T>> adjacentTetrahedra = new LinkedList<ManagedObjectReference<T>>();
//	private LinkedList<Tetrahedron<T>> adjacentTetrahedra = new LinkedList<Tetrahedron<T>>();

	/**
	 * Stores the cross section area associated with this edge.
	 */
	private double crossSectionArea = 0.0;
	
	
//	/**
//	 * The unique address of this ManagedObject
//	 */
//	protected long address = 0;
//	
//	/**
//	 * The active objects that owns this ManagedObject
//	 */
//	protected SpatialOrganizationManager som= null;
//	
//	//Identifies which transaction (if any) this Edge is included in.
//	private Integer transactionID = null;
	
	
	
	/**
	 * Initializes a new edge with two specified endpoints.
	 * The SOM responsible for this object is chosen based on the SOMs responsible for the incident objects.
	 * @param a The first endpoint of the new edge.
	 * @param b The second endpoint of the new edge.
	 */
	public Edge(SpaceNode<T> a, SpaceNode<T> b, CacheManager<T> cm) {
		this(a,b, (a!=null)?a.getSOM():b.getSOM(), cm);
	}
	
	/**
	 * Initializes a new edge with two specified endpoints.
	 * @param a The first endpoint of the new edge.
	 * @param b The second endpoint of the new edge.
	 * @param som
	 *            The SpatialOrganizationManager that owns this ManagedObject.
	 */
	public Edge(SpaceNode<T> a, SpaceNode<T> b, SpatialOrganizationManager<T> som, CacheManager<T> cm) {
		initTracker(cm, som);
		cm.registerNewEdge(this);
		
		this.a = (a==null)?null:a.getReference();
		this.b = (b==null)?null:b.getReference();

		if (a != null) a.addEdge(this.getReference());
		if (b != null) b.addEdge(this.getReference());
//		this.posA = (a!=null)?a.addEdge(this):null;
//		this.posB = (b!=null)?b.addEdge(this):null;
	}

	
	/**
	 * Copy constructor, creates a new Edge identical to the one passed.
	 * @param origObj the object to copy from
	 */
	public Edge(Edge<T> origObj) {
		initTracker(origObj);
		this.a = origObj.a;
		this.b = origObj.b;
		
		//I guess we do not need to register as a neighbour again??
		//I have therefore commented this code
		//if (a != null) a.addEdge(this);
		//if (b != null) b.addEdge(this);
//		this.posA = (a!=null)?a.addEdge(this):null;
//		this.posB = (b!=null)?b.addEdge(this):null;
		this.adjacentTetrahedra =
				(LinkedList<ManagedObjectReference<T>>) origObj.adjacentTetrahedra.clone();
		this.crossSectionArea = origObj.crossSectionArea;
	}

	private SpaceNode<T> getNodeA() throws NodeLockedException, ManagedObjectDoesNotExistException {
		return tracker.organizeNode(a);
	}

	private SpaceNode<T> getNodeB() throws NodeLockedException, ManagedObjectDoesNotExistException {
		return tracker.organizeNode(b);
	}

	public ManagedObjectReference<T> getNodeAReference() {
		return (a==null)?null:a.getReference();
	}
	
	public ManagedObjectReference<T> getNodeBReference() {
		return (b==null)?null:b.getReference();
	}
	
	/** 
	 * {@inheritDoc}
	 * TODO: this function should receive a MOR
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public SpaceNode<T> getOpposite(SpaceNode<T> comingFrom) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (comingFrom == null) {
			if (a == null) return getNodeB();
			else if (b == null) return getNodeA();
			else throw new UnknownManagedObjectException("The edge "+this+" is not adjacent to the node "+comingFrom);
		} 
		else {
			ManagedObjectReference<T> reference = comingFrom.getReference();
			if (reference.equals(a)) return getNodeB();
			else if (reference.equals(b)) return getNodeA();
			else throw new UnknownManagedObjectException("The edge "+this+" is not adjacent to the node "+comingFrom);
		}
	}
	
//	public T getOppositeElement_(T element) {
//		if (a != null) {
//			if (a.getUserObject() == element) {
//				if (b != null)
//					return b.getUserObject();
//			}
//			else if (b != null) {
//				if (b.getUserObject() == element) 
//						return a.getUserObject();
//			}
//		}
//		if (b != null) {
//			if (b.getUserObject() == element) {
//				if (a != null)
//					return a.getUserObject();
//			}
//		}
//		return null;
//	}
	
	/**
	 * {@inheritDoc}
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public T getOppositeElement(T element) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (a!=null  && b!=null) {
			T aT = getNodeA().getUserObject();
			if(element == aT){
				return getNodeB().getUserObject();
			}else{
				return getNodeA().getUserObject();
			}
		}
		return null;
	}
	
	/**
	 * {@inheritDoc}
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public T getFirstElement() throws NodeLockedException, ManagedObjectDoesNotExistException {
		return getNodeA().getUserObject();
	}
	
	/**
	 * {@inheritDoc}
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public T getSecondElement() throws NodeLockedException, ManagedObjectDoesNotExistException {
		return getNodeB().getUserObject();
	}
	
	/**
	 * {@inheritDoc}
	 */
	public double getCrossSection() {
		return this.crossSectionArea;
	}

	/**
	 *	@return A string representation of this edge. The format of this string is
	 *     "(ID1 - ID2)", where ID1 and ID2 denote the identification numbers of the
	 *     endpoints of this edge. 
	 */
	public String toString() {
		if (this.isValid())
			return "(" + a + " - " + b + ")";
		else
			return "[(" + a + " - " + b + ")]";
	}
	
	/**
	 * Tests whether this edge is connecting a pair of points.
	 * @param a The first node.
	 * @param b The second node.
	 * @return <code>true</code>, if this edge connects <code>a</code> and <code>b</code>.
	 * TODO: this function should receive MOR's - maybe...
	 */
	protected boolean equals(SpaceNode<T> a, SpaceNode<T> b) {
		if (a == null) {
			if (b == null) 
				return (this.a == null) && (this.b == null);
			else {
				ManagedObjectReference<T> bref = b.getReference();
				return ((this.a == null) && (this.b.equals(bref))) ||
				 	   ((this.b == null) && (this.a.equals(bref)));
			}
		}
		else if (b == null) {
			ManagedObjectReference<T> aref = a.getReference();
			return ((this.a == null) && (this.b.equals(aref))) ||
		 	       ((this.b == null) && (this.a.equals(aref)));
		}
		else {
			ManagedObjectReference<T> aref = a.getReference();
			ManagedObjectReference<T> bref = b.getReference();
			return ((this.a.equals(aref)) && (this.b.equals(bref))) || 
		       ((this.b.equals(aref)) && (this.a.equals(bref)));
		}
	}
	
	/**
	 * Removes a tetrahedron from this edge's list of tetrahedra. If this edge is not incident to
	 * any tetrahedra after the removal of the specified tetrahedron, the edge removes itself from
	 * the triangulation by calling {@link #remove()}.
	 * @param tetrahedron A tetrahedron incident to this edge which should be removed.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected void removeTetrahedron(Tetrahedron<T> tetrahedron) throws NodeLockedException, ManagedObjectDoesNotExistException {
		adjacentTetrahedra.remove(tetrahedron.getReference());
//		element.remove();
		if (adjacentTetrahedra.isEmpty())
			remove();
	}
	
	/**
	 * Removes this edge from the triangulation. To do so, the two endpoints are informed
	 * that the edge was removed. 
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 *  
	 */
	protected void remove() throws NodeLockedException, ManagedObjectDoesNotExistException {
		invalidate();
		if (a != null)
			getNodeA().removeEdge(this);
		if (b != null)
			getNodeB().removeEdge(this);
		//		if (posA != null)
//			posA.remove();
//		if (posB != null)
//			posB.remove();
	}
	
	/**
	 * Returns the list of incident tetrahedra.
	 * @return The list of incident tetrahedra.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected LinkedList<Tetrahedron<T>> getAdjacentTetrahedra() throws NodeLockedException, ManagedObjectDoesNotExistException {
		LinkedList<Tetrahedron<T>> ret = new LinkedList<Tetrahedron<T>>();
		for (ManagedObjectReference<T> mor : adjacentTetrahedra) {
			ret.add(tracker.organizeTetrahedron(mor));
		}
		return ret;
	}
	
	/**
	 * Adds a tetrahedron to this edge's list of incident tetrahedra.
	 * 
	 * @param tetrahedron The tetrahedron that should be added.
	 * 
	 * TODO: Maybe change the type of the parameter to MOR
	 */
	protected void addAdjacentTetrahedron(Tetrahedron<T> tetrahedron) {
		this.adjacentTetrahedra.add(tetrahedron.getReference());
	}
	
	/**
	 * Changes the cross section area of this edge.
	 * @param change The value by which the cross section area of this edge has changed.
	 * At initialization, this area is set to zero and all tetrahedra that are registered as
	 * incident tetrahedra increase the cross section area.
	 */
	protected void changeCrossSectionArea(double change) {
		this.crossSectionArea += change;
	}
	
//	/**
//	 * @return the unique address of this Managed Object
//	 * @see ini.cx3d.parallelSpatialOrganization.ManagedObject#getAddress()
//	 */
//	public long getAddress() {
//		return address;
//	}

	/**
	 * Organizes a local copy of this Edge. The request is directed to this object's ManagedObjectTracker
	 * which will then either request a copy from the assigned SpatialOrganizationManager or from the assigned 
	 * CacheManager, if this object is a local copy. (In the latter case, however, this function will return <code>null</code>, 
	 * because there may not be two local copies of one object.)
	 * @return a local deep copy of the Managed Object if this object is not a local copy and <code>null</code> else.
	 * @see ManagedObjectTracker#organizeNode(ManagedObjectReference)
	 * @deprecated It doesn't really make any sense to use this function at all. Either,  a SpatialOrganizationManager will
	 * create a local copy of an object which should be modified itself, or local copies of incident objects are requested.
	 */
	public Edge<T> getLocalCopy() throws NodeLockedException, ManagedObjectDoesNotExistException {
		return tracker.organizeEdge(tracker.getReference());
    }

	
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
//		EdgeReference<T> ret = new EdgeReference(som, address);
//		return ret; 
//		
//	}
//	
//	/**
//	 * @param the current transaction ID
//	 */
//	public void setTransactionID(Integer transactionID){
//		this.transactionID = transactionID;
//		
//	}
	
}
