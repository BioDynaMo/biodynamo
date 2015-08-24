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

/**
 * A ManagedObject-tracker that is used by local copies of ManagedObjects to organize copies of
 * other managed objects.
 * @author dennis
 * @param <T>
 */
public class CacheRoutedMOTracker<T> implements ManagedObjectTracker<T>, Serializable{
	CacheManager<T> cm;
	ManagedObjectReference<T> myReference;
	
	public CacheRoutedMOTracker(CacheManager<T> cm, ManagedObjectReference<T> myReference) {
		this.cm = cm;
		this.myReference = myReference;
	}
	
	public long getAddress() {
		return myReference.getAddress();
	}

	public ManagedObjectReference<T> getReference() {
		return myReference;
	}

	public SpatialOrganizationManager<T> getSOM() {
		return myReference.getSOM();
	}

	public Edge<T> organizeEdge(ManagedObjectReference<T> mor)
			throws NodeLockedException {
		Edge<T> ret = cm.getEdge(mor);
		return cm.getEdge(mor);
	}

	public SpaceNode<T> organizeNode(ManagedObjectReference<T> mor)
			throws NodeLockedException {
		return cm.getNode(mor);
	}

	public Tetrahedron<T> organizeTetrahedron(ManagedObjectReference<T> mor)
			throws NodeLockedException {
		return cm.getTetrahedron(mor);
	}

	public Triangle3D<T> organizeTriangle(ManagedObjectReference<T> mor)
			throws NodeLockedException {
		return cm.getTriangle(mor);
	}

	public CacheManager<T> getLockingCacheManager() {
		return cm;
	}
	
	public CacheRoutedMOTracker<T> getCopy() {
		return new CacheRoutedMOTracker<T>(cm,myReference.getCopy());
	}

	public SpatialOrganizationManager<T> getSupervisingSOM() {
		return cm.getSOM();
	}

	
}
