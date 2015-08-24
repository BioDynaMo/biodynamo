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

package ini.cx3d.synapses;

import static ini.cx3d.utilities.Matrix.*;
import ini.cx3d.localBiology.NeuriteElement;
import ini.cx3d.physics.PhysicalObject;
import ini.cx3d.physics.PhysicalSphere;
/**
 * General class for dendritic spines and axonal boutons
 * This class doesn't derive from PhysicalObject. It has thus no SpaceNode.
 * @author fredericzubler
 *
 */
public abstract class Excrescence {
	/** the physical object it is attached to.*/
	PhysicalObject po;
	/** the other structure with which it forms a synapse.*/ 
	Excrescence ex = null; 
	/** The position on the Physical Object where it's origin is.*/
	double[] positionOnPO;  // in polar coordinates
	/** length.*/
	double length = 1;
	/** spine or bouton*/
	int type;
	public static final int SPINE = 0;
	public static final int BOUTON = 1;
	public static final int SOMATICSPINE = 2;
	public static final int SHAFT = 3;
	
	/**
	 * Method to create a spine-bouton synapse.
	 * @param otherExcressence the other spine/bouton
	 * @param creatPhysicalBond is true, a PhysicalBond is made between the two respective
	 * PhysicalObjects possessing the Excrescences.
	 * 
	 * @return true if the synapse was performed correctly
	 */
	public abstract boolean synapseWith(Excrescence otherExcrescence, boolean createPhysicalBond);
	
	// Inserted by roman. This modifications allow additionally making synapses with somatic spines and with dendritic shafts.
	public abstract boolean synapseWithSoma(Excrescence otherExcrescence, boolean createPhysicalBond);
	public abstract boolean synapseWithShaft(NeuriteElement otherNe, double maxDis, int nrSegments, boolean createPhysicalBond);
	// The neurite Element to which the shaft synapse is made
	public NeuriteElement neShaft = null;
	// End: Changes by roman
	
	// dumb getters and setters .......................
	public Excrescence getEx() {
		return ex;
	}
	public void setEx(Excrescence ex) {
		this.ex = ex;
	}
	public double getLength() {
		return length;
	}
	public void setLength(double length) {
		this.length = length;
	}
	public PhysicalObject getPo() {
		return po;
	}
	public void setPo(PhysicalObject po) {
		this.po = po;
	}
	public double[] getPositionOnPO() {
		return positionOnPO;
	}
	public void setPositionOnPO(double[] positionOnPO) {
		this.positionOnPO = positionOnPO;
	}
	public int getType() {
		return type;
	}
	public void setType(int type) {
		this.type = type;
	}
	
	/** returns the absolute coord of the point where this element is attached on the PO.*/
	public double[] getProximalEnd(){
		return po.transformCoordinatesPolarToGlobal(positionOnPO);
	}
	
	/** returns the absolute coord of the point where this element ends.*/
	public double[] getDistalEnd(){
		double[] prox =  po.transformCoordinatesPolarToGlobal(positionOnPO);
		// if no synapse, defined by the length
		// if a synapse is made, this is the middle of the distance between the two attachment points
		if(ex==null){
			double[] axis = po.getUnitNormalVector(positionOnPO);
			return add(prox, scalarMult(length,axis));
		}else{
			// if a synapse is made, this is the middle of the distance between the two attachment points	
			return scalarMult(0.5, add(this.getProximalEnd(), ex.getProximalEnd()));
		}
	}
	
}
