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

import java.io.Serializable;
import java.util.LinkedList;

/**
 * A simple implementation of {@link NodeToSOMAssignmentPolicy} that will assign nodes to 
 * SOMs solely based on their coordinate.
 * @author dennis
 *
 * @param <T>
 */
public class SimpleAssignmentPolicy<T> implements NodeToSOMAssignmentPolicy<T>, Serializable {
	/**
	 * A list of all SOMs that are known.
	 */
	LinkedList<SpatialOrganizationManager<T>> somList = new LinkedList<SpatialOrganizationManager<T>>();
	
	/**
	 * Defines into how many slices the space will be separated in each dimension.
	 */
	int[] spaceSeparation = new int[3];
	
	/**
	 * The dimensions of the working space. 
	 */
	double[][] spaceDimensions = new double[3][3];
	
	/**
	 * Creates a new instance of this assignment policy.
	 * @param minCoordinate The estimated minimum coordinate of the working space.
	 * @param maxCoordinate The estimated maximum coordinate of the working space.
	 */
	public SimpleAssignmentPolicy(double[] minCoordinate, double[] maxCoordinate) {
		for (int i = 0; i < spaceSeparation.length; i++) 
			spaceSeparation[i] = 1;
		for (int i = 0; i < spaceDimensions.length; i++) {
			spaceDimensions[i][0] = minCoordinate[i];
			spaceDimensions[i][1] = maxCoordinate[i];
			spaceDimensions[i][2] = maxCoordinate[i]-minCoordinate[i];
		}
	}
	
	/**
	 * Registers a new SOM. 
	 * Watch out! All SOMs that will be used have to be registered before starting the
	 * process! In addition, you have to register all SOM's for <b>ALL</b> instances of
	 * {@link SimpleAssignmentPolicy} in one program run in the same order!
	 * Otherwise, each single instance of this assignment policy will return different 
	 * SOM's for the same coordinate! 
	 * @param newSOM The {@link SpatialOrganizationManager} that should be registered.
	 */
	public void registerNewSOM(SpatialOrganizationManager<T> newSOM) {
		this.somList.addLast(newSOM);
		
		// separate the space into regions of same size, as cubic as possible:
		int leftOver = somList.size();
		LinkedList<Integer> factors = new LinkedList<Integer>();
		int current = 2;
		// find factors:
		while (leftOver > 1) {
			if (leftOver % current == 0) {
				factors.addFirst(new Integer(current));
				leftOver /= current;
			}
			else current++;
		}
		for (int i = 0; i < spaceSeparation.length; i++) 
			spaceSeparation[i] = 1;
		while (!factors.isEmpty()) {
			int min = 0;
			for (int i = 2; i < 3; i++) 
				if (spaceSeparation[i] < spaceSeparation[min])	min = i;
			spaceSeparation[min] *= factors.removeFirst().intValue();
		}
	}

	/* (non-Javadoc)
	 * @see ini.cx3d.parallelSpatialOrganization.NodeToSOMAssignmentPolicy#getResponsibleSOM(double[])
	 */
	public SpatialOrganizationManager<T> getResponsibleSOM(double[] coordinate) {
		// calculate the appropriate SOM:
		int position = 0;
		int stepSize = 1;
		for (int i = 0; i < 3; i++) {
			if (coordinate[i] >= spaceDimensions[i][0]) {
				if (coordinate[i] >= spaceDimensions[i][1]) 
					position += stepSize-1;
				else 
					position += stepSize*((int)((coordinate[i]-spaceDimensions[i][0])*spaceSeparation[i]/spaceDimensions[i][2]));
			}
			stepSize *= spaceSeparation[i];
		}
		return somList.get(position);
	}

}
