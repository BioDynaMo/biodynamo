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

import java.util.Iterator;

import ini.cx3d.spatialOrganization.interfaces.Triangle3D;

/**
 * Instances of child classes of this class are used to keep track of 
 * nodes in an incomplete triangulation which might possibly become neighbors of open triangles.  
 * 
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of user objects associated with nodes in the current triangulation.
 */
public abstract class AbstractTriangulationNodeOrganizer<T> {
	public interface DistanceReporter {
		public double getCurrentMinimalDistance();
	}
	public abstract Iterator<ini.cx3d.spatialOrganization.interfaces.SpaceNode<T>> getNodeIterator(ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> referencePoint, DistanceReporter rep);
	public Iterable<ini.cx3d.spatialOrganization.interfaces.SpaceNode<T>> getNodes(final ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> referencePoint, final DistanceReporter rep) {
		return new Iterable<ini.cx3d.spatialOrganization.interfaces.SpaceNode<T>>() {
			public Iterator<ini.cx3d.spatialOrganization.interfaces.SpaceNode<T>> iterator() {
				return getNodeIterator(referencePoint, rep);
			}
		};
	}
	public abstract void removeNode(ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> node);
	public abstract void addNode(ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> node);
	public void addTriangleNodes(Triangle3D<T> triangle) {
		addNode(triangle.getNodes()[1]);
		addNode(triangle.getNodes()[2]);
		addNode(triangle.getNodes()[0]);
	}
	public abstract ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> getFirstNode();
}
