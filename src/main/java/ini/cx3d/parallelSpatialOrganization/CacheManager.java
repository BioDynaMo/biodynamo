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

import java.io.Serializable;
import java.util.Hashtable;
import java.util.LinkedList;

/**
 *
 * @author Toby Weston
 * @version 0.1
 * @Date: 13/6/2008
 *
 * Responsibe for caching local copies of objects owned by  remote SpatialOrganisationManager objects.
 */


public abstract class CacheManager<T> implements Serializable{
	//Set this to false to imporove eficiency
	boolean debug = true;
	
    Hashtable<ManagedObjectReference<T>, SpaceNode<T>> nodes;
    Hashtable<ManagedObjectReference<T>, Tetrahedron<T>> tetrahedra;
    Hashtable<ManagedObjectReference<T>, Triangle3D<T>> triangles;
    Hashtable<ManagedObjectReference<T>, Edge<T>> edges;
    SpatialOrganizationManager<T> mySOM;
    long id;
    
    /**
     * Creates a new CacheManager. 
     * @param id The ID-value of this CacheManager which will be used to compare the priorities
     * of two processes.
     */
    public CacheManager(SpatialOrganizationManager<T> mySOM) {
    	if (mySOM != null)
    		this.id = mySOM.getUniqueAddress();
    	else this.id = 0;
    	// TODO: (Long long time from now) Find out what would be appropriate sizes to initialize
    	// the HashTables.
    	this.nodes = null;
    	this.tetrahedra = null;
    	this.triangles = null;
    	this.edges = null;
    	this.mySOM = mySOM;
    }
    
    public abstract String descriptionString();
    
    private void init() {
    	this.nodes = new Hashtable<ManagedObjectReference<T>, SpaceNode<T>>();
    	this.tetrahedra = new Hashtable<ManagedObjectReference<T>, Tetrahedron<T>>();
    	this.triangles = new Hashtable<ManagedObjectReference<T>, Triangle3D<T>>();
    	this.edges = new Hashtable<ManagedObjectReference<T>, Edge<T>>();
    }
    
    private void clean() {
    	this.nodes = null;
    	this.tetrahedra = null;
    	this.triangles = null;
    	this.edges = null;
    }
    
    /**
     * Performs the task associated with this {@link CacheManager}.  
     * @return <code>true</code>, if the task was successfully finished and <code>false</code> otherwise. 
     * @throws NodeLockedException If the modifying process tried to access a locked node.
     */
    abstract boolean executeTask() throws NodeLockedException;
    
    /**
     * This function is executed from run() after a task has finished successfully. Any implementation of {@link CacheManager} can now
     * perform finishing steps. (I.e. inform the SOM that a certain node is now integrated.)  
     */
    protected void jobSuccessful() {
    }

    /**
     * Returns the priority of this process.
     * @return a low value, if the priority is high and a high value, if the priority is low. 
     */
    public abstract int getPriority();
    
    
    public void run() {
    	init();
    	boolean output = false;
    	boolean success = false;
    	long reason = 0l;
		try {
			if (output)
				System.out.println(id+": Starting process "+descriptionString());
			success = executeTask();
		} catch (NodeLockedException e) {
			// nothing to do, success is false => this will cause a rollback.
			reason = e.getLockingCacheManagerIndex();
		}
		if (success) {
			try {
				commit();
				if (output)
					System.out.println(id+": Process "+descriptionString()+" finished successfully!");
				jobSuccessful();
				clean();
			} catch (NodeLockedException e) {
				e.printStackTrace();
				throw new RuntimeException("Error: commit function threw a NodeLockedException! This should never occur!");
			}
		}
		else {
			if (output)
				System.out.println(id+": Process "+descriptionString()+" has to roll back (Reason: Process #"+reason+")!");
			rollBack();
		}
//		if (!mySOM.checkTriangulation()) {
//			System.out.println(id+": The triangulation just became corrupt!");
//		}
		mySOM.activeProcessHasFinished();
    }
    
//    /**
//     * Adds a local copy of a node to the cache. This procedure is necessary because all
//     * nodes that are incident to any stored ManagedObject have to cached. (Because they will
//     * also be locked. 
//     * @param node The node that will be added to this Cache.
//     * @deprecated Use {@link CacheManager#getNode(ManagedObjectReference)} instead.
//     */
//    public void cacheNode(SpaceNode<T> node) {
//    	this.nodes.put(node.getReference(), node);
//    }
    
