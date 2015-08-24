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
@SuppressWarnings("unchecked")

/**
 * An exception that will be thrown by a {@link SpatialOrganizationManager}, if a local copy of 
 * an object is requested that is currently locked by a CacheManager.
 * 
 * @author Dennis Goehlsdorf;
 *
 */
public class NodeLockedException extends Throwable {
	private ManagedObjectReference lockedNode;
	private long lockingIndex = 0;
	/**
	 * Initializes a simple NodeLockedException.
	 * @param lockedNode A reference to the node that caused the problem.
	 */
	public NodeLockedException(ManagedObjectReference lockedNode, long lockingCacheManager) {
		super();
		this.lockedNode = lockedNode;
		this.lockingIndex = lockingCacheManager;
	}
	
	/**
	 * Initializes a NodeLockedException with an additional comment about the reason for this exception.
	 * @param lockedNode A reference to the node that caused the problem.
	 * @param arg Further explanation about this Exception.
	 */
	public NodeLockedException(ManagedObjectReference lockedNode, long lockingCacheManager, String arg) {
		super(arg);
		this.lockedNode = lockedNode;
		this.lockingIndex = lockingCacheManager;
	}
	
	/**
	 * Gives a reference to the node to which an access was attempted although it is locked.
	 * @return
	 */
	public ManagedObjectReference getLockedNodeReference() {
		return this.lockedNode;
	}
	
	public long getLockingCacheManagerIndex() {
		return this.lockingIndex;
	}
}
