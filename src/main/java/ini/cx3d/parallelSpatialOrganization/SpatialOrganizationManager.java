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

import java.io.Serializable;
import java.util.Date;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.Random;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * This class manages a part of the triangulation.
 * It is needed to perform the transaction between virtual memory addresses and
 * locally stored objects. The latter are stored in HashTables to guarantee fast 
 * translations from virtual memory addresses to local addresses.
 * @author Dennis Goehlsdorf, Toby Weston
 * TODO make this class an ProActive-ActiveObject
 */
public class SpatialOrganizationManager<T> implements Serializable{
	
	//Get a logger
	private static Logger theLogger = Logger.getLogger(SpatialOrganizationManager.class.getName());	
	///////////////////////////////////////////////////////////////////
	/// Replace with config. file at some point.....
	///////////////////////////////////////////////////////////////////	
	{
		//theLogger.setLevel(Level.FINER);
//		theLogger.setLevel(Level.FINEST);
		theLogger.setLevel(Level.OFF);
	}
	///////////////////////////////////////////////////////////////////		
	
	
	//Set this to false to imporove eficiency
	boolean debug = true;
	
	public static boolean javaMultiThreaded = true; 
	
	//Change this to use ProActive discovery or some other method of providing the default SOM
	// TODO: If used, this will crash the system since no NodeToSOMAssignmentPolicy is given.
	private static SpatialOrganizationManager defaultSOM = new SpatialOrganizationManager(null, 0L, 50000L);
	
	Hashtable<ManagedObjectReference<T>, SpaceNode<T>> nodes;
	Hashtable<ManagedObjectReference<T>, Tetrahedron<T>> tetrahedra;
	Hashtable<ManagedObjectReference<T>, Triangle3D<T>> triangles;
	Hashtable<ManagedObjectReference<T>, Edge<T>> edges;
	long maxAddress, current;
	Hashtable<Long, Integer> activePs = new Hashtable<Long,Integer>(20000);
	
	LinkedList<CacheManager<T>> allActions = new LinkedList<CacheManager<T>>();
	
	ActionQueue<T> actionQueue = new ActionQueue<T>();
	int activeProcesses = 0;
	int maxParallelActiveProcesses = 2;
	int nonFinalizedProcesses = 0;
	int initiatedThreads = 0;
	
	double maxDivergence = 0.03;
	
	//Unique Identifier for this SOM
	Long myUniqueID; 

		
	
	/**
	 * List of nodes that are not yet added to this SOM but that will be added soon.
	 */
	LinkedList<ManagedObjectReference<T>> pendingNodes = new LinkedList<ManagedObjectReference<T>>();
	
	NodeToSOMAssignmentPolicy<T> nodeToSOMAssignmentPolicy;
	
	// only for debugging: used to interrupt all processes
//	Object stopLock = new Object();
	
	//Global list of locked objects, to avoid checking each cache manager (may 
	//not be a good idea.
	//List<ManagedObjectReference> locked = new ArrayList();
	
//	//The list of currently running transactions
//	Hashtable<Integer, CacheManager> transactions = new Hashtable();
	
	/**
	 * No argument constructor for ProActive compatibility.
	 */
	public SpatialOrganizationManager() {
		// TODO adjust the sizes of the following HashTables! Standard size might not 
		// be appropriate!
		this.nodes = new Hashtable<ManagedObjectReference<T>, SpaceNode<T>>(10000);
		this.tetrahedra = new Hashtable<ManagedObjectReference<T>, Tetrahedron<T>>(70000);
		this.triangles = new Hashtable<ManagedObjectReference<T>, Triangle3D<T>>(140000);
		this.edges = new Hashtable<ManagedObjectReference<T>, Edge<T>>(70000);
		myUniqueID = new Long(new Date().getTime() + new Random().nextLong());
	}
	
	/**
	 * No argument constructor for ProActive compatibility.
	 */
	public void init(NodeToSOMAssignmentPolicy<T> nodeToSOMAssignmentPolicy, long addressRangeMin, long addressRangeMax) {
		this.nodeToSOMAssignmentPolicy = nodeToSOMAssignmentPolicy;
		this.maxAddress = addressRangeMax;
		this.current = addressRangeMin;
	}

	
	
	public SpatialOrganizationManager(NodeToSOMAssignmentPolicy<T> nodeToSOMAssignmentPolicy, long addressRangeMin, long addressRangeMax) {
		this.nodeToSOMAssignmentPolicy = nodeToSOMAssignmentPolicy;
		this.maxAddress = addressRangeMax;
		this.current = addressRangeMin;
		// TODO adjust the sizes of the following HashTables! Standard size might not 
		// be appropriate!
		this.nodes = new Hashtable<ManagedObjectReference<T>, SpaceNode<T>>(10000);
		this.tetrahedra = new Hashtable<ManagedObjectReference<T>, Tetrahedron<T>>(70000);
		this.triangles = new Hashtable<ManagedObjectReference<T>, Triangle3D<T>>(140000);
		this.edges = new Hashtable<ManagedObjectReference<T>, Edge<T>>(70000);
//		this.nodes = new Hashtable<ManagedObjectReference<T>, SpaceNode<T>>(20);
//		this.tetrahedra = new Hashtable<ManagedObjectReference<T>, Tetrahedron<T>>(20);
//		this.triangles = new Hashtable<ManagedObjectReference<T>, Triangle3D<T>>(20);
//		this.edges = new Hashtable<ManagedObjectReference<T>, Edge<T>>(20);
		
		myUniqueID = new Long(new Date().getTime() + new Random().nextLong());
	}
	
	/**
	 * Creates the first node of a simulation. This node is added to this SOM's hashtable.
	 * Watch out! This implementation is not Thread-safe. The first node should be created before any other process might interact with the
	 * triangulation!
	 * TODO: Wrap this into a new instance of CacheManager? (Might also cause problems, because this process HAS to be finished before
	 * anything else is started.
	 * @param coordinate 
	 * @param userObject
	 * @return
	 */
	public SpaceNodeFacade<T> createInitialNode(double[] coordinate, T userObject) {

		theLogger.finest("In SOM --- Creating Initial Node");
		if (!nodes.isEmpty())
			throw new RuntimeException("This function may only be called once to create an initial node! Use SpaceNodeFacade.getNewInstance() instead!");

		theLogger.finest("In SOM --- Before get this");
		SpatialOrganizationManager<T> tempSOM = getThis();

		theLogger.finest("In SOM --- Before make new Ref");
		ManagedObjectReference<T> tempObjRef = new ManagedObjectReference<T>(getUniqueAddress(), tempSOM);
		 
		theLogger.finest("In SOM --- Before make new Space Node");
		SpaceNode<T> firstNode = new SpaceNode<T>(coordinate, userObject, null, tempObjRef);

		theLogger.finest("In SOM --- Adding Node\n");
		nodes.put(firstNode.getReference(), firstNode);

		theLogger.finest("In SOM --- Making Facade\n");
		SpaceNodeFacade<T> nodeFacade = new SpaceNodeFacade<T>(firstNode.getReference());
		return nodeFacade;
	}