    /**
     * This function attempts to put a Tetrahedron into the cache. The given reference is considered to be unreliable, meaning that
     * the object could already be removed at runtime. 
     * Therefore, this function has the same functionality as {@link CacheManager#getTetrahedron(ManagedObjectReference)}, but this function might also throw a ManagedObjectReferenceDoesNotExistException.
     * @param ref The unreliable reference to the desired Tetrahedron.
     * @return A local copy of the desired tetrahedron.
     * @throws ManagedObjectDoesNotExistException if the tetrahedron did not exist any more.
     * @throws NodeLockedException if the tetrahedron or any of its incident nodes were locked by another modifiying process.
     */
    public Tetrahedron<T> tryToGetTetrahedron(ManagedObjectReference<T> ref) throws NodeLockedException, ManagedObjectDoesNotExistException {
    	Tetrahedron<T> tet = tetrahedra.get(ref);
    	if (tet != null) {
    		return tet;
		} else {
			Tetrahedron<T> fetchedTetrahedron = ref.getSOM().getCopyOfTetrahedron(ref, this);
			tetrahedra.put(ref, fetchedTetrahedron);
			return fetchedTetrahedron;
		} 
    }

    /**
     * This function attempts to put a Node into the cache. The given reference is considered to be unreliable, meaning that
     * the object could already be removed at runtime. 
     * Therefore, this function has the same functionality as {@link CacheManager#getTetrahedron(ManagedObjectReference)}, but this function might also throw a ManagedObjectReferenceDoesNotExistException.
     * @param ref The unreliable reference to the desired Node.
     * @return A local copy of the desired node.
     * @throws ManagedObjectDoesNotExistException if the node did not exist any more.
     * @throws NodeLockedException if the node or any of its incident nodes were locked by another modifying process.
     */
    public SpaceNode<T> tryToGetNode(ManagedObjectReference<T> ref) throws NodeLockedException, ManagedObjectDoesNotExistException {
    	SpaceNode<T> node = nodes.get(ref);
    	if (node != null) {
    		return node;
		} else {
			SpaceNode<T> fetchedNode = ref.getSOM().getCopyOfSpaceNode(ref, this);
			nodes.put(ref, fetchedNode);
			return fetchedNode;
		} 
    }
    
    
    /**
     * Get the node identified by ref from the local cache or get a copy of the spacenode from the
     * SOM and add it to the cache.
     * @param ref global reference to the requested Node
     * TODO Look at this again, regarding remote locking of ManagedObjects
     */
    public SpaceNode<T> getNode(ManagedObjectReference<T> ref) throws NodeLockedException {
    	SpaceNode<T> node = nodes.get(ref);
   	 	if(node != null){	   	 		
       	 	return node;
       	}
       	else{
       		try {
				SpaceNode<T> fetchedNode = ref.getSOM().getCopyOfSpaceNode(ref, this);
				nodes.put(ref, fetchedNode);
				return fetchedNode;
			} catch (ManagedObjectDoesNotExistException e) {
				throw new RuntimeException("A modifying process tried " +
						"to access a non-existing Node! The locking policy " +
						"should make sure that this can never occur!");			
			}
       	}      		
    }
    
    
//    /**
//     * Add the node identified by ref to the local cache
//     * @param ref global reference to the Node to be added
//     * @return the copy of the node which was added to the Cache
//	 * @deprecated Creation of a new ManagedObject in CacheManager is inconsistent. 
//     */
//    public SpaceNode<T> putNode(SpaceNode<T> node){
//    		ManagedObjectReference ref = node.getReference();
//        if(!nodes.containsKey(ref)){
//
//        	   SpaceNode copy = new SpaceNode(node);
//            nodes.put(ref, copy);
//            return copy;
//        }
//        else{
//            return null;
//        }
//    }
    
    /**
     * Does the cache contain this node
     * @param ref global reference to the Node to be added
     */
    public Boolean containsNode(ManagedObjectReference<T> ref) {
	 	if(!nodes.containsKey(ref)){	   	 		
    	 		return new Boolean(false);
	    	}
	    	else{		
    	 		return new Boolean(true);
	    	}      		
    }
    
