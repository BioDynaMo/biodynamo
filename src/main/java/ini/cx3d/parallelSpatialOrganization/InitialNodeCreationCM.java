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

/**
 * 
 */
package ini.cx3d.parallelSpatialOrganization;

import ini.cx3d.spatialOrganization.PositionNotAllowedException;

/**
 * @author dennis
 *
 */
public class InitialNodeCreationCM<T> extends CacheManager<T> {

	T userObject; 
	double[] coordinate;
	ManagedObjectReference<T> insertionNodeReference;
	ManagedObjectReference<T> preliminaryNodeReference;	
	
	public InitialNodeCreationCM(T userObject, double[] coordinate,
			ManagedObjectReference<T> insertionNodeReference,
			ManagedObjectReference<T> preliminaryNodeReference,
			SpatialOrganizationManager<T> mySOM) {
		super(mySOM);
		this.preliminaryNodeReference = preliminaryNodeReference;
		this.insertionNodeReference = insertionNodeReference;
		this.coordinate = coordinate;
		this.userObject = userObject;
	}

	public String descriptionString() {
//		return "Initial Nodeinsertion at ["+coordinate[0]+", "+coordinate[1]+", "+coordinate[2]+"]";
		return "Initial Nodeinsertion #" +id;
	}

    protected void jobSuccessful() {
		preliminaryNodeReference.getSOM().removePendingNode(preliminaryNodeReference);
	}

	boolean executeTask() throws NodeLockedException {
		SpaceNode<T> insertionNode = null;
		while (insertionNode == null) {
			try {
				insertionNode = tryToGetNode(insertionNodeReference);
			} catch (ManagedObjectDoesNotExistException e) {
				insertionNode = null;
				insertionNodeReference = insertionNodeReference.getSOM().getANodeReference();
			}
		}
		
		// now, cm should own a locked copy of a starting point...
		try {
			SpaceNode<T> newNode = insertionNode.getNewInstance(coordinate, userObject, preliminaryNodeReference); 
			return true;
		} catch (PositionNotAllowedException e) {
			return false;
			// TODO: very annoying... need a new treatment for PositionNotAllowed-cases
		} catch (ManagedObjectDoesNotExistException e) {
			throw new RuntimeException("A modifying process tried to access a managed object that didn't exist!");
		}
	}
	
	@Override
	public int getPriority() {
		return 0;
	}
	
	
}