	/**
	 * Creates the first node of a simulation. This node is added to this SOM's hashtable.
	 * @param x
	 * @param y
	 * @param z
	 * @param userObject
	 * @return
	 */
	public SpaceNodeFacade<T> createInitialNode(double x, double y, double z, T userObject) {
		return createInitialNode(new double[] {x,y,z}, userObject);
	}
	
	
	ManagedObjectReference<T> getANodeReference() {
		synchronized (nodes) {
			if (this.nodes.size() > 0)
				return nodes.keys().nextElement();
			else
				// TODO: This should redirect the request to some other SOM:
				throw new RuntimeException("This SpatialOrganizationManager doesn't own any nodes at the moment!");
		}
	}
	
	ManagedObjectReference<T> getATetrahedronReference() {
		synchronized (tetrahedra) {
			if (this.tetrahedra.size() > 0)
				return tetrahedra.keys().nextElement();
			else
				// TODO: This should redirect the request to some other SOM:
				throw new RuntimeException("This SpatialOrganizationManager doesn't own any tetrahedra at the moment!");
		}
	}

	boolean checkTriangulation() {
		Object[] list = null;
		synchronized (triangles) {
			list = triangles.keySet().toArray();
		}				
		for (Object ref : list) {
			
			try {
				Triangle3D<T> triangle = getCopyOfTriangle((ManagedObjectReference<T>)ref, null);
				if (!triangle.checkConformity())
					return false;
			} catch (NodeLockedException e) {
			} catch (ManagedObjectDoesNotExistException e) {
			}
		}
		return true;
	}
	
	/**
	 * Adjusts the maximal number of nodes operations that may be pending before the
	 * data structure will be classified as non-reliable.
	 * @param newMaxDivergence The maximum fraction of nodes for which a modification was already 
	 * requested but not yet applied. The default is 0.03 (if the data structure contains 10000 nodes, 
	 * a maximum of 300 operations may be in the queue). 
	 * @see #isReliable()
	 */
	public void setMaxDivergence(double newMaxDivergence) {
		this.maxDivergence = newMaxDivergence;
	}

	/**
	 * Determines whether to many operations on the data structure are pending.
	 * This is done by comparing the number of operations that were requested but not yet 
	 * executed with the number of nodes times the maximal Divergence 
	 * (see {@link #setMaxDivergence(double)}) 
	 * @return <code>true</code>, if the data structure is still reliable and <code>false</code> otherwise.
	 */
	public boolean isReliable() {
		return this.maxDivergence * this.nodes.size() > (this.actionQueue.getSize()+this.activeProcesses);
	}
	
	public void waitUntilReliable() {
		synchronized (actionQueue) {
			while (!isReliable()) {
				try {
					actionQueue.wait();
				} catch (InterruptedException e) {
				}
			}
		}
	}
	
	/**
	 * Informs this SOM that an active process has finished. It does not matter whether 
	 * the process was successful or not.
	 */
	synchronized void activeProcessHasFinished() {
		synchronized (actionQueue) {
			activeProcesses--;
			checkActionQueue();
			actionQueue.notify();
		}
	}
	
	/**
	 * Tests whether there are still pending actions in the queue and starts new processes,
	 * if the maximum number of parallel active processes is not yet reached.
	 */
	synchronized void checkActionQueue() {
		synchronized (actionQueue) {
			while ((activeProcesses < maxParallelActiveProcesses) && 
					(activeProcesses == 0 || activeProcesses*100 < nodes.size()) && 
					(!actionQueue.isEmpty())) {
				activeProcesses++;
				final CacheManager<T> action = actionQueue.nextAction();
				Integer value = activePs.get(new Long(action.getID()));
				if (value != null)
					activePs.put(new Long(action.getID()), new Integer(value+1));
				else activePs.put(new Long(action.getID()), new Integer(1));
				nonFinalizedProcesses++;
				if (javaMultiThreaded) {
//					CacheManager<T> act = actionQueue.nextAction();
//					new Thread(act.descriptionString()) {
					new Thread("blubb") {
//						CacheManager<T> action = actionQueue.nextAction();
						public void run() {
							action.run();
//							action = null;
						}
						protected void finalize() throws Throwable {
							
							super.finalize();
						}
					}.start();
				}
				// not multi-threaded => run process right now!
				else
					action.run();
			}
		}
	}
	
	public void processFinalized(long number) {
		synchronized (actionQueue) {
			this.nonFinalizedProcesses--;
			Integer value = activePs.remove(new Long(number));
			if (value > 1)
				activePs.put(new Long(number), value-1);
		}
	}
	
	void addPendingNode(ManagedObjectReference<T> nodeReference) {
		theLogger.finest("In SOM --- addPendingNode");
		//synchronized (pendingNodes) {
			pendingNodes.add(nodeReference);
		//}
	}
	
	void removePendingNode(ManagedObjectReference<T> nodeReference) {
		synchronized (pendingNodes) {
			pendingNodes.remove(nodeReference);
			pendingNodes.notifyAll();
		}
	}
	
	boolean isNodePending(ManagedObjectReference<T> nodeReference) {
		synchronized (pendingNodes) {
			return pendingNodes.contains(nodeReference);
			
		}
	}
	
	
//	/**
//	 * Creates a new SpaceNode with at a given coordinate and associates it with
//	 * a user object.
//	 * 
//	 * @param position
//	 *            The position for this SpaceNode.
//	 * @param content
//	 *            The user object that should be associated with this SpaceNode.
//	 * @param som
//	 *            The SpatialOrganizationManager that owns this ManagedObject.
//	 */
//	public void newSpaceNode(double[] position, long address, T content){
//		
//		SpaceNode newNode = new SpaceNode(position, content, this);
//		ManagedObjectReference nodeRef = new ManagedObjectReference(address, this);
//		nodes.put(nodeRef, newNode);
//	}
	
	/**
	 * Returns the Node associated with a given reference. The returned link is a link to the original object.
	 * Therefore, this function may only be called for the correct SOM.
	 * @param ref The reference to the node.
	 * @return A link to the original object.
	 * @throws ManagedObjectDoesNotExistException if the object does not exist.
	 * @throws RuntimeException if the object belongs to a different SOM.
	 */
	SpaceNode<T> findNode(ManagedObjectReference<T> ref) throws ManagedObjectDoesNotExistException {
		SpaceNode<T> node =null;
//		synchronized (stopLock) {
			
//		}
		// make sure that the desired node is not removed from the pendingNodes-List
		// right after you checked the nodes-hashtable!
		synchronized (pendingNodes) {
			synchronized (nodes) {
				node = nodes.get(ref);
			}
			if (node == null)
				//if (ref.getSOM().getUniqueSOMID() == getUniqueSOMID()) {
				if (belongsToThisSOM(ref)) {
					if (!isNodePending(ref)) {
						throw new ManagedObjectDoesNotExistException(ref,"Reference to an unknown Node!");
					}
					else {
						// wait until the desired node is no longer in the list of pending nodes
						while (isNodePending(ref)) {
							try {
								pendingNodes.wait();
							} catch (InterruptedException e) {
							}
						}
						// now the requested node should be in the hashtable...
						// therefore, try the same procedure over again
						return findNode(ref);
					}
				}
				else 
					throw new RuntimeException("This function returns a link to the original object and may therefore only called from inside the proper SOM!");
			return node;
		}
	}
	