    /**
     * Get the Tetrahedron identified by ref to the local cache
     * @param ref global reference to the rerquested Tetrahedron
     * TODO Look at this again, regarding remote locking of ManagedObjects
     */
    public Tetrahedron<T> getTetrahedron(ManagedObjectReference<T> ref) throws NodeLockedException {
    	Tetrahedron<T> tet = tetrahedra.get(ref);
    	if (tet != null) {
    		return tet;
		} else {
			try {
				Tetrahedron<T> fetchedTetrahedron = ref.getSOM().getCopyOfTetrahedron(ref, this);
				tetrahedra.put(ref, fetchedTetrahedron);
				return fetchedTetrahedron;
			} catch (ManagedObjectDoesNotExistException e) {
				throw new RuntimeException("A modifying process tried " +
						"to access a non-existing Tetrahedron! The locking policy " +
						"should make sure that this can never occur!");			
			}
		} 
    }
    
//     /**
//		 * Add the Tetrahedron identified by ref to the local cache
//		 * 
//		 * @param ref
//		 *            global reference to the Tetrahedron to be added
//		 * @deprecated Creation of a new ManagedObject in CacheManager is inconsistent. 
//		 */
//    public Tetrahedron<T> putTetrahedron(Tetrahedron<T> tet){
//		ManagedObjectReference ref = tet.getReference();
//        if(!tetrahedra.containsKey(ref)){
//
//        	   Tetrahedron copy = new Tetrahedron(tet);
//        	   tetrahedra.put(ref, copy);
//            return copy;
//        }
//        else{
//            return null;
//        } 
//     }
    
    /**
     * Does the cache contain this Tetrahedron
     * @param ref global reference to the Tetrahedron to be added
     */
    public Boolean containsTetrahedron(ManagedObjectReference<T> ref) {
	 	if (!tetrahedra.containsKey(ref)) {
			return new Boolean(false);
		} else {
			return new Boolean(true);
		}      		
    }
    
    
    /**
	 * Get the Triangle3D identified by ref to the local cache
	 * 
	 * @param ref
	 *            global reference to the rerquested Triangle3D 
	 * TODO Look at this again, regarding remote locking of ManagedObjects
	 */
    public Triangle3D<T> getTriangle(ManagedObjectReference<T> ref) throws NodeLockedException {
    	Triangle3D<T> triangle = triangles.get(ref);
    	if (triangle != null) { 
    		return triangle;
		} else {
			try {
				Triangle3D<T> fetchedTriangle = ref.getSOM().getCopyOfTriangle(ref, this);
				triangles.put(ref, fetchedTriangle);
				return fetchedTriangle;
			} catch (ManagedObjectDoesNotExistException e) {
				throw new RuntimeException("A modifying process tried " +
						"to access a non-existing Triangle! The locking policy " +
						"should make sure that this can never occur!");			
			}
				
		}
	}
    
    
//     /**
//		 * Add the Triangle identified by ref to the local cache
//		 * 
//		 * @param ref
//		 *            global reference to the Triangle to be added
//		 * @deprecated Creation of a new ManagedObject in CacheManager is inconsistent. 
//		 */
//    private Triangle3D<T> putTriangle(Triangle3D<T> triangle){
//        ManagedObjectReference ref = triangle.getReference();
//        if(!triangles.containsKey(ref)){
//        	   Triangle3D copy = new Triangle3D(triangle);
//        	   triangles.put(ref, copy);
//            return copy;
//        }
//        else{
//            return null;
//        }         
//     }
    
	boolean checkTriangles() {
		Object[] list = null;
		synchronized (triangles) {
			list = triangles.keySet().toArray();
		}				
		for (Object ref : list) {
			
			try {
				Triangle3D<T> triangle = this.getTriangle((ManagedObjectReference<T>)ref);
				if (!triangle.checkConformity())
					return false;
			} catch (NodeLockedException e) {
			} catch (ManagedObjectDoesNotExistException e) {
			}
		}
		return true;
	}

    
    /**
     * Does the cache contain this Triangle
     * @param ref global reference to the Triangle to be added
     */
    public Boolean containsTriangle(ManagedObjectReference<T> ref){
	 	if(!triangles.containsKey(ref)){	   	 		
    	 		return new Boolean(false);
	    	}
	    	else{		
    	 		return new Boolean(true);
	    	}      		
    }
    
