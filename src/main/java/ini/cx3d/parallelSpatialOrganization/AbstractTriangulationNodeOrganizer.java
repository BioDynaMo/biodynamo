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

import java.util.Iterator;

/**
 * Instances of child classes of this class are used to keep track of 
 * nodes in an incomplete triangulation which might possibly become neighbors of open triangles.  
 * 
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of user objects associated with nodes in the current triangulation.
 */
public abstract class AbstractTriangulationNodeOrganizer<T>  {
	public interface DistanceReporter {
		public double getCurrentMinimalDistance();
	}
	public abstract Iterator<SpaceNode<T>> getNodeIterator(SpaceNode<T> referencePoint, DistanceReporter rep);
	public Iterable<SpaceNode<T>> getNodes(final SpaceNode<T> referencePoint, final DistanceReporter rep) {
		return new Iterable<SpaceNode<T>>() {
			public Iterator<SpaceNode<T>> iterator() {
				return getNodeIterator(referencePoint, rep);
			}
		};
	}
	public abstract void removeNode(SpaceNode<T> node);
	public abstract void addNode(SpaceNode<T> node);
	public void addTriangleNodes(Triangle3D<T> triangle) throws NodeLockedException, ManagedObjectDoesNotExistException {
		SpaceNode<T>[] triangleNodes = triangle.getNodes();
		addNode(triangleNodes[1]);
		addNode(triangleNodes[2]);
		addNode(triangleNodes[0]);
	}
	public abstract SpaceNode<T> getFirstNode();
}