	/**
	 * Returns the tetrahedron associated with a given reference. The returned link is a link to the original object.
	 * Therefore, this function may only be called for the correct SOM.
	 * @param ref The reference to the tetrahedron.
	 * @return A link to the original object.
	 * @throws ManagedObjectDoesNotExistException if the object does not exist.
	 * @throws RuntimeException if the object belongs to a different SOM.
	 */
	Tetrahedron<T> findTetrahedron(ManagedObjectReference<T> ref) throws ManagedObjectDoesNotExistException {
		
		Tetrahedron<T> tetrahedron = null;
		synchronized (tetrahedra) {
			tetrahedron = tetrahedra.get(ref);	
		}
		if (tetrahedron == null)
			//if (ref.getSOM().getUniqueSOMID() == getUniqueSOMID())
			if (belongsToThisSOM(ref)) {
				throw new ManagedObjectDoesNotExistException(ref,"Reference to an unknown Tetrahedron!");
			}
			else 
				throw new RuntimeException("This function returns a link to the original object and may therefore only called from inside the proper SOM!");
		return tetrahedron;
	}
	
	/**
	 * Returns the triangle associated with a given reference. The returned link is a link to the original object.
	 * Therefore, this function may only be called for the correct SOM.
	 * @param ref The reference to the triangle.
	 * @return A link to the original object.
	 * @throws ManagedObjectDoesNotExistException if the object does not exist.
	 * @throws RuntimeException if the object belongs to a different SOM.
	 */
	Triangle3D<T> findTriangle(ManagedObjectReference<T> ref) throws ManagedObjectDoesNotExistException {
		Triangle3D<T> triangle = null;
		synchronized (triangles) {
			triangle = triangles.get(ref);
			
		}
		if (triangle == null)
			//if (ref.getSOM().getUniqueSOMID() == getUniqueSOMID())
			if (belongsToThisSOM(ref)) {
				throw new ManagedObjectDoesNotExistException(ref,"Reference to an unknown Triangle!");
			}
			else 
				throw new RuntimeException("This function returns a link to the original object and may therefore only called from inside the proper SOM!");
		return triangle;
	}
	
	/**
	 * Returns the edge associated with a given reference. The returned link is a link to the original object.
	 * Therefore, this function may only be called for the correct SOM.
	 * @param ref The reference to the edge.
	 * @return A link to the original object.
	 * @throws ManagedObjectDoesNotExistException if the object does not exist.
	 * @throws RuntimeException if the object belongs to a different SOM.
	 */
	Edge<T> findEdge(ManagedObjectReference<T> ref) throws ManagedObjectDoesNotExistException {
		Edge<T> edge = null;
		synchronized (edges) {
			edge = edges.get(ref);
		}
		if (edge == null)
			//if (ref.getSOM().getUniqueSOMID() == getUniqueSOMID())
			if (belongsToThisSOM(ref)) {
				throw new ManagedObjectDoesNotExistException(ref,"Reference to an unknown Edge!");
			}
			else 
				throw new RuntimeException("This function returns a link to the original object and may therefore only called from inside the proper SOM!");
		return edge;
	}
	
	/**
	 * Organizes a local copy of a SpaceNode.
	 * @param vma The ManagedObjectReference pointing to the requested SpaceNode.
	 * @param customer The CacheManager requesting this SpaceNode, or <code>null</code>, if the object doesn't need to be locked. 
	 * (This would imply that the calling process won't modify the node.)
	 * @return A local copy of the requested SpaceNode if possible and <code>null</code>, if the CacheManager requesting the node 
	 * already owns a local copy of that object. 
	 * @throws NodeLockedException If the SpaceNode is locked by another modifying process.
	 * @throws ManagedObjectDoesNotExistException If the object doesn't exist anymore.
	 */
	public SpaceNode<T> getCopyOfSpaceNode(ManagedObjectReference<T> vma, CacheManager<T> customer) 
			throws NodeLockedException, ManagedObjectDoesNotExistException {
		//Check if this is the correct SOM to handle the request
		//if(vma.getSOM().equals(getThis())){
		//if(vma.getSOM().getUniqueSOMID() == getUniqueSOMID()){
		if (belongsToThisSOM(vma)) {
			SpaceNode<T> originalNode = findNode(vma);
			// try to get a lock on the SpaceNode if the node was requested by a modifying process:
			if (customer != null) {	
				// try to acquire a lock on the node:
				CacheManager<T> copyOwner = originalNode.lock(customer);
				// successful:
				if (copyOwner == null)
					// create a local copy:
					return new SpaceNode<T>(originalNode);
				// node was already owned by some CacheManager:
				else {
					// if that was the "customer" himself, return null so he NEVER gets two copies. (VERY BAD CASE!)
					if (copyOwner.equals(customer))
						return null;
					// if the node was locked by someone else, that's bad luck for the "customer"...
					else
						throw new NodeLockedException(vma,copyOwner.getID(),"Attempt to create a local copy of a locked object!");
				}
			}
			// otherwise, just return a local copy and don't care about locking it:
			else {
				SpaceNode<T> spaceNode = new SpaceNode<T>(originalNode);
				spaceNode.unlock();
				return spaceNode;
			}
		}
		else{
			//The Object is not ascociated with this SOM so call the method on
			//its own SOM
			return vma.getSOM().getCopyOfSpaceNode(vma, customer);
		}
	}
	
	/**
	 * Organizes a local copy of a tetrahedron. If this function is called by a modifying process (CacheManager), 
	 * this function will also lock all nodes incident to the requested object and send local copies of them
	 * to the calling CacheManager. 
	 * @param vma The reference of the requested object.
	 * @param customer The CacheManager that made the request for a ManagedObject, or <code>null</code>, 
	 * if the object doesn't need to be locked. (This would imply that the calling process won't modify the node.)
	 * @return A local copy of the requested ManagedObject if possible and <code>null</code>, if the CacheManager requesting the object 
	 * already owns a local copy of that object. 
	 * @throws NodeLockedException If any of the nodes incident to the requested object is locked by another modifying process.
	 * @throws ManagedObjectDoesNotExistException If the object doesn't exist anymore.
	 */
	public Tetrahedron<T> getCopyOfTetrahedron(ManagedObjectReference<T> vma,
			CacheManager<T> customer) throws NodeLockedException,
			ManagedObjectDoesNotExistException {
		//if (vma.getSOM().equals(getThis())) {
		//if(vma.getSOM().getUniqueSOMID() == getUniqueSOMID()){
		if (belongsToThisSOM(vma)) {
			Tetrahedron<T> originalTetrahedron = findTetrahedron(vma); 
			if (customer != null) {
				
				/* TODO:
				 * Instead of locking the nodes one after each other and then locking the tetrahedron, 
				 * simply do it the following way:
				 * 1) lock the tetrahedron by requesting a cache copy (local copy)
				 *    (fails, if locked)
				 * 2) call Tetrahedron.getAdjacentNodes() on the local copy, which will automatically 
				 *    add the copies to the appropriate CacheManager.
				 *    
				 * (also change this for edges and triangles (see below)
				*/
				
				// first, attempt to lock all incident nodes:
				ManagedObjectReference[] incidentNodes = originalTetrahedron.getAdjacentNodeReferences(); 
				for (int i = 0; i < 4; i++) {
					if (incidentNodes[i] != null) {
						customer.getNode(incidentNodes[i]);
//						SpaceNode<T> copy = getCopyOfSpaceNode(incidentNodes[i], customer);
//						// add incident nodes to the Cache
//						if (copy != null)
//							customer.cacheNode(copy);
					}
				}
				// if that was successful, a copy of the requested Tetrahedron will be transmitted 
				// - follow the same scheme as for SpaceNode:
					// try to acquire a lock on the tetrahedron:
				// (The purpose of this call is more about making sure that no CacheManager can get 2 copies of the same object
				// than about locking the object. Real "locks" are needed for nodes only.)  
				CacheManager<T> copyOwner = originalTetrahedron.lock(customer);
				// successful:
				if (copyOwner == null) {
					// create a local copy:
					return new Tetrahedron<T>(originalTetrahedron);
				}
				// tetrahedron was already owned by some CacheManager:
				else {
					// if that was the "customer" himself, return null so he NEVER gets two copies. (VERY BAD CASE!)
					if (copyOwner.equals(customer))
						return null;
					// if the tetrahedron was locked by someone else, something very unexpected happened because the incident nodes were available:
					else {
//						synchronized (stopLock) {
							throw new RuntimeException("Attempt to create a local copy of a tetrahedron failed because it was locked. This should not have happen since none of the incident nodes was locked!");
//						}
					}
				}
			}
			else {
				Tetrahedron<T> tetrahedron = new Tetrahedron<T>(originalTetrahedron);
				tetrahedron.unlock();
				return tetrahedron;
			}
		}
		else return vma.getSOM().getCopyOfTetrahedron(vma, customer);
	}

