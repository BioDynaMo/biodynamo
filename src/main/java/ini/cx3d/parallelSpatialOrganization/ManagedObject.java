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

import javax.net.ssl.ManagerFactoryParameters;

/** 
 * 
 * @author Toby Weston
 * @version 0.1
 * @Date: 13/6/2008
 * 
 * Describes an object that is owned and managed by an instance of SpatialOrganizationManager.
 */

public abstract class ManagedObject<T> implements Serializable{
	
	//Get a logger
	private static Logger theLogger = Logger.getLogger(ManagedObject.class.getName());	
	///////////////////////////////////////////////////////////////////
	/// Replace with config. file at some point.....
	///////////////////////////////////////////////////////////////////	
	{
		//theLogger.setLevel(Level.FINER);
		theLogger.setLevel(Level.FINEST);
	}
	///////////////////////////////////////////////////////////////////		
	
	/**
	 * A boolean used to keep track of ManagedObjects that were deleted within a modifying process. 
	 */
	private boolean valid = true;
	
	/**
	 * A ManagedObjectTracker that stores all information about the virtual memory position of this node. 
	 * (Assigned SpatialOrganizationManager, address and - for local copies - a CacheManager.)
	 */
	ManagedObjectTracker<T> tracker;
//	
//	public ManagedObject(CacheManager<T> cm, ManagedObjectReference<T> ref) {
//		this.tracker = new CacheRoutedMOTracker<T>(cm,ref);
//		cm.registerNewManagedObject(ref,this);
//	}
//	
//	public ManagedObject(SpatialOrganizationManager<T> som, CacheManager<T> cm) {
//		this(cm, new ManagedObjectReference<T>(som));
//	}
//	
//	public ManagedObject(ManagedObject<T> origObj) {
//		this.tracker = origObj.tracker.clone();
//	}

	public void initTracker(CacheManager<T> cm, ManagedObjectReference<T> ref) {
		this.tracker = new CacheRoutedMOTracker<T>(cm, ref);
	}
	
	public void initTracker(CacheManager<T> cm, SpatialOrganizationManager<T> som) {
		this.tracker = new CacheRoutedMOTracker<T>(cm, new ManagedObjectReference<T>(som));
	}

	public void initTracker(ManagedObject<T> origObj) {
		this.tracker = origObj.tracker.getCopy();
	}
//	/**
//	 * @return a local deep copy of the Managed Object
//	 */
//	public abstract ManagedObject<T> getLocalCopy() throws NodeLockedException, ManagedObjectDoesNotExistException;
	
	public CacheManager<T> getLockingCacheManger() {
		return tracker.getLockingCacheManager();
	}
	
	/**
	 * @return a unique reference to this ManagedObject (including an ID and the host SOM)
	 */
	public ManagedObjectReference<T> getReference() {
		return tracker.getReference();
	}

	/**
	 * @return the SpatialOrganizationManager that hosts this Managed Object
	 */
	public SpatialOrganizationManager<T> getSOM() {
		return tracker.getSOM();
	}
	
	public SpatialOrganizationManager<T> getSupervisingSOM() {
		return tracker.getSupervisingSOM();
	}

	/**
	 * @return the unique address of this Managed Object
	 */
	public long getAddress() {
		return tracker.getAddress();
	}
	
	/**
	 * Assigns this ManagedObject to a CacheManager and locks the object.
	 * @param cm The CacheManager that requested this object.
	 * @return the CacheManager that owned this object before this function was called, or <code>null</code>,
	 * if the object was not locked before.
	 */
	// TODO: The locking logic should be moved to the SOM: 
	public CacheManager<T> lock(CacheManager<T> cm) {
		// check whether you're owned by any CacheManager yet:
		CacheManager<T> lockingCM = tracker.getLockingCacheManager();
		// Yes? Don't do anything but inform the calling process that you were owned before (so it can throw a NodeLockedException etc.) 
		// No? So then you are from now on:
		if (lockingCM == null) {
			this.tracker = new CacheRoutedMOTracker<T>(cm, this.tracker.getReference());
		}
		return lockingCM;
	}
	
	/**
	 * Unlocks this ManagedObject. If this object was linked to a modifying process, this dependency is removed. 
	 */
	public void unlock() {
		this.tracker = this.tracker.getReference();
	}

	/**
	 * Returns whether this object is locked by a specific CacheManager
	 * @param cm The CacheManager that might have locked this object;
	 * @return <code>true</code>, if <code>cm</code> owns a lock for this object and <code>false</code> else.
	 */
	public boolean isLockedBy(CacheManager<T> cm) {
		theLogger.finest("In ManagedObject --- isLockedBy");
		if(null == cm){
			theLogger.finest("                      ----     Locking Cache Manager is NULL");
		}

		if(null == this.tracker.getLockingCacheManager()){
			theLogger.finest("                      ----     This Locking Cache Manager is NULL");
			return false;
		}
		else if (this.tracker.getLockingCacheManager().hashCode() == cm.hashCode())
			// TODO: This call
			return true;
		else if (cm.equals(this.tracker.getLockingCacheManager()))
			throw new RuntimeException("WTF");
		else return false;
	}
	
	public void revalidate() {
		this.valid = true;
	}
	
	/**
	 * Marks this object as removed.
	 */
	public void invalidate() {
		this.valid = false;
	}
	
	/**
	 * Returns whether this object is still valid.
	 * @return <code>true</code>, if this object is still part of the triangulation and <code>false</code> else.
	 */
	public boolean isValid() {
		return valid;
	}
	
//	/**
//	 * @param the current transaction ID
//	 */
//	public void setTransactionID(Integer transactionID);

}