    /**
     * Get the Edge identified by ref to the local cache
     * @param ref global reference to the rerquested Edge
     * TODO Look at this again, regarding remote locking of ManagedObjects
     */
    public Edge<T> getEdge(ManagedObjectReference<T> ref) throws NodeLockedException {
    	Edge<T> edge = edges.get(ref);
    	if (edge != null) {
    		return edge;
		} else {
			try {
				Edge<T> fetchedEdge = ref.getSOM().getCopyOfEdge(ref, this);
				edges.put(ref,fetchedEdge); 
				return fetchedEdge;
			} catch (ManagedObjectDoesNotExistException e) {
				throw new RuntimeException("A modifying process tried " +
						"to access a non-existing Edge! The locking policy " +
						"should make sure that this can never occur!");			
			}
		}

	}
    
//     /**
//		 * Add the Edge identified by ref to the local cache
//		 * 
//		 * @param ref
//		 *            global reference to the Edge to be added
//		 * @deprecated Creation of a new ManagedObject in CacheManager is inconsistent. 
//		 */
//    private Edge<T> putEdge(Edge<T> edge){
//        ManagedObjectReference ref = edge.getReference();
//        if(!edges.containsKey(ref)){
//        	   Edge copy = new Edge(edge);
//        	   edges.put(ref, copy);
//            return copy;
//        }
//        else{
//            return null;
//        }  
//     } 
   
    /**
     * Does the cache contain this Edge
     * @param ref global reference to the Node to be added
     */
    public Boolean containsEdge(ManagedObjectReference<T> ref){
	 	if(!edges.containsKey(ref)){	   	 		
    	 		return new Boolean(false);
	    	}
	    	else{		
    	 		return new Boolean(true);
	    	}      		
    }
    
    public void registerNewManagedObject(ManagedObjectReference<T> reference, ManagedObject<T> mo) {
    	if (mo instanceof SpaceNode) 
    		nodes.put(mo.getReference(), (SpaceNode<T>)mo);
    	if (mo instanceof Tetrahedron) 
    		tetrahedra.put(mo.getReference(), (Tetrahedron<T>)mo);
    	if (mo instanceof Edge) 
    		edges.put(mo.getReference(), (Edge<T>)mo);
    	if (mo instanceof Triangle3D) 
    		triangles.put(mo.getReference(), (Triangle3D<T>)mo);
    }
    
	public void registerNewNode(SpaceNode<T> node) {
		this.nodes.put(node.getReference(), node);
	}
	
	public void registerNewTetrahedron(Tetrahedron<T> tetrahedron) {
		this.tetrahedra.put(tetrahedron.getReference(), tetrahedron);
	}
	
	public void registerNewTriangle(Triangle3D<T> triangle) {
		this.triangles.put(triangle.getReference(), triangle);
	}
	
	public void registerNewEdge(Edge<T> edge) {
		this.edges.put(edge.getReference(), edge);
	}	
    