	/**
	 * Organizes a local copy of a triangle. If this function is called by a modifying process (CacheManager), 
	 * this function will also lock all nodes incident to the requested object and send local copies of them
	 * to the calling CacheManager. 
	 * @param vma The reference of the requested object.
	 * @param customer The CacheManager that made the request for a ManagedObject, or <code>null</code>, 
	 * if the object doesn't need to be locked. (This would imply that the calling process won't modify the node.)
	 * @return A local copy of the requested ManagedObject if possible and <code>null</code>, if the CacheManager requesting the object 
	 * already owns a local copy of that object. 
	 * @throws NodeLockedException If any of the nodes incident to the requested object is locked by another modifying process.
	 * @throws ManagedObjectDoesNotExistException If the object doesn't exist anymore.
	 */
	public Triangle3D<T> getCopyOfTriangle(ManagedObjectReference<T> vma,
			CacheManager<T> customer) throws NodeLockedException,
			ManagedObjectDoesNotExistException {
		//if (vma.getSOM().equals(getThis())) {
		//if(vma.getSOM().getUniqueSOMID() == getUniqueSOMID()){
		if (belongsToThisSOM(vma)) {
			Triangle3D<T> originalTriangle = findTriangle(vma);
			if (customer != null) {
				// first, attempt to lock all incident nodes:
				ManagedObjectReference[] incidentNodes = originalTriangle.getAdjacentNodeReferences();  
				for (int i = 0; i < 3; i++) {
					if (incidentNodes[i] != null) {
						customer.getNode(incidentNodes[i]);
//						SpaceNode<T> copy = getCopyOfSpaceNode(incidentNodes[i], customer);
//							// add incident nodes to the Cache
//						if (copy != null)
//							customer.cacheNode(copy);
					}
				}
				// if that was successful, a copy of the requested Triangle will be transmitted 
				// - follow the same scheme as for SpaceNode:
				// try to acquire a lock on the tetrahedron:
				// (The purpose of this call is more about making sure that no CacheManager can get 2 copies of the same object
				// than about locking the object. Real "locks" are needed for nodes only.)  
				CacheManager<T> copyOwner = originalTriangle.lock(customer);
				// successful:
				if (copyOwner == null) {
					// create a local copy:
					return new Triangle3D<T>(originalTriangle);
				}
				// triangle was already owned by some CacheManager:
				else {
					// if that was the "customer" himself, return null so he NEVER gets two copies. (VERY BAD CASE!)
					if (copyOwner.equals(customer))
						return null;
					// if the triangle was locked by someone else, something very unexpected happened because the incident nodes were available:
					else
						throw new RuntimeException("Attempt to create a local copy of a triangle failed because it was locked. This should not have happen since none of the incident nodes was locked!");
				}
			}				
			else {
				Triangle3D<T> triangle = new Triangle3D<T>(originalTriangle);
				triangle.unlock();
				return triangle;
			}
	    }
		else return vma.getSOM().getCopyOfTriangle(vma, customer);
	}

	/**
	 * Organizes a local copy of an edge. If this function is called by a modifying process (CacheManager), 
	 * this function will also lock all nodes incident to the requested object and send local copies of them
	 * to the calling CacheManager. 
	 * @param vma The reference of the requested object.
	 * @param customer The CacheManager that made the request for a ManagedObject, or <code>null</code>, 
	 * if the object doesn't need to be locked. (This would imply that the calling process won't modify the node.)
	 * @return A local copy of the requested ManagedObject if possible and <code>null</code>, if the CacheManager requesting the object 
	 * already owns a local copy of that object. 
	 * @throws NodeLockedException If any of the nodes incident to the requested object is locked by another modifying process.
	 * @throws ManagedObjectDoesNotExistException If the object doesn't exist anymore.
	 */
	public Edge<T> getCopyOfEdge(ManagedObjectReference<T> vma,
			CacheManager<T> customer) throws NodeLockedException,
			ManagedObjectDoesNotExistException {
		//if (vma.getSOM().equals(getThis())) {
		//if(vma.getSOM().getUniqueSOMID() == getUniqueSOMID()){
		if (belongsToThisSOM(vma)) {
			Edge<T> originalEdge = findEdge(vma);
			if (customer != null) {
				// first, attempt to lock all incident nodes:
				ManagedObjectReference<T> a = originalEdge.getNodeAReference(), b = originalEdge.getNodeBReference();
				if (a != null) {
					//make sure that the customer owns a copy of adjacent node a
					customer.getNode(a);
//					SpaceNode<T> copy = getCopyOfSpaceNode(a, customer);
//					// add incident nodes to the Cache
//					if (copy != null)
//						customer.cacheNode(copy);
				}
				if (b != null) {
					//make sure that the customer owns a copy of adjacent node b
					customer.getNode(b);
//					SpaceNode<T> copy = getCopyOfSpaceNode(b, customer);
//					// add incident nodes to the Cache
//					if (copy != null)
//						customer.cacheNode(copy);
				}
				
				// if that was successful, a copy of the requested Edge will be transmitted 
				// - follow the same scheme as for SpaceNode:
					// try to acquire a lock on the edge:
				// (The purpose of this call is more about making sure that no CacheManager can get 2 copies of the same object
				// than about locking the object. Real "locks" are needed for nodes only.)  
				CacheManager<T> copyOwner = originalEdge.lock(customer);
				// successful:
				if (copyOwner == null) {
					// create a local copy:
					return new Edge<T>(originalEdge);
				}
				// Edge was already owned by some CacheManager:
				else {
					// if that was the "customer" himself, return null so he NEVER gets two copies. (VERY BAD CASE!)
					if (copyOwner.equals(customer))
						return null;
					// if the edge was locked by someone else, something very unexpected happened because the incident nodes were available:
					else
						throw new RuntimeException("Attempt to create a local copy of an edge failed because it was locked. This should not have happen since none of the incident nodes was locked!");
				}
				
			}
			else {
				Edge<T> edge = new Edge<T>(originalEdge);
				edge.unlock();
				return edge;
			}
		}
		else return vma.getSOM().getCopyOfEdge(vma, customer);
	}
	
