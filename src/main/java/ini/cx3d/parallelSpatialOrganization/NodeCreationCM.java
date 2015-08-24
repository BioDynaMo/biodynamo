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

import ini.cx3d.spatialOrganization.PositionNotAllowedException;

public class NodeCreationCM<T> extends CacheManager<T> {
	T userObject; 
	double[] coordinate;
	ManagedObjectReference<T> startingTetrahedron;
	ManagedObjectReference<T> nodeReference;	
	
	public NodeCreationCM(T userObject, double[] coordinate,
			ManagedObjectReference<T> insertionSite,
			ManagedObjectReference<T> preliminaryNodeReference,
			SpatialOrganizationManager<T> mySOM) {
		super(mySOM);
		this.nodeReference = preliminaryNodeReference;
		this.startingTetrahedron = insertionSite;
		this.coordinate = coordinate;
		this.userObject = userObject;
	}
	
    protected void jobSuccessful() {
		nodeReference.getSOM().removePendingNode(nodeReference);
	}

	public String descriptionString() {
		return "Nodeinsertion #"+id;
//		return "Nodeinsertion at ["+coordinate[0]+", "+coordinate[1]+", "+coordinate[2]+"]";
	}
    
	boolean executeTask() throws NodeLockedException {
		Tetrahedron<T> insertionSite = null;
		while (insertionSite == null) {
			SpatialOrganizationManager<T> lastSOM = null;
			// if visibilityWalk returns a tetrahedron reference that belongs to the same SOM as the tetrahedron that was given as parameter, 
			// the visibilityWalk has ended: 
			while ((lastSOM == null) || (!lastSOM.belongsToThisSOM(startingTetrahedron))) {
				lastSOM = startingTetrahedron.getSOM();
				startingTetrahedron = startingTetrahedron.getSOM().visibilityWalk(coordinate, startingTetrahedron);
			}
			try {
				insertionSite = tryToGetTetrahedron(startingTetrahedron);
			} catch (ManagedObjectDoesNotExistException e) {
				insertionSite = null;
				System.out.println("Watch out! I had to restart the search for an insertion site!");
				startingTetrahedron = startingTetrahedron.getSOM().getATetrahedronReference();
			}
		}
		// now, cm should own a locked copy of a starting point...
		SpaceNode<T> newNode = new SpaceNode<T>(coordinate, userObject, this, nodeReference);
		try {
			Triangle3D<T>[] triangles = insertionSite.getAdjacentTriangles();
			SpaceNode<T>[] opposites = new SpaceNode[4];
			for (int i = 0; i < opposites.length; i++) {
				if (!triangles[i].isInfinite()) {
					opposites[i] = triangles[i].getOppositeTetrahedron(insertionSite).getOppositeNode(triangles[i]);
					if (opposites[i] != null) {
					triangles[i].update();
					if (triangles[i].orientation(opposites[i].getPosition(), coordinate) > 0)
						System.out.println("the point doesn't lie on the inside of the selected tetrahedron!");
					}
				}
			}
			newNode.insert(insertionSite);
			return true;
		} catch (PositionNotAllowedException e) {
			return false;
			// TODO: very annoying... need a new treatment for PositionNotAllowed-cases
		} catch (ManagedObjectDoesNotExistException e) {
			throw new RuntimeException("A modifying process tried to access a managed object that didn't exist!");
		}
//		mySOM.addInsertionActionAtTetrahedron(userObject, coordinate, nodeReference, startingTetrahedron);
	}
	
	@Override
	public int getPriority() {
		return 0;
	}

}