	/**
	 * Returns the transaction id for this cache manager.  
	 */
	public long getID() { 
		return id; 
	}
	
	
	/**
	 * Attempts to write all pending changes to the SOMs
	 * @throws NodeLockedException if any of the nodes involved in the transaction were 
	 * already locked by a different transaction.  
	 */
	synchronized private void commit() throws NodeLockedException{ 
		///////////////////////////////////////////////////////////////////////////////
		//Debug, check all of the objects to ensure they are owned by this CacheManager
		// Note: by using getCopyOfSpaceNode to check if the SOM original is locked, we do lock the node if
		// if was not locked. This should be a fatal error anyway but beware!
//		if(debug == true){
//			//Check Nodes
//			// @Toby: nice way to do this using Java 5 syntax...
//			for (SpaceNode nextNode : nodes.values()) {
////			Enumeration nodeEnum  = nodes.elements();
////			while(nodeEnum.hasMoreElements()){
////				SpaceNode nextNode = ((SpaceNode)nodeEnum.nextElement());
//				try{
//					// This statement doesn't work! nextNode.getReference returns the 
//					// true ManagedObjectReference, which will always report that it is not locked at all!
//					// What was the name of that law again?
////					if(! this.equals( nextNode.getReference().getLockingCacheManager() ) || 
////							nextNode.getReference().getSOM().getCopyOfSpaceNode(nextNode.getReference(), this) != null ){
//					if(!nextNode.isLockedBy(this) || 
//							nextNode.getReference().getSOM().getCopyOfSpaceNode(nextNode.getReference(), this) != null ){
//						//TODO log this 
//						throw new NodeLockedException(nextNode.getReference(), "Error while commiting, node locked by another cache manager!");		
//					}
//				}catch(ManagedObjectDoesNotExistException mone){
////					TODO log this 
//					throw new RuntimeException("The SOM copy of this node did not exist");
//				}
//			}
//
//			//Check Tetrahedrons
//			Enumeration tetEnum  = tetrahedra.elements();
//			while(tetEnum.hasMoreElements()){
//				Tetrahedron nextTet = ((Tetrahedron)tetEnum.nextElement());
//				try{
//					if(!nextTet.isLockedBy(this)  || 
//						nextTet.getReference().getSOM().getCopyOfSpaceNode(nextTet.getReference(), this) != null ){
//						//TODO log this 
//						throw new NodeLockedException(nextTet.getReference(), "Error while commiting, node locked by another cache manager!");		
//					}
//				}catch(ManagedObjectDoesNotExistException mone){
//	//				TODO log this 
//					throw new RuntimeException("The SOM copy of this node did not exist");
//				}
//			}
//
//			//Check triangles
//			Enumeration triEnum  = triangles.elements();
//			while(triEnum.hasMoreElements()){
//				Triangle3D nextTri = ((Triangle3D)triEnum.nextElement());
//				try{
//					if(!nextTri.isLockedBy(this)  || 
//						nextTri.getReference().getSOM().getCopyOfSpaceNode(nextTri.getReference(), this) != null ){
//						//TODO log this 
//						throw new NodeLockedException(nextTri.getReference(), "Error while commiting, node locked by another cache manager!");		
//					}
//				}catch(ManagedObjectDoesNotExistException mone){
////					TODO log this 
//					throw new RuntimeException("The SOM copy of this node did not exist");
//				}
//			}
//
//			//Check edges
//			Enumeration edgeEnum  = edges.elements();
//			while(edgeEnum.hasMoreElements()){
//				Edge nextEdge = ((Edge)edgeEnum.nextElement());
//				try{
//					if(!nextEdge.isLockedBy(this)  || 
//						nextEdge.getReference().getSOM().getCopyOfSpaceNode(nextEdge.getReference(), this) != null ){
//						//TODO log this 
//						throw new NodeLockedException(nextEdge.getReference(), "Error while commiting, node locked by another cache manager!");		
//					}
//				}catch(ManagedObjectDoesNotExistException mone){
//	//				TODO log this 
//					throw new RuntimeException("The SOM copy of this node did not exist");
//				}
//			}
//		}// End of Debug
			
		
//		for (Triangle3D<T> triangle : triangles.values()) {
//			if (triangle.isValid() && (!triangle.isInfinite()))
//			try {
//				if (!triangle.checkConformity()) 
//						System.out.println("Trying to commit a malformed triangle");
//			} catch (ManagedObjectDoesNotExistException e) {
//			}
//			
//		}

		// first, commit all objects, nodes go last:
		for (Tetrahedron<T> tetrahedron : tetrahedra.values()) {
			tetrahedron.getSOM().commitTetrahedron(this, tetrahedron);
		}
		for (Triangle3D<T> triangle : triangles.values()) {
			triangle.getSOM().commitTriangle(this, triangle);
		}
		for (Edge<T> edge : edges.values()) {
			edge.getSOM().commitEdge(this, edge);
		}
		int dummyCounter = 0;
		for (SpaceNode<T> node : nodes.values()) {
			if (node.getAdjacentTetrahedraCount() < 5 && getSOM().nodes.size() > 30) {
				dummyCounter++;
				if (dummyCounter > 1) {
					System.out.println("The node "+node+" has less than 5 adjacent tetrahedra and is being committed by process "+this);
					try {
						LinkedList<Tetrahedron<T>> tetrahedra = node.getAdjacentTetrahedra();
						System.out.println("List: "+tetrahedra);
					} catch (ManagedObjectDoesNotExistException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
			node.getSOM().commitNode(this, node);
		}

		// now, unlock all objects, nodes are the last objects to be unlocked:
		for (Tetrahedron<T> tetrahedron : tetrahedra.values()) {
			try {
				if (tetrahedron.isValid())
					tetrahedron.getSOM().unlockTetrahedron(tetrahedron.getReference());
			} catch (ManagedObjectDoesNotExistException e) {
				//TODO log this 
				throw new RuntimeException("Unexpected Error commiting ManagedObject - Transactional integrity lost - QUIT!");
			}
		}
		for (Triangle3D<T> triangle: triangles.values()) {
			try {
				if (triangle.isValid())
					triangle.getSOM().unlockTriangle(triangle.getReference());
			} catch (ManagedObjectDoesNotExistException e) {
				//TODO log this 
				throw new RuntimeException("Unexpected Error commiting ManagedObject - Transactional integrity lost - QUIT!");
			}
		}
		for (Edge<T> edge: edges.values()) {
			try {
				if (edge.isValid())
					edge.getSOM().unlockEdge(edge.getReference());
			} catch (ManagedObjectDoesNotExistException e) {
				//TODO log this 
				throw new RuntimeException("Unexpected Error commiting ManagedObject - Transactional integrity lost - QUIT!");
			}
		}
		for (SpaceNode<T> node: nodes.values()) {
			try {
				if (node.isValid())
					node.getSOM().unlockNode(node.getReference());
			} catch (ManagedObjectDoesNotExistException e) {
				//TODO log this 
				throw new RuntimeException("Unexpected Error commiting ManagedObject - Transactional integrity lost - QUIT!");
			}
		}
	    
	}
	
	/**
	 * Frees all ManagedObjects that were locked by this modifying process and re-queues this task.
	 *  
	 */
	private void rollBack() {
		// unlock all objects, nodes are the last objects to be unlocked:
		for (ManagedObjectReference<T> ref : edges.keySet()) {
			try {
				ref.getSOM().unlockEdge(ref);
			} catch (ManagedObjectDoesNotExistException e) {
				// this is more than expected... every new object will cause a ManagedObjectDoesNotExistException!
//				throw new RuntimeException("Unexpected Error while rolling back a transaction - Transactional integrity lost - QUIT!");
			}
		}
		for (ManagedObjectReference<T> ref : triangles.keySet()) {
			try {
				ref.getSOM().unlockTriangle(ref);
			} catch (ManagedObjectDoesNotExistException e) {
				// this is more than expected... every new object will cause a ManagedObjectDoesNotExistException!
//				throw new RuntimeException("Unexpected Error while rolling back a transaction - Transactional integrity lost - QUIT!");
			}
		}
		for (ManagedObjectReference<T> ref : tetrahedra.keySet()) {
			try {
				ref.getSOM().unlockTetrahedron(ref);
			} catch (ManagedObjectDoesNotExistException e) {
				// this is more than expected... every new object will cause a ManagedObjectDoesNotExistException!
//				throw new RuntimeException("Unexpected Error while rolling back a transaction - Transactional integrity lost - QUIT!");
			}
		}
		for (ManagedObjectReference<T> ref : nodes.keySet()) {
			try {
				ref.getSOM().unlockNode(ref);
			} catch (ManagedObjectDoesNotExistException e) {
				// this is more than expected... every new object will cause a ManagedObjectDoesNotExistException!
//				throw new RuntimeException("Unexpected Error while rolling back a transaction - Transactional integrity lost - QUIT!");
			}
		}
		// free memory:
		clean();
//		id = mySOM.getUniqueAddress();
		mySOM.addAction(this);
		
	}

	/**
	 * @return the mySOM
	 */
	protected SpatialOrganizationManager<T> getSOM() {
		return mySOM;
	}
	
	public String toString() {
		return this.descriptionString();
	}
	
	protected void finalize() throws Throwable
	{
//		System.out.println(id+": Bye bye!");
		mySOM.processFinalized(getID());
		super.finalize(); //not necessary if extending Object.
	}
    
}