	/**
	 * @return the next unique address for ManagedObjects within this SOM 
	 * TODO - Some object should be responsible to assign a new address space to this SOM.
	 */
	//public synchronized long getUniqueAddress() {
	public long getUniqueAddress() {

		theLogger.finest("In SpatialOrganizationManager --- getUniqueAddress()        ......... current: " + current + ",  maxAddress:" + maxAddress);
		if (current < maxAddress){
			return current++;
		}
		else{theLogger.finest("This SpatialOrganizationManager exceeded it's amount of addresses!");
			throw new RuntimeException("This SpatialOrganizationManager exceeded it's amount of addresses!");
		}
	}
	
	/**
	 * Unlocks a node.
	 * @param ref The reference of the node that should be unlocked.
	 * @throws ManagedObjectDoesNotExistException 
	 */
	public void unlockNode(ManagedObjectReference<T> ref) throws ManagedObjectDoesNotExistException {
		SpaceNode<T> node = null;
		synchronized (nodes) {
			node = nodes.get(ref);
		}
		// this function may not use findNode since that function would wait for a node in pendingNodes.
		// But nodes that should be unlocked can never be in the list of pendingNodes. (and if they are, they are new!)
		if (node == null) {
			//if (ref.getSOM().getUniqueSOMID() == getUniqueSOMID())
			if (belongsToThisSOM(ref)) {
				throw new ManagedObjectDoesNotExistException(ref,"Reference to an unknown Tetrahedron!");
			}
			else 
				throw new RuntimeException("This function returns a link to the original object and may therefore only called from inside the proper SOM!");
		}
		else
			node.unlock();
	}
	
	/**
	 * Unlocks a tetrahedron.
	 * @param ref The reference of the tetrahedron that should be unlocked.
	 * @throws ManagedObjectDoesNotExistException 
	 */
	public void unlockTetrahedron(ManagedObjectReference<T> ref) throws ManagedObjectDoesNotExistException {
		findTetrahedron(ref).unlock();
	}
	
	/**
	 * Unlocks a triangle.
	 * @param ref The reference of the triangle that should be unlocked.
	 * @throws ManagedObjectDoesNotExistException 
	 */
	public void unlockTriangle(ManagedObjectReference<T> ref) throws ManagedObjectDoesNotExistException {
		findTriangle(ref).unlock();
	}
	
	/**
	 * Unlocks an edge.
	 * @param ref The reference of the edge that should be unlocked.
	 * @throws ManagedObjectDoesNotExistException 
	 */
	public void unlockEdge(ManagedObjectReference<T> ref) throws ManagedObjectDoesNotExistException {
		findEdge(ref).unlock();
	}
	
	/**
	 * @return the default SOM, the default Active Object host for the ManagedObjects
	 */
	public static<T> SpatialOrganizationManager<T> getDefaultSOM(){
		//TODO Some sort of ProActive code here to find the default ActiveObject
		return defaultSOM;
	}
	
	
	
	/**
	 * Attempts to commit a node. The old version of the node is removed from 
	 * the hashtable and the new version is put into it. 
	 * The CacheManager that is trying to commit this object has to own a 
	 * lock for the original object (unless there doesn't exist an old version).
	 * Otherwise, a RuntimeException is thrown. 
	 * @param transaction The CacheManager that is trying to commit the specified object.
	 * @param obj The node that is being committed.
	 */
	public void commitNode(CacheManager<T> transaction, SpaceNode<T> obj) {
		ManagedObjectReference<T> ref = obj.getReference();
		//if (ref.getSOM().getUniqueSOMID() == getUniqueSOMID()) {
		if (belongsToThisSOM(ref)) {
			synchronized (nodes) {
				SpaceNode<T> original = nodes.get(ref);
				if ((original == null) || (original.isLockedBy(transaction))) {
					if (original != null)
						nodes.remove(ref);
					if (obj.isValid())
						nodes.put(ref, obj);
				}
				else
					throw new RuntimeException("A CacheManager is trying to overwrite a ManagedObject for which it doesn't own a lock!");
			}
		}
		else ref.getSOM().commitNode(transaction, obj); 
	}

	
	/**
	 * Attempts to commit a tetrahedron. The old version of the tetrahedron is removed from 
	 * the hashtable and the new version is put into it. 
	 * The CacheManager that is trying to commit this object has to own a 
	 * lock for the original object (unless there doesn't exist an old version).
	 * Otherwise, a RuntimeException is thrown. 
	 * @param transaction The CacheManager that is trying to commit the specified object.
	 * @param obj The tetrahedron that is being committed.
	 */
	public void commitTetrahedron(CacheManager<T> transaction, Tetrahedron<T> obj) {
		ManagedObjectReference<T> ref = obj.getReference();
		//if (ref.getSOM().getUniqueSOMID() == getUniqueSOMID()) {
		if (belongsToThisSOM(ref)) {
			synchronized (tetrahedra) {
				Tetrahedron<T> original = tetrahedra.get(ref);
				// It's ok if the object either did not exist or it was locked by the transaction that is trying to commit it...
				if ((original == null) || (original.isLockedBy(transaction))) {
					// it did exist => remove the old copy!
					if (original != null)
						tetrahedra.remove(ref);
					// it's valid => put a new one!
					if (obj.isValid())
						tetrahedra.put(ref, obj);
				}
				else
					throw new RuntimeException("A CacheManager is trying to overwrite a ManagedObject for which it doesn't own a lock!");
			}
		}
		else ref.getSOM().commitTetrahedron(transaction, obj); 
	}
	
	/**
	 * Attempts to commit a triangle. The old version of the triangle is removed from 
	 * the hashtable and the new version is put into it. 
	 * The CacheManager that is trying to commit this object has to own a 
	 * lock for the original object (unless there doesn't exist an old version).
	 * Otherwise, a RuntimeException is thrown. 
	 * @param transaction The CacheManager that is trying to commit the specified object.
	 * @param obj The triangle that is being committed.
	 */
	public void commitTriangle(CacheManager<T> transaction, Triangle3D<T> obj) {
		ManagedObjectReference<T> ref = obj.getReference();
		//if (ref.getSOM().getUniqueSOMID() == getUniqueSOMID()) {
		if (belongsToThisSOM(ref)) {
			synchronized (triangles) {
				Triangle3D<T> original = triangles.get(ref);
				if ((original == null) || (original.isLockedBy(transaction))) {
					if (original != null)
						triangles.remove(ref);
					if (obj.isValid())
						triangles.put(ref, obj);
				}
				else {
//					synchronized (stopLock) {
					throw new RuntimeException("A CacheManager is trying to overwrite a ManagedObject for which it doesn't own a lock!");
//					}
				}
			}
		}
		else ref.getSOM().commitTriangle(transaction, obj); 
	}
	
