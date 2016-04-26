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

import ini.cx3d.spatialOrganization.interfaces.*;
import ini.cx3d.spatialOrganization.interfaces.SpaceNode;
import ini.cx3d.swig.simulation.SimpleTriangulationNodeOrganizerT_PhysicalNode;

import java.util.AbstractSequentialList;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * This class is a very simple implementation of {@link AbstractTriangulationNodeOrganizer}.
 * All nodes are stored in a binary tree in order to obtain a good performance when
 * checking whether a certain node is already added to this organizer. 
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of user objects associated with the nodes in the current triangulation.
 */
public class SimpleTriangulationNodeOrganizer<T>  extends SimpleTriangulationNodeOrganizerT_PhysicalNode {
//		AbstractTriangulationNodeOrganizer<T> {

	BinaryTreeElement<T> treeHead = BinaryTreeElement.generateTreeHead();
	
	public SimpleTriangulationNodeOrganizer() {
//		registerJavaObject(this);
	}

	public void addTriangleNodes(ini.cx3d.spatialOrganization.interfaces.Triangle3D triangle) {
		addNode(triangle.getNodes()[1]);
		addNode(triangle.getNodes()[2]);
		addNode(triangle.getNodes()[0]);
	}

	public AbstractSequentialList<ini.cx3d.spatialOrganization.interfaces.SpaceNode> getNodes(final ini.cx3d.spatialOrganization.interfaces.SpaceNode referencePoint) {
		AbstractSequentialList<ini.cx3d.spatialOrganization.interfaces.SpaceNode> list = new LinkedList<>();
		Iterator<SpaceNode> it = getNodeIterator(referencePoint);
		while (it.hasNext()) {
			list.add(it.next());
		}
		return list;
	}

	@Override
	public String toString() {
		return "[" + treeHead + "]";
	}
	@Override
	public void addNode(ini.cx3d.spatialOrganization.interfaces.SpaceNode node) {
		treeHead.insert(node);
	}
	       
	@Override
	public Iterator<ini.cx3d.spatialOrganization.interfaces.SpaceNode> getNodeIterator(
			ini.cx3d.spatialOrganization.interfaces.SpaceNode referencePoint) {
		return treeHead.iterator();
	}

	@Override
	public void removeNode(ini.cx3d.spatialOrganization.interfaces.SpaceNode node) {
		treeHead.remove(node, null);
	}
	@Override
	public ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> getFirstNode() {
		return treeHead.bigger.content;
	}

}
