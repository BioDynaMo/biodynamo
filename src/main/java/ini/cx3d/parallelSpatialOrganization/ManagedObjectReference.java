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
import java.util.logging.Level;
import java.util.logging.Logger;

/** 
 * 
 * @author Toby Weston, Dennis Goehlsdorf
 * @version 0.1
 * @Date: 13/6/2008
 * 
 * Encapsulates a reference to an object that is owned and managed by an instance of SpatialOrganisationManager.
 */

public class ManagedObjectReference<T> implements ManagedObjectTracker<T>, Serializable {

	//Get a logger
	private static Logger theLogger = Logger.getLogger(ManagedObjectReference.class.getName());	
	///////////////////////////////////////////////////////////////////
	/// Replace with config. file at some point.....
	///////////////////////////////////////////////////////////////////	
	{
		//theLogger.setLevel(Level.FINER);
		theLogger.setLevel(Level.FINEST);
	}
	///////////////////////////////////////////////////////////////////		
	
	
	protected long address = 0;
	protected SpatialOrganizationManager<T> som= null;
	
	/**
	 * Simple constructor to create a new ManagedObjectReference.
	 * @param address The unique address of the ManagedObject.
	 * @param som The SpatialOrganizationManager that is storing the object.
	 */
	public ManagedObjectReference(long address, SpatialOrganizationManager<T> som) {
		theLogger.finest("In ManagedObjectReference --- ManagedObjectReference(long address, SpatialOrganizationManager<T> som)");
		this.address = address;
		this.som = som;
	}
	
	/**
	 * Creates a new ManagedObjectReference which is pointing to a given SpatialOrganizationManager. A new unique address is 
	 * requested from the som.
	 * @param som
	 */
	public ManagedObjectReference(SpatialOrganizationManager<T> som) {
		theLogger.finest("In ManagedObjectReference --- ManagedObjectReference(SpatialOrganizationManager<T> som)");
		this.address = som.getUniqueAddress();
		this.som = som;
		//this(som.getUniqueAddress(),som);
	}
	
	/**
	 * Simple clone-constructor;
	 * @param original The ManagedObjectReference that should be cloned.
	 */
	public ManagedObjectReference(ManagedObjectReference<T> original) {
		this(original.getAddress(),original.getSOM());
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Object#clone()
	 */
	public ManagedObjectReference<T> getCopy() {
		return new ManagedObjectReference(address, som);
	}
	
//	//Identifies which transaction (if any) this node is included in.
//	protected Integer transactionID = null;
	
	/**
	 * @return the unique address of this Managed Object
	 */
	public long getAddress() {
		return address;
	}

	/**
	 * @return a reference to the Managed Object
	 */
	public ManagedObjectReference<T> getReference() {
		return this;
	}
	
//	/**
//	 * @return a local deep copy of the Managed Object
//	 */
//	public abstract ManagedObject<T> getLocalCopy() throws NodeLockedException;

	/**
	 * @return the SpatialOrganizationManager that hosts this Managed Object
	 */
	public SpatialOrganizationManager<T> getSOM() {
		return som;
	}
	
	
	/**
	 * Returns the hashCode for this managed object reference.  
	 */
	public int hashCode() { 
		return (int)(address); 
	}
	
	public boolean equals(Object o) {
		if (o instanceof ManagedObjectReference) {
			return ((ManagedObjectReference<T>)o).getAddress() == address;
		}
		else 
			return false;
	}

	/** 
	 * Used to determine whether an object is locked by a cache manager.
	 * @return <code>null</code>, if the object is not locked at the moment and a 
	 * link to a {@link CacheManager} if it is locked.
	 */
	
	public Edge<T> organizeEdge(ManagedObjectReference<T> mor)
			throws NodeLockedException, ManagedObjectDoesNotExistException {
		return som.getCopyOfEdge(mor,null);
	}

	public SpaceNode<T> organizeNode(ManagedObjectReference<T> mor)
			throws NodeLockedException, ManagedObjectDoesNotExistException {
		return som.getCopyOfSpaceNode(mor, null);
	}

	public Tetrahedron<T> organizeTetrahedron(ManagedObjectReference<T> mor)
			throws NodeLockedException, ManagedObjectDoesNotExistException {
		return som.getCopyOfTetrahedron(mor, null);
	}

	public Triangle3D<T> organizeTriangle(ManagedObjectReference<T> mor)
			throws NodeLockedException, ManagedObjectDoesNotExistException {
		return som.getCopyOfTriangle(mor, null);
	}
	
	public CacheManager<T> getLockingCacheManager() {
		// not locked:
		return null;
	}

	public SpatialOrganizationManager<T> getSupervisingSOM() {
		return som;
	}
	
	public String toString() {
		return ""+this.getAddress();
	}

}