	/**
	 * Attempts to commit an edge. The old version of the edge is removed from 
	 * the hashtable and the new version is put into it. 
	 * The CacheManager that is trying to commit this object has to own a 
	 * lock for the original object (unless there doesn't exist an old version).
	 * Otherwise, a RuntimeException is thrown. 
	 * @param transaction The CacheManager that is trying to commit the specified object.
	 * @param obj The edge that is being committed.
	 */
	public void commitEdge(CacheManager<T> transaction, Edge<T> obj) {
		theLogger.finest("In SOM --- commitEdge");
		ManagedObjectReference<T> ref = obj.getReference();
		//if (ref.getSOM().getUniqueSOMID() == getUniqueSOMID()) {
		if (belongsToThisSOM(ref)) {
			synchronized (edges) {
				Edge<T> original = edges.get(ref);
				if ((original == null) || (original.isLockedBy(transaction))) {
					if (original != null)
						edges.remove(ref);
					if (obj.isValid())
						edges.put(ref, obj);
				}
				else
					throw new RuntimeException("A CacheManager is trying to overwrite a ManagedObject for which it doesn't own a lock!");
			}
		}
		else ref.getSOM().commitEdge(transaction, obj); 
	}
	
	
	
//	/**
//	 * Attempts to write the pending changes in to the SOM
//	 * @param transaction, the CacheManager that wants to commit its changes
//	 * @param obj, the ManagedObject to commit
//	 * @return true if the ManagedObject was successfully commited
//	 * @deprecated Use commitNode etc. instead! This function doesn't work! 
//	 */
//	public boolean commit(CacheManager transaction, ManagedObject obj){ 
//		ManagedObjectReference ref = obj.getReference();
//		ManagedObject master = getObject(ref);
//		if(master == null || master.getReference().getLockingCacheManager() != transaction){
//			throw new RuntimeException("Unexpected Exception:: The SOM copy of this node did not exist or was locked by another transaction");
//		}
//		
//		//Unlock the cache manager copy and overwrite the SOM master with the unlocked cachemanager copy
//		if(master instanceof SpaceNode){
//			obj.unlock();
//			//nodes.remove(ref);
//			nodes.put(ref, (SpaceNode)obj);
//			return true;
//		}
//		if(master instanceof Triangle3D){
//			obj.unlock();
//			//triangles.remove(ref);
//			triangles.put(ref, (Triangle3D)obj);
//			return true;
//		}
//		if(master instanceof Tetrahedron){
//			obj.unlock();
//			//tetrahedra.remove(ref);
//			tetrahedra.put(ref, (Tetrahedron)obj);
//			return true;
//		}
//		if(master instanceof Edge){
//			obj.unlock();
//			//edges.remove(ref);
//			edges.put(ref, (Edge)obj);
//			return true;
//		}
//		return false;
//	}
//	
//	
//	/*
//	 * Get the SOM copy of the specified object
//	 * @param obj, the ManagedObject to commit
//	 * @return the master copy of the ManagedObjectRef, or NULL if this SOM
//	 * does not own it
//	 */
//	private ManagedObject getObject(ManagedObjectReference obj ){
//		ManagedObject toReturn = null;
//		toReturn = nodes.get(obj);
//		if(null == toReturn){
//			toReturn = tetrahedra.get(obj);
//		}
//		if(null == toReturn){
//			toReturn = triangles.get(obj);
//		}
//		if(null == toReturn){
//			toReturn = edges.get(obj);
//		}
//		return toReturn;
//	}
	
	
	/**
	 * Return the position of the node.
	 * @return the position of the SpaceNode
	 * @throws ManagedObjectDoesNotExistException 
	 * @see ini.cx3d.spatialOrganization.SpatialOrganizationNode#getPosition()
	 */
	public double[] getNodePosition(ManagedObjectReference<T> nodeReference) throws ManagedObjectDoesNotExistException {
		//if (!nodeReference.getSOM().getUniqueSOMID()equals(getThis())) 
		//if (nodeReference.getSOM().getUniqueSOMID() != getUniqueSOMID()){
		if (!belongsToThisSOM(nodeReference)) {
			return nodeReference.getSOM().getNodePosition(nodeReference);
		}
		else {
			waitUntilReliable();
			return findNode(nodeReference).getPosition();
		}
// This following procedure is not necessary! It's much easier to access the original object here:
//		if (!nodeReference.getSOM().equals(this)) 
//			return nodeReference.getSOM().getNodePosition(nodeReference);
//		else {
//			SpaceNode<T> local = nodes.get(nodeReference);
//			if (local == null) {
//				throw new RuntimeException("Trying to access the position of an unknown node!");
//			}
//	
//			
//			//SpaceNode<T> local = null;
//			
//			try{
//				local = getCopyOfSpaceNode(nodeReference, null);
//				return local.getPosition();
//			}catch (ManagedObjectDoesNotExistException e) {
//				e.printStackTrace();
//				throw new RuntimeException("Unexpected exception: a node used in the simulation was" + 
//						"not recognised by the SOM");	
//			} catch (NodeLockedException nle) {
//				// Should not be thrown as we called getCopyOfSpaceNode without a CacheManager
//				nle.printStackTrace();
//				throw new RuntimeException("Unexpected exception: a read only process (getPosition) is not" + 
//						"transactional, does not use a chache manager and should not produce" + 
//						"NodeLockedExceptions.");	
//			} 
//		}
	}
	
	public double getNodeVolume(ManagedObjectReference<T> nodeReference) throws ManagedObjectDoesNotExistException {
		//if (!nodeReference.getSOM().equals(getThis())) {
		//if (nodeReference.getSOM().getUniqueSOMID() != getUniqueSOMID()){
		if (!belongsToThisSOM(nodeReference)) {
			return nodeReference.getSOM().getNodeVolume(nodeReference);
		}
		else {
			waitUntilReliable();
			return findNode(nodeReference).getVolume();
		}
	}
	
	public T getNodeUserObject(ManagedObjectReference<T> nodeReference) throws ManagedObjectDoesNotExistException {
		//if (nodeReference.getSOM().getUniqueSOMID() != getUniqueSOMID()){
		//if (!nodeReference.getSOM().equals(getThis())) {
		if (!belongsToThisSOM(nodeReference)) {
			return nodeReference.getSOM().getNodeUserObject(nodeReference);
		}
		else {
//			waitUntilReliable();
			return findNode(nodeReference).getUserObject();
		}
	}

	public void addAction(CacheManager<T> action) {
		synchronized (actionQueue) {

			theLogger.finer("In SpatialOrganizationManager --- addAction()");
			this.actionQueue.addAction(action);
			// The following line should open a huge memory leak 
			// this.allActions.add(action);
		}
		checkActionQueue();
	}

	protected void addInsertionActionAtTetrahedron(T userObject, double [] coordinate, ManagedObjectReference<T> preliminaryNodeReference, ManagedObjectReference<T> startingTetrahedron) {
		// run a stochastic visibility walk:
		ManagedObjectReference<T> insertionSite = visibilityWalk(coordinate, startingTetrahedron);
		if (!belongsToThisSOM(insertionSite))
			// this call should be ProActive-ok, since it is the last call and no one is waiting for any result.
			// therefore, the current SOM will no longer be blocked.
			insertionSite.getSOM().addInsertionActionAtTetrahedron(userObject, coordinate, preliminaryNodeReference, insertionSite);
		else {
			// now that you found an appropriate position to insert the tetrahedron: add the action to the queue!
			// instantiate a new queueableAction:
			CacheManager<T> insertionAction = new NodeCreationCM<T>(userObject, coordinate, insertionSite, preliminaryNodeReference, insertionSite.getSOM());
			addAction(insertionAction);
		}
	}
	
