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

import ini.cx3d.parallelSpatialOrganization.ManagedObject;
import ini.cx3d.parallelSpatialOrganization.NodeReference;
//import javax.vecmath.Point3d;

/** 
 * 
 * @author Toby Weston
 * @version 0.1
 * @Date: 13/6/2008
 * 
 * Defines the interface between the simulation and the spatial organisation sub system.
 * 
 * TODO - Define this interface with Fred
 */


public interface SpatialOrganisation<T> {

	/**
	 * Get the edges of the specified node
	 * @param theNode
	 */
	//public Iterable<SpatialOrganizationEdge<T>> getEdges();
	
	
	/**
	 * Get the neighbours of the specified node
	 * @param theNode
	 */
	//public Iterable<T> getNodeNeighbors(ManagedObject theNode);
	
	
//	/**
//	 * Create a new Node instance,
//	 * @param theNode existing node
//	 * @param pos the location of the node
//	 * @param userObject the user object
//	 */
//	public SpatialOrganizationNode<T> getNewNodeInstance(ManagedObject theNode, Point3d pos, T userObject) 
//		throws PositionNotAllowedException;
	
	
	/**
	 * Create a new Node instance,
	 * @param theNode existing node
	 * @param pos the location of the node
	 * @param userObject the user object
	 * @deprecated
	 */
	//public SpatialOrganizationNode<T> getNewNodeInstance(ManagedObject theNode, double[] position, T userObject) 
	//	throws PositionNotAllowedException;
	
	/**
	 * Get the position of this node
	 * @param theNode
	 */
//	public Point3d getNodePosition(ManagedObject theNode);
	
	/**
	 * Get the position of this node
	 * @param theNode
	 * @deprecated
	 */
	//public double[] getNodePosition(ManagedObject theNode);
	
	/**
	 * Get the user object attached to this node
	 * @param theNode
	 */
	//public T getNodeUserObject(ManagedObject theNode);
	
	/**
	 * Get the volume of space reprasented by this node
	 * @param theNode
	 */
	//public double getNodeVolume(ManagedObject theNode);
	
	/**
	 * Move the specified node.
	 * @param theNode node to be moved
	 * @param pos The coordinates this node should move to.
	 */
//	public void moveNode( ManagedObject theNode, Point3d pos) throws PositionNotAllowedException;
	
	
	/**
	 * Move the specified node.
	 * @param theNode node to be moved
	 * @param delta The coordinates this node should move to.
	 */
	public void moveNodeFrom( NodeReference theNode, double[] delta) throws PositionNotAllowedException;
	
	
	/**
	 * Remove the specified node.
	 * @param theNode node to be removed
	 */
	//public void removeNode(ManagedObject theNode);
	
}
