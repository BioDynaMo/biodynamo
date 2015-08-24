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

import sun.awt.image.ImageWatched.Link;

/**
 * A wrapper interface that is used by managed objects to either direct requests for 
 * other managed directly to the responsible SpatialOrganizationManager or to the 
 * current CacheManager. 
 * 
 * @author dennis
 *
 * @param <T>
 */
public interface ManagedObjectTracker<T> {
	public ManagedObjectReference<T> getReference();
	public SpatialOrganizationManager<T> getSOM();
	public SpatialOrganizationManager<T> getSupervisingSOM();
	public long getAddress();
	
	/** 
	 * Used to determine whether an object is locked by a cache manager.
	 * @return <code>null</code>, if the object is not locked at the moment and a 
	 * link to a {@link CacheManager} if it is locked.
	 */
	public CacheManager<T> getLockingCacheManager(); 
	public SpaceNode<T> organizeNode(ManagedObjectReference<T> mor) throws NodeLockedException, ManagedObjectDoesNotExistException;
	public Triangle3D<T> organizeTriangle(ManagedObjectReference<T> mor) throws NodeLockedException, ManagedObjectDoesNotExistException;
	public Tetrahedron<T> organizeTetrahedron(ManagedObjectReference<T> mor) throws NodeLockedException, ManagedObjectDoesNotExistException;
	public Edge<T> organizeEdge(ManagedObjectReference<T> mor) throws NodeLockedException, ManagedObjectDoesNotExistException;
	public ManagedObjectTracker<T> getCopy();
}