	public ManagedObjectReference<T> createPendingNodeReference() {
		ManagedObjectReference<T> preliminaryNodeReference = new ManagedObjectReference<T>(getUniqueAddress(),this);
		addPendingNode(preliminaryNodeReference);
		return preliminaryNodeReference;
	}
	
	public ManagedObjectReference<T> addInsertionAction(T userObject, double [] coordinate, ManagedObjectReference<T> closebyNode) {
		
		theLogger.finest("In SpatialOrganizationManager --- addInsertionAction(b)");
		// create a MOR for the node that will be inserted:
		SpatialOrganizationManager<T> targetSOM = this.nodeToSOMAssignmentPolicy.getResponsibleSOM(coordinate);
		
		ManagedObjectReference<T> preliminaryNodeReference = targetSOM.createPendingNodeReference();
		addInsertionAction(userObject, coordinate, closebyNode, preliminaryNodeReference);
		return preliminaryNodeReference;
	}
	
	private void addInsertionAction(T userObject, double [] coordinate, ManagedObjectReference<T> closebyNode, ManagedObjectReference<T> preliminaryNodeReference) {

		theLogger.finest("In SpatialOrganizationManager --- addInsertionAction(c)");
		SpatialOrganizationManager <T> tempSOM = closebyNode.getSOM();
		

		theLogger.finer("This  SOM:" + getUniqueSOMID());
		theLogger.finer("Other SOM:" + tempSOM.getUniqueSOMID());
		
		//if ( tempSOM.getUniqueSOMID() != getUniqueSOMID()) {
		if ( !belongsToThisSOM(closebyNode)) {
			theLogger.info("Forign SOM - call addInsertionAction on remote SOM");
			closebyNode.getSOM().addInsertionAction(userObject, coordinate, closebyNode, preliminaryNodeReference);
		}
		else {

			theLogger.finer("This SOM - Do stuff");
			// first, get a local copy of the specified node and get any other if that is not possible:
			SpaceNode<T> startNode = null;
			theLogger.finest("addInsertionAction .... 1");
			while (startNode == null) {
				theLogger.finest("addInsertionAction .... looping... ");
				try {
					theLogger.finest("addInsertionAction .... 2           --- before getCopyOfSpacenode");
					startNode = getCopyOfSpaceNode(closebyNode, null);
					theLogger.finest("addInsertionAction .... 3");
				} catch (NodeLockedException e2) {
					throw new RuntimeException("Error! Received a NodeLockedException while trying to obtain a local copy from a non-modifying process (visibility walk)!");
				} catch (ManagedObjectDoesNotExistException e2) {
					synchronized (nodes) {
						if (nodes.isEmpty())
							// TODO: Stop using defaultSOM.
							((SpatialOrganizationManager<T>)this.getDefaultSOM()).addInsertionAction(userObject, coordinate, (ManagedObjectReference<T>)(getDefaultSOM().getANodeReference()), preliminaryNodeReference);
	//						throw new RuntimeException("Bad case happened: Attempt to insert a node at a SpatialOrganizationManager that doesn't contain any Tetrahedra!");
						else
							closebyNode = getANodeReference();

					}
				}
			}

			theLogger.finest("addInsertionAction .... 4                --- before get new ManagedObjectReference");

			theLogger.finest("addInsertionAction .... 5                --- before getFirstAdjacentTetrahedronReference");
						
			ManagedObjectReference<T> tetrahedronReference = startNode.getFirstAdjacentTetrahedronReference();

			theLogger.finest("addInsertionAction .... 6                --- before addPendingNode");
						
			
			theLogger.finest("addInsertionAction .... 7                --- before null check");
			if (tetrahedronReference == null) {
				// apparently, there are less than four nodes..
				// now apply the old technique, don't bother about searching the correct insertion tetrahedron.

				theLogger.finest("addInsertionAction .... 8                --- before new InitialNodeCreationCM(...)");
				CacheManager<T> cm = new InitialNodeCreationCM<T>(userObject, coordinate, closebyNode, preliminaryNodeReference,startNode.getSOM());

				theLogger.finest("addInsertionAction .... 9                --- before startNode.getSOM().addAction");
				startNode.getSOM().addAction(cm);
			}
			else {
				theLogger.finest("addInsertionAction .... 8a                --- before addInsertionActionAtTetrahedron");

				addInsertionActionAtTetrahedron(userObject, coordinate, preliminaryNodeReference, tetrahedronReference);
			}
		}
	}

	protected void addMovementAction(ManagedObjectReference<T> nodeReference, double [] delta) {
		addAction(new NodeMovementCM<T>(nodeReference, delta, getThis()));
	}
	
	/**
	 * This function will perform a stochastic visibility walk towards a given coordinate.
	 * The returned reference will be a non-reliable reference to the tetrahedron that surrounds the given coordinate.
	 * @param coordinate The coordinate to search for.
	 * @param startingTetrahedron Any tetrahedron from which the search is started.
	 * @return A reference to a tetrahedron. Be aware that this reference might be out of date once you use it.
	 */
	protected ManagedObjectReference<T> visibilityWalk(double [] coordinate, ManagedObjectReference<T> startingTetrahedron) {
		if (!belongsToThisSOM(startingTetrahedron)) {
			// return the same reference -> the caller must notice that this reference is pointing
			// to a different SOM and call visibilitywalk on that SOM.
			return startingTetrahedron;
		//if (startingTetrahedron.getSOM().getUniqueSOMID() != getUniqueSOMID()) {
//			return startingTetrahedron.getSOM().visibilityWalk(coordinate, startingTetrahedron);
		}
		else {
			startingTetrahedron = getATetrahedronReference();
			// perform a stochastic visibility walk to the insertion position:
			while (true) {
				// first, try to get a local copy of the specified startingTetrahedron
				// if the reference is not valid any more, pick any tetrahedron!
				Tetrahedron<T> current = null;
				while (current == null) {
					try {
						current = getCopyOfTetrahedron(startingTetrahedron, null); 
					} catch (ManagedObjectDoesNotExistException e1) {
						synchronized (tetrahedra) {
							if (tetrahedra.isEmpty())
								throw new RuntimeException("Bad case happened: Attempt to insert a node at a SpatialOrganizationManager that doesn't contain any Tetrahedra!");
							startingTetrahedron = getATetrahedronReference();
						}
					} catch (NodeLockedException e) {
						throw new RuntimeException("Error! Received a NodeLockedException while trying to obtain a local copy from a non-modifying process (visibility walk)!");
					}
				}
				// now that we have a copy of at least one tetrahedron...
				// perform a stochastic visibility walk
				try {
					if (current.isInfinite()) {
						try {
							current = current.getOppositeTriangle(null).getOppositeTetrahedron(current);
						} catch (NodeLockedException e) {
							throw new RuntimeException("Error! Received a NodeLockedException while trying to obtain a local copy from a non-modifying process (visibility walk)!");
						} catch (UnknownManagedObjectException e) {
							return visibilityWalk(coordinate, startingTetrahedron);
						}
					}
					Tetrahedron<T> last = null;
					while ((current != last) && (!current.isInfinite())) {
						//if (current.getSOM().getUniqueSOMID() != getUniqueSOMID()) {
						if (!belongsToThisSOM(current.getReference())) {
							// can't continue... the caller should redirect the request to another SOM
							return current.getReference();
//							return current.getSOM().visibilityWalk(coordinate, current.getReference());
						}
						last = current;
						try {
							current = current.walkToPoint(coordinate);
							if (current == null)
								theLogger.warning("ups... walkToPoint returned null!");
						} catch (PositionNotAllowedException e) {
							// TODO Find an elegant alternative to PositionNotAllowedExceptions!
						} catch (NodeLockedException e) {
							throw new RuntimeException("Error! Received a NodeLockedException while trying to obtain a local copy from a non-modifying process (visibility walk)!");
						} catch (UnknownManagedObjectException e) {
							// probably, some object was removed while you were trying to walk further...
							// this call should be okay from a ProActive-point-of-view, since we don't queue up again...
							return visibilityWalk(coordinate, startingTetrahedron);
						}
					}

					return current.getReference();
					
				} catch (ManagedObjectDoesNotExistException e) {
					// try again... first, pick a random tetrahedron...
					startingTetrahedron = getATetrahedronReference();
				}
			}
		}
	}
	
