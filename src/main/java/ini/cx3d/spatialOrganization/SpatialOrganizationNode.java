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
 * Interface to define the basic properties of a node in the triangulation.
 * 
 * @author Dennis Goehlsdorf & Frederic Zubler
 *
 * @param <T> The type of user objects associated with each node in the triangulation.
 */
public interface SpatialOrganizationNode<T> {
	
	public void addSpatialOrganizationNodeMovementListener(SpatialOrganizationNodeMovementListener<T> listener);
	
	public Iterable<SpatialOrganizationEdge<T>> getEdges();
	
	public Iterable<T> getNeighbors();
	
	public SpatialOrganizationNode<T> getNewInstance(double[] position, T userObject) 
		throws PositionNotAllowedException;
	
	public Iterable<T> getPermanentListOfNeighbors();
	
	public double[] getPosition();
	
	public T getUserObject();
	
	public Object[] getVerticesOfTheTetrahedronContaining(double[] position);
	
	public double getVolume();
	
	public void moveFrom(double[] delta) throws PositionNotAllowedException;
	
	public void remove();

}
