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

/**
 * Class to provide hash values for triangles.
 * A simple algorithm is used to calculate a hash value for a triplet of nodes.
 * The calculated hash values do not depend on the order of nodes. Therefore, triangles can
 * be reliably found back even when they were initialized with the same endpoints in a different order.  
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of user objects associated with nodes in the triangulation.
 */
class TriangleHashKey<T> {
	SpaceNode<T> a, b, c;
	int hashCode;

	/**
	 * Creates a hash value for a triplet of nodes.
	 * @param a The first node.
	 * @param b The second node.
	 * @param c The third node.
	 */
	TriangleHashKey(SpaceNode<T> a, SpaceNode<T> b, SpaceNode<T> c) {
		this.a = a;
		this.b = b;
		this.c = c;
		createHashCode((a != null) ? a.getId() : -1,
				(b != null) ? b.getId() : -1,
				(c != null) ? c.getId() : -1);
	}

	/**
	 * Internal function to create the hash value. Three integer values are needed to compute this value.
	 * @param aId The first node ID.
	 * @param bId The second node ID.
	 * @param cId The third node ID.
	 */
	private void createHashCode(int aId, int bId,
			int cId) {
		hashCode = (Math.min(aId, Math.min(bId, cId))
				* 31
				+ Math.max(aId, Math.max(bId, cId))
				* 11 + aId + bId + cId) % 2000000001;
	}

	/**
	 * @return The hash value associated with this triplet of nodes.
	 */
	public int hashCode() {
		return hashCode;
	}

	/** 
	 * Determines whether this object is equal to another one.
	 * @param obj The object with which this <code>TriangleHashKey</code> should be compared.
	 * @return <code>true</code>, iff <code>obj</code> is an instance of <code>TriangleHashKey</code> and 
	 * 		   refers to the same three points as this <code>TriangleHashKey</code>. 	
	 */
	public boolean equals(Object obj) {
		if (obj instanceof TriangleHashKey) {
			TriangleHashKey<T> other = (TriangleHashKey<T>) obj;
			return ((a == other.a) && (((b == other.b) && (c == other.c)) || ((b == other.c) && (c == other.b))))
					|| ((a == other.b) && (((b == other.a) && (c == other.c)) || ((b == other.c) && (c == other.a))))
					|| ((a == other.c) && (((b == other.a) && (c == other.b)) || ((b == other.b) && (c == other.a))));
		} else
			return false;
	}
}