	public String toString() {
		return "SOM ["+this.current+"-"+this.maxAddress+"]\n"+nodes+"\n"+edges+"\n"+triangles+"\n"+tetrahedra;
	}
	
	public void printStats() {
		theLogger.info("non-finalized processes: "+this.nonFinalizedProcesses);
		theLogger.info("Amount of nodes: "+nodes.size());
		theLogger.info("Amount of edges: "+edges.size());
		theLogger.info("Amount of triangles: "+triangles.size());
		theLogger.info("Amount of tetrahedra: "+tetrahedra.size());
		theLogger.info("Actions waiting: "+actionQueue.getSize());
		theLogger.info("Nodes pending: "+pendingNodes.size());
		if (nonFinalizedProcesses > 2000) {
//			synchronized (stopLock) {
				theLogger.warning("please debug here...");
//			}
		}
//		synchronized (nodes) {
//			for (SpaceNode<T> node : nodes.values()) {
//				if (node.tracker.getLockingCacheManager() != null)
//					theLogger.error("a locked node was found!");
//			}
//		}
//		synchronized (tetrahedra) {
//			for (Tetrahedron<T> tetrahedron : tetrahedra.values()) {
//				if (tetrahedron.tracker.getLockingCacheManager() != null)
//					theLogger.error("a locked tetrahedron was found!");
//			}
//			
//		}
//		synchronized (triangles) {
//			for (Triangle3D<T> triangle : triangles.values()) {
//				if (triangle.tracker.getLockingCacheManager() != null)
//					theLogger.error("a locked triangle was found!");
//			}
//			
//		}
//		synchronized (edges) {
//			for (Edge<T> edge : edges.values()) {
//				if (edge.tracker.getLockingCacheManager() != null)
//					theLogger.error("a locked edge was found!");
//			}
//		}
	}
	
	
//	/**
//	 * Starts a modifying process that will perform a move of a specified node.
//	 * @param theNode node to be moved
//	 * @param delta The coordinates this node should move to.
//	 */
//	public void moveNodeFrom(ManagedObjectReference<T> nodeReference, double[] delta) throws PositionNotAllowedException{
//		// TODO: Move this function into CacheManager 
//		// create a new CacheManager:
//		//Note: this uses the same unique address as the managed objects
//		CacheManager transactionCache = new CacheManager(getUniqueAddress());
//		
//		// get a local copy of the SpaceNode that should be moved:
//		SpaceNode localNode = null;
//		try {
//			localNode = transactionCache.getNode(nodeReference);
//			try {
//				localNode.moveFrom(delta);
//			} catch (ManagedObjectDoesNotExistException e) {
//				throw new RuntimeException("A modifying process tried to access a managed object that didn't exist!");
//			}
//			transactionCache.commit();
//		} catch (NodeLockedException e1) {
//			transactionCache.rollBack();
//			this.addAction(new NodeMovement<T>(nodeReference, delta));
//		} 
//	}
//
//	
//	/**
//	 * Launches a modifying process that creates a new node that is assigned a given nodeReference.
//	 * @param userObject
//	 * @param coordinate
//	 * @param startingTetrahedron
//	 * @param nodeReference
//	 */
//	public void insertNode(T userObject, double[] coordinate, ManagedObjectReference<T> startingTetrahedron, ManagedObjectReference<T> nodeReference) {
//		// perform a stochastic visibility walk to the insertion position:
//		CacheManager<T> cm = new CacheManager<T>(getUniqueAddress());
//		try {
//			Tetrahedron<T> insertionSite = null;
//			while (insertionSite == null) {
//				try {
//					// TODO: It'd be better if a good starting point could be searched in a non-modifying process! the current version will lock all objects along a stochastic visibility walk trail.
//					
//					insertionSite = cm.tryToGetTetrahedron(startingTetrahedron);
//				} catch (ManagedObjectDoesNotExistException e) {
//					insertionSite = null;
//					// TODO: what a nasty line of code:
//					startingTetrahedron = startingTetrahedron.getSOM().tetrahedra.keys().nextElement();
//				}
//			}
//			// now, cm should own a locked copy of a starting point...
//			SpaceNode<T> newNode = new SpaceNode<T>(coordinate, userObject, nodeReference);
//			try {
//				newNode.insert(insertionSite);
//			} catch (PositionNotAllowedException e) {
//				// TODO: very annoying... need a new treatment for PositionNotAllowed-cases
//			} catch (ManagedObjectDoesNotExistException e) {
//				throw new RuntimeException("A modifying process tried to access a managed object that didn't exist!");
//			}
//			cm.commit();
//		} catch (NodeLockedException e) {
//			// it didn't work out...
//			
//			// undo the locking:
//			cm.rollBack();
//			
//			// and re-queue an insertion action:
//			this.addInsertionActionAtTetrahedron(userObject, coordinate, startingTetrahedron, this);
//		}
//		
//	}
	
	/**
	 * Returns the Unique ID of this SOM
	 * @return A reference to a tetrahedron. Be aware that this reference might be out of date once you use it.
	 */
	public Long getUniqueSOMID(){
		return myUniqueID;
	}
	
	/**
	 * Returns a reference to this SOM - wrapper for the Java "this"
	 * @return this SpatialOrganizationManager
	 */
	protected SpatialOrganizationManager<T> getThis(){

		theLogger.finest("In SOM --- getThis()");
		return this;
	}
	
	/**
	 * Returns true if the SOM passed in is the reference to this SOM - wrapper for the Java "this"
	 * @return this SpatialOrganizationManager
	 */
	protected Boolean belongsToThisSOM( ManagedObjectReference<T> magedObj ){
		theLogger.finest("In SOM --- belongsToThisSOM()");
		if(getUniqueSOMID().equals( magedObj.getSOM().getUniqueSOMID() )){
			return Boolean.TRUE;
		}
		else{
			return  Boolean.FALSE;
		}
	}

}