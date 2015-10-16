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

import ini.cx3d.utilities.StringUtilities;

import java.util.AbstractSequentialList;
import java.util.LinkedList;
import java.util.Objects;

/**
 * This class is used to represent an edge in a triangulation. Each edge has two endpoint and
 * additionally stores links to all tetrahedra adjacent to it.
 *
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of the user objects stored in the endpoints of an edge.
 */
public class Edge<T> implements ini.cx3d.spatialOrganization.interfaces.Edge<T> {
//	/**
//	 * Used to assign a unique identification number to each initialized edge.
//	 */
//	private static int IDCOUNTER = 0;
//
//	/**
//	 * The identification number of an edge.
//	 */
//	private int id = IDCOUNTER++;

	/**
	 * The two endpoints of this edge.
	 */
	private SpaceNode<T> a, b;

	/**
	 * A list of all tetrahedra that are adjacent to this edge.
	 */
	private LinkedList<Tetrahedron> adjacentTetrahedra = new LinkedList<Tetrahedron>();

	/**
	 * Stores the cross section area associated with this edge.
	 */
	private double crossSectionArea = 0.0;

	/**
	 * Initializes a new edge with two specified endpoints.
	 * @param a The first endpoint of the new edge.
	 * @param b The second endpoint of the new edge.
	 */
	public Edge(SpaceNode<T> a, SpaceNode<T> b) {
		this.a = a;
		this.b = b;
		if (a != null) a.addEdge(this);
		if (b != null) b.addEdge(this);

//		this.posA = (a!=null)?a.addEdge(this):null;
//		this.posB = (b!=null)?b.addEdge(this):null;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public SpatialOrganizationNode<T> getOpposite(SpaceNode<T> comingFrom) {
		if (Objects.equals(comingFrom, a)) return b;
		else if (Objects.equals(comingFrom, b)) return a;
		else throw new RuntimeException("The edge "+this+" is not adjacent to the node "+comingFrom);
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
	 */
	@Override
	public T getOppositeElement(T element) {
		if (a!=null  && b!=null) {
			T aT = a.getUserObject();
			if(Objects.equals(element, aT)){
				return b.getUserObject();
			}else{
				return a.getUserObject();
			}
		}
		return null;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public T getFirstElement() {
		return a.getUserObject();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public T getSecondElement() {
		return b.getUserObject();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public double getCrossSection() {
		return this.crossSectionArea;
	}

	/**
	 *	@return A string representation of this edge. The format of this string is
	 *     "(ID1 - ID2)", where ID1 and ID2 denote the identification numbers of the
	 *     endpoints of this edge.
	 */
	@Override
	public String toString() {
		return "(" + StringUtilities.toStr(crossSectionArea) + " - " + a + " - " + b + " - "+ StringUtilities.toStr(adjacentTetrahedra) +")";
	}

	/**
	 * Tests whether this edge is connecting a pair of points.
	 * @param a The first node.
	 * @param b The second node.
	 * @return <code>true</code>, if this edge connects <code>a</code> and <code>b</code>.
	 */
	public boolean equals(SpaceNode<T> a, SpaceNode<T> b) {
		return (Objects.equals(this.a, a) && Objects.equals(this.b, b)) || (Objects.equals(this.b, a) && Objects.equals(this.a, b));
	}

	/**
	 * Removes a tetrahedron from this edge's list of tetrahedra. If this edge is not incident to
	 * any tetrahedra after the removal of the specified tetrahedron, the edge removes itself from
	 * the triangulation by calling {@link #remove()}.
	 * @param tetrahedron A tetrahedron incident to this edge which should be removed.
	 */
	public void removeTetrahedron(Tetrahedron<T> tetrahedron) {
		adjacentTetrahedra.remove(tetrahedron);
//		element.remove();
		if (adjacentTetrahedra.isEmpty())
			remove();
	}

	/**
	 * Adds a tetrahedron to this edge's list of tetrahedra.
	 * @param tetrahedron A tetrahedron incident to this edge which should be added.
	 */
	public void addTetrahedron(Tetrahedron<T> tetrahedron) {
		adjacentTetrahedra.add(tetrahedron);
	}

	/**
	 * Removes this edge from the triangulation. To do so, the two endpoints are informed
	 * that the edge was removed.
	 *
	 */
	public void remove() {
		if (a != null)
			a.removeEdge(this);
		if (b != null)
			b.removeEdge(this);
		//		if (posA != null)
//			posA.remove();
//		if (posB != null)
//			posB.remove();
	}

	/**
	 * Returns the list of incident tetrahedra.
	 * @return The list of incident tetrahedra.
	 */
	public AbstractSequentialList<Tetrahedron> getAdjacentTetrahedra() {
		return adjacentTetrahedra;
	}

	/**
	 * Changes the cross section area of this edge.
	 * @param change The value by which the cross section area of this edge has changed.
	 * At initialization, this area is set to zero and all tetrahedra that are registered as
	 * incident tetrahedra increase the cross section area.
	 */
	public void changeCrossSectionArea(double change) {
		this.crossSectionArea += change;
	}

}
