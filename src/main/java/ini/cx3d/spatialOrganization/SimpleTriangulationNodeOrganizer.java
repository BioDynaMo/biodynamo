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
import java.util.Stack;

/**
 * This class is a very simple implementation of {@link AbstractTriangulationNodeOrganizer}.
 * All nodes are stored in a binary tree in order to obtain a good performance when
 * checking whether a certain node is already added to this organizer. 
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of user objects associated with the nodes in the current triangulation.
 */
public class SimpleTriangulationNodeOrganizer<T> extends
		AbstractTriangulationNodeOrganizer<T> {

	BinaryTreeElement<T> treeHead = BinaryTreeElement.generateTreeHead();
	
	public SimpleTriangulationNodeOrganizer() {
	}

	public String toString() {
		return "[" + treeHead + "]";
	}
	public void addNode(SpaceNode<T> node) {
		treeHead.insert(node);
	}
	       
	public Iterator<SpaceNode<T>> getNodeIterator(
			SpaceNode<T> referencePoint, DistanceReporter rep) {
		return treeHead.iterator();
	}

	public void removeNode(SpaceNode<T> node) {
		treeHead.remove(node, null);
	}
	public SpaceNode<T> getFirstNode() {
		return treeHead.bigger.content;
	}

}
