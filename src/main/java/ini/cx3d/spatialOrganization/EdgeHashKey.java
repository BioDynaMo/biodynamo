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

package ini.cx3d.spatialOrganization;

import static ini.cx3d.utilities.Matrix.crossProduct;
import static ini.cx3d.utilities.Matrix.dot;
import static ini.cx3d.utilities.Matrix.normalize;
import static ini.cx3d.utilities.Matrix.subtract;

/**
 * Class to provide hash values for edges between two nodes.
 * A simple algorithm is used to calculate a hash value for a pair of nodes.
 * The hash values do not depend on a direction of an edge. Therefore, edges can
 * be reliably found back even when they were initialized with the same endpoints
 * in opposite order.  
 * 
 * This class also provides basic functions for comparisons of two edges.
 * 
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of user objects associated with nodes in this triangulation.
 */
class EdgeHashKey<T> {
	/**
	 * The endpoints of the edge for which a hash value should be calculated.
	 */
	SpaceNode<T> a, b;
	
	/**
	 * The vector connecting the positions of <code>a</code> and <code>b</code>.
	 */
	double[] ab;
	
	/**
	 * A vector which is orthogonal to <code>ab</code> and points into the direction of
	 * the non-open side of this edge.
	 */
	double[] lastNormalVector;
	
	/**
	 * The hash value associated with this edge.
	 */
	int hashCode;

	/**
	 * Creates a new instance of <code>EdgeHashKey</code> with the two endpoints
	 * <code>a</code> and <code>b</code>. A third node is expected as additional 
	 * parameter which defines the direction of the non-open side of this edge.
	 * @param a The first endpoint of the represented edge.
	 * @param b The second enpoint of the represented edge.
	 * @param oppositeNode A node on the non-open side of this edge.
	 */
	EdgeHashKey(SpaceNode<T> a, SpaceNode<T> b,
			SpaceNode<T> oppositeNode) {
		this.a = a;
		this.b = b;
		// this.c = oppositeNode;
		this.ab = subtract(b.getPosition(), a
				.getPosition());
		this.lastNormalVector = normalize(crossProduct(
				ab, subtract(
						oppositeNode.getPosition(), a
								.getPosition())));
		this.hashCode = Math.max(a.getId(), b.getId())
				* 11 + Math.min(a.getId(), b.getId())
				* 31;
	}

	/**
	 * {@inheritDoc}
	 */
	public String toString() {
		return "(" + a + ", " + b + ")";
	}

	/**
	 * {@inheritDoc} 
	 */
	public int hashCode() {
		return hashCode;
	}

	/**
	 * Compares the represented edge with another object.
	 * @param obj The Object with which this edge should be compared.
	 * @return <code>true</code>, if <code>obj</code> is of type 
	 * <code>EdgeHashKey</code> and has the same endpoints as this edge. 
	 * <code>false</code> is returned in all other cases.
	 */
	public boolean equals(Object obj) {
		if (obj instanceof EdgeHashKey) {
			EdgeHashKey other = (EdgeHashKey) obj;
			return ((a == other.a) && (b == other.b))
					|| ((a == other.b) && (b == other.a));
		} else
			return false;
	}

	/**
	 * computes the cosine between this edge to another point measured at the 
	 * first endpoint of this edge.
	 * @param fourthPoint The other point.
	 * @return The cosine between this edge and an edge between the first
	 * endpoint of this edge and <code>fourthPoint</code>.
	 */
	double getCosine(double[] fourthPoint) {
		double[] normal = normalize(crossProduct(ab,
				subtract(fourthPoint, a.getPosition())));
		// double sine = norm(crossProduct(normal,lastNormalVector));
		double cosine = dot(normal, lastNormalVector);
		// if ((sine > -0.000000001) && (sine < 0.000000001)) {
		// double[] ac = subtract(c.getPosition(),a.getPosition());
		// double[] abNorm = normalize(ab);
		// normal = subtract(ac,scalarMult(dot(ac,abNorm), abNorm));
		// Plane3D dummyPlane = new
		// Plane3D(normal,dot(normal,a.getPosition()));
		// if (dummyPlane.trulyOnDifferentSides(c.getPosition(),
		// fourthPoint))
		// return -1;
		// else return 1;
		// }
		// if (sine < 0) return -cosine;
		// else return cosine;
		if (cosine > 0.999999999)
			return 1;
		else if (cosine < -0.99999999)
			return -1;
		return cosine;
	}

	/**
	 * Returns the opposite node of a given node if the latter is incident to this edge.
	 * @param node The given node.
	 * @return The incident node opposite to <code>node</code>.
	 */
	SpaceNode<T> oppositeNode(SpaceNode<T> node) {
		if (a == node)
			return b;
		else
			return a;
	}
}
