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

/**
 * Interface to define the basic properties of an edge in the simulation.
 * @author Frederic Zubler, Dennis Goehlsdorf
 *
 * @param <T> The type of user objects associated with nodes of the triangulation.
 */
public interface SpatialOrganizationEdge<T> {
	/**
	 * Given one user object associated with one endpoint of this edge, 
	 * this function returns the user object associated with the other endpoint of this edge.
	 * @param element A user object associated with one of the two endpoints of this edge.
	 * @return The second user object associated to an endpoint of this edge.
	 */
	public T getOppositeElement(T element);
	
	/**
	 * Given one endpoint of this edge, this function returns the other endpoint.
	 * @param first One endpoint of this edge.
	 * @return The other endpoint of this edge. Throws a RuntimeException if the node <code>first</code>
	 * is not incident to this edge. 
	 */
	public SpatialOrganizationNode<T> getOpposite(SpatialOrganizationNode<T> first);
	
	/**
	 * @return One of the two user objects associated to the endpoints of this edge. 
	 * Returns the opposite user object to the result of {@link #getSecondElement()}. 
	 */
	public T getFirstElement();
	
	/**
	 * @return One of the two user objects associated to the endpoints of this edge. 
	 * Returns the opposite user object to the result of {@link #getFirstElement()}. 
	 */
	public T getSecondElement();
	
	/**
	 * Returns the current cross section area associated with this edge. 
	 * @return A double value representing the cross section area between the two endpoints of this edge.
	 */
	public double getCrossSection();
}
