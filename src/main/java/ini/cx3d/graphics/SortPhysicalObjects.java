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

package ini.cx3d.graphics;

import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.physics.PhysicalSphere;
import ini.cx3d.simulations.ECM;

import java.util.Comparator;

/** Class to sort object according to depth, so that we can display them in the
 * appropriate order.
 * @author sabina
 *
 */
public class SortPhysicalObjects implements Comparator{

	/* ECM reference */
	ECM ecm = ECM.getInstance();

	public void sortableObject(){
	}

	/**
	 * Compare method to evaluate the difference between the depth position of two 
	 * PhysicalObjects.
	 */
	public int compare(Object comparator1, Object comparator2){

		int result = 0;
		
		if (comparator1 instanceof PhysicalSphere & comparator2 instanceof PhysicalSphere){

			PhysicalSphere ps1 = (PhysicalSphere)comparator1;
			PhysicalSphere ps2 = (PhysicalSphere)comparator2;

			// Compare sphere depths
			if (ecm.getView().getDepth(ps1.getMassLocation())<ecm.getView().getDepth(ps2.getMassLocation())){
				result = -1;
			}
			else {
				result = 1;
			}
		}
		else if (comparator1 instanceof PhysicalCylinder & comparator2 instanceof PhysicalCylinder){

			PhysicalCylinder pc1 = (PhysicalCylinder)comparator1;
			PhysicalCylinder pc2 = (PhysicalCylinder)comparator2;
			
			// Compare sphere depths
			if (ecm.getView().getDepth(pc1.getMassLocation())<ecm.getView().getDepth(pc2.getMassLocation())){
				result = -1;
			}
			else {
				result = 1;
			}
		}
		
		
		
		return result;
	}
}


