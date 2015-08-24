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

package ini.cx3d.localBiology;

import static ini.cx3d.utilities.Matrix.add;
import static ini.cx3d.utilities.Matrix.dot;
import static ini.cx3d.utilities.Matrix.norm;
import static ini.cx3d.utilities.Matrix.perp3;
import static ini.cx3d.utilities.Matrix.randomNoise;
import static ini.cx3d.utilities.Matrix.rotAroundAxis;
import static ini.cx3d.utilities.Matrix.scalarMult;
import static ini.cx3d.utilities.Matrix.subtract;
import static ini.cx3d.utilities.Matrix.printlnLine;

import java.util.Vector;

import ini.cx3d.Param;
import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.physics.PhysicalObject;
import ini.cx3d.simulations.ECM;
import ini.cx3d.synapses.BiologicalBouton;
import ini.cx3d.synapses.BiologicalSpine;
import ini.cx3d.synapses.Excrescence;
import ini.cx3d.synapses.PhysicalBouton;
import ini.cx3d.synapses.PhysicalSpine;
import ini.cx3d.utilities.Matrix;

/**
 * Class defining the biological properties of a neurite segment, if it contains
 * a <code>LacalBiologyModule</code>. This class is associated with a <code>PhysicalCylinder</code>.
 * @author fredericzubler
 *
 */

public class NeuriteElement extends CellElement {

	/* The PhysicalObject this NeuriteElement is associated with.*/
	private PhysicalCylinder physicalCylinder = null;

	/* true if part of an axon, false if dendrite.*/
	private boolean isAnAxon = false;


	// *************************************************************************************
	//   Constructor & stuff
	// *************************************************************************************

	public NeuriteElement() {
		super();
		ecm.addNeuriteElement(this);
	}

	/** Note : doesn't copy the <code>LocalBiologyModule</code> list 
	 * (this is done in <code>branch()</code> etc.).*/
	public NeuriteElement getCopy() {
		NeuriteElement ne = new NeuriteElement();
		ne.isAnAxon = isAnAxon;
		ne.cell = this.cell;
		return ne;
	}


	/** <b>Users should not use this method!</b> 
	 * It is called by the physicalObject associated with this neuriteElement, when it is deleted.*/
	public void removeYourself(){
		// remove from the NeuriteElementList in ECM
		ecm.removeNeuriteElement(this);
		// Yet the SomaElement doesn't contain a list of the NeuriteElements
		// but if it does in the future, we'll have to remove this NeuriteElement from there also.
	}


	// *************************************************************************************
	//   Run
	// *************************************************************************************


	public void run() {
		// run local biological modules...
		runLocalBiologyModules();
	}

	// *************************************************************************************
	//   Movements
	// *************************************************************************************


	/** Retracts the Cylinder associated with this NeuriteElement, if it is a terminal one.
	 * @param speed the retraction speed in micron/h
	 */
	public void retractTerminalEnd(double speed) {
		physicalCylinder.retractCylinder(speed);
	}

	/** Moves the point mass of the Cylinder associated with this NeuriteElement, if it is a terminal one.
	 *  BUT : if "direction" points in an opposite direction than the cylinder axis, i.e.
	 *  if the dot product is negative, there is no movement (only elongation is possible).
	 * @param speed
	 * @param direction
	 */
	public void elongateTerminalEnd(double speed, double[] direction){
		physicalCylinder.extendCylinder(speed, direction);
	}


	// *************************************************************************************
	//   Branching & Bifurcating
	// *************************************************************************************

	/**
	 * Makes a side branch, i.e. splits this cylinder into two and puts a daughter right at the proximal half.
	 * @param newBranchDiameter
	 * @param growthDirection (But will be automatically corrected if not at least 45 degrees from the cylinder's axis).
	 * @return
	 */
	public NeuriteElement branch(double newBranchDiameter, double[] growthDirection) {
		// create a new NeuriteElement for side branch
		NeuriteElement ne = getCopy();
		// define direction if not defined
		if(growthDirection==null){
			growthDirection = perp3(add(physicalCylinder.getUnitaryAxisDirectionVector(), randomNoise(0.1, 3)));

		}
		// making the branching at physicalObject level
		PhysicalCylinder pc1 = physicalCylinder.branchCylinder(1.0, growthDirection);
		// linking biology and phyics
		ne.setPhysical(pc1);  // (this also sets the call back)
		// specifying the diameter we wanted
		pc1.setDiameter(newBranchDiameter);
		// 
		pc1.setBranchOrder(physicalCylinder.getBranchOrder() + 1);
		// TODO : Caution : doesn't change the value distally on the main branch

		// Copy of the local biological modules:
		for (LocalBiologyModule m : super.localBiologyModulesList) {
			if(m.isCopiedWhenNeuriteBranches()){
				LocalBiologyModule m2 = m.getCopy();
				ne.addLocalBiologyModule(m2);
			}
		}
		return ne;
	}

	/**
	 * Makes a side branch, i.e. splits this cylinder into two and puts a daughteRight at the proximal half.
	 * @param growthDirection (But will be automatically corrected if not at least 45 degrees from the cylinder's axis).
	 * @return
	 */
	public NeuriteElement branch(double[] growthDirection) {
		return branch(physicalCylinder.getDiameter(),growthDirection);
	}

	/**
	 * Makes a side branch, i.e. splits this cylinder into two and puts a daughter right at the proximal half.
	 * @param diameter of the side branch
	 * @return
	 */
	public NeuriteElement branch(double diameter) {
		double[] growthDirection = perp3(add(physicalCylinder.getUnitaryAxisDirectionVector(), randomNoise(0.1, 3)));
		growthDirection = Matrix.normalize(growthDirection);
		//		growthDirection = add(
		//				physicalCylinder.getUnitaryAxisDirectionVector(),
		//				scalarMult(1, growthDirection));
		return branch(diameter,growthDirection);
	}

	/**
	 * Makes a side branch, i.e. splits this cylinder into two and puts a daughter right at the proximal half.
	 * @param diameter of the side branch
	 * @return
	 */
	public NeuriteElement branch() {
		double newBranchDiameter = physicalCylinder.getDiameter();
		double[] growthDirection = perp3(add(physicalCylinder.getUnitaryAxisDirectionVector(), randomNoise(0.1, 3)));
		return branch(newBranchDiameter,growthDirection);
	}

	/**
	Returns <code>true</code> if it is a terminal cylinder with length of at least 1micron.
	 * @return
	 */
	public boolean bifurcationPermitted(){
		return physicalCylinder.bifurcationPermitted();
	}

	/**
	 * Bifurcation of a growth come (only works for terminal segments). 
	 * Note : angles are corrected if they are pointing backward.
	 * @param diameter_1  of new daughterLeft
	 * @param diameter_2 of new daughterRight
	 * @param direction_1
	 * @param direction_2
	 * @return
	 */
	public NeuriteElement[] bifurcate(
			double diameter_1, 
			double diameter_2,
			double[] direction_1,
			double[] direction_2) {
		return bifurcate(Param.NEURITE_DEFAULT_ACTUAL_LENGTH, diameter_1, diameter_2, direction_1,direction_2);
		
	}
	
	/**
	 * Bifurcation of a growth come (only works for terminal segments). 
	 * Note : angles are corrected if they are pointing backward.
	 * @param length of new branches
	 * @param diameter_1  of new daughterLeft
	 * @param diameter_2 of new daughterRight
	 * @param direction_1
	 * @param direction_2
	 * @return
	 */
	public NeuriteElement[] bifurcate(
			double length, 
			double diameter_1, 
			double diameter_2,
			double[] direction_1,
			double[] direction_2) {

		// 1) physical bifurcation
		PhysicalCylinder[] pc = physicalCylinder.bifurcateCylinder(length, direction_1, direction_2);
		// if bifurcation is not allowed...
		if(pc == null){
			(new RuntimeException("Bifurcation not allowed!")).printStackTrace();
			return null;
		}

		// 2) creating the first daughter branch
		NeuriteElement ne1 = getCopy();
		PhysicalCylinder pc1 = pc[0];
		ne1.setPhysical(pc1);
		pc1.setDiameter(diameter_1);
		pc1.setBranchOrder(physicalCylinder.getBranchOrder() + 1);

		// 3) the second one
		NeuriteElement ne2 = getCopy();
		PhysicalCylinder pc2 = pc[1];
		ne2.setPhysical(pc2);
		pc2.setDiameter(diameter_2);
		pc2.setBranchOrder(physicalCylinder.getBranchOrder() + 1);

		// 4) the local biological modules :
		for (int i = 0; i<super.localBiologyModulesList.size(); i++) {
			LocalBiologyModule m = super.localBiologyModulesList.get(i);
			// copy...
			if(m.isCopiedWhenNeuriteBranches()){
				// ...for the first neurite
				LocalBiologyModule m2 = m.getCopy();
				ne1.addLocalBiologyModule(m2);
				// ...for the second neurite
				m2 = m.getCopy();
				ne2.addLocalBiologyModule(m2);
			}
			// and remove
			if(m.isDeletedAfterNeuriteHasBifurcated()){
				super.localBiologyModulesList.remove(m);
			}
		}
		return new NeuriteElement[] {ne1, ne2};
	}

	/**
	 * 
	 * @param direction_1
	 * @param direction_2
	 * @return
	 */
	public NeuriteElement[] bifurcate(double [] direction_1, double [] direction_2) {
		// initial default length :
		double l = Param.NEURITE_DEFAULT_ACTUAL_LENGTH;
		// diameters :
		double d = physicalCylinder.getDiameter();

		return bifurcate(l, d, d, direction_1, direction_2);
	}

	/**
	 * 
	 * @return
	 */
	public NeuriteElement[] bifurcate() {
		// initial default length :
		double l = Param.NEURITE_DEFAULT_ACTUAL_LENGTH;
		// diameters :
		double d = physicalCylinder.getDiameter();
		// direction : (60 degrees between branches)
		double[] perpPlane = perp3(physicalCylinder.getSpringAxis());
		double angleBetweenTheBranches = Math.PI/3.0;
		double[] direction_1 = rotAroundAxis(physicalCylinder.getSpringAxis(), angleBetweenTheBranches*0.5, perpPlane);
		double[] direction_2 = rotAroundAxis(physicalCylinder.getSpringAxis(), -angleBetweenTheBranches*0.5, perpPlane);

		return bifurcate(l, d, d, direction_1, direction_2);

	}


	// *************************************************************************************
	//   Synapses
	// *************************************************************************************


	/**
	 * Makes spines (the physical and the biological part) on this NeuriteElement.
	 * @param interval the average interval between the boutons.
	 */
	public void makeSpines(double interval){
		// how many spines for this NeuriteElement ?
		double length = physicalCylinder.getActualLength();
		double spineOnThisSegment = length/interval;
		int nb = (int)Math.round(spineOnThisSegment);
		// TODO : better way to define number (ex : if interval >> length -> no spine at all) 
		for (int i = 0; i < nb; i++) {
			// create the physical part
			double[] coord = {length*ECM.getRandomDouble(), 6.28*ECM.getRandomDouble()};
			PhysicalSpine pSpine = new PhysicalSpine(physicalCylinder,coord,3);
			physicalCylinder.addExcrescence(pSpine);
			// create the biological part and set call backs
			BiologicalSpine bSpine = new BiologicalSpine();
			pSpine.setBiologicalSpine(bSpine);
			bSpine.setPhysicalSpine(pSpine);
		}
	}

	/**
	 * Makes a single spine (the physical and the biological part) randomly on this NeuriteElement.
	 */
	public void makeSingleSpine(){
		double length = physicalCylinder.getActualLength();
		// create the physical part
		double[] coord = {length*ECM.getRandomDouble(), 6.28*ECM.getRandomDouble()};
		PhysicalSpine pSpine = new PhysicalSpine(physicalCylinder,coord,3);
		physicalCylinder.addExcrescence(pSpine);
		// create the biological part and set call backs
		BiologicalSpine bSpine = new BiologicalSpine();
		pSpine.setBiologicalSpine(bSpine);
		bSpine.setPhysicalSpine(pSpine);
	}

	/**
	 * Makes a single spine (the physical and the biological part) randomly on this NeuriteElement.
	 */
	public void makeSingleSpine(double distFromProximalEnd){
		double length = physicalCylinder.getActualLength();
		if(distFromProximalEnd>length){
			System.out.println("NeuriteElement.makeSingleSpine(): no spine formed 'cause this cylinder is shorter" +
					"than "+distFromProximalEnd+" microns. I suggest you learn to count, dumbhead !");
			return;
		}
		// create the physical part
		double[] coord = {distFromProximalEnd, 6.28*ECM.getRandomDouble()};
		PhysicalSpine pSpine = new PhysicalSpine(physicalCylinder,coord,3);
		physicalCylinder.addExcrescence(pSpine);
		// create the biological part and set call backs
		BiologicalSpine bSpine = new BiologicalSpine();
		pSpine.setBiologicalSpine(bSpine);
		bSpine.setPhysicalSpine(pSpine);
	}

	/**
	 * Make boutons (the physical and the biological part) on this NeuriteElement.
	 * @param interval the average interval between the boutons.
	 */
	public void makeBoutons(double interval){
		// how many boutons for this NeuriteElement ?
		double length = physicalCylinder.getActualLength();
		double boutonsOnThisSegment = length/interval;
		int nb = (int)Math.round(boutonsOnThisSegment);		

		// TODO : better way to define number (ex : if interval >> length -> no spine at all) 
		for (int i = 0; i < nb; i++) {
			// create the physical part
			double[] coord = {length*ECM.getRandomDouble(), -3.14 + 6.28*ECM.getRandomDouble()};
			PhysicalBouton pBouton = new PhysicalBouton(physicalCylinder,coord,2);
			physicalCylinder.addExcrescence(pBouton);
			// create the biological part and set call backs
			BiologicalBouton bBouton = new BiologicalBouton();
			pBouton.setBiologicalBouton(bBouton);
			bBouton.setPhysicalBouton(pBouton);
		}
	}

	/**
	 * Adds one bouton (the physical and the biological part) randomly on this NeuriteElement.
	 */
	public void makeSingleBouton(double distFromProximalEnd){
		double length = physicalCylinder.getActualLength();
		if(distFromProximalEnd>length){
			System.out.println("NeuriteElement.makeSingleBouton(): no spine formed 'cause this cylinder is shorter" +
					"than "+distFromProximalEnd+" microns. I suggest you learn to count, dumbhead !");
			return;
		}
		// create the physical part
		double[] coord = {distFromProximalEnd, 6.28*ECM.getRandomDouble()};
		PhysicalBouton pBouton = new PhysicalBouton(physicalCylinder,coord,2);
		physicalCylinder.addExcrescence(pBouton);
		// create the biological part and set call backs
		BiologicalBouton bBouton = new BiologicalBouton();
		pBouton.setBiologicalBouton(bBouton);
		bBouton.setPhysicalBouton(pBouton);
	}
	
	/**
	 * Adds one bouton (the physical and the biological part) randomly on this NeuriteElement.
	 */
	public void makeSingleBouton(){
		// how many boutons for this NeuriteElement ?
		double length = physicalCylinder.getActualLength();
		// create the physical part
		double[] coord = {length*ECM.getRandomDouble(), -3.14 + 6.28*ECM.getRandomDouble()};
		PhysicalBouton pBouton = new PhysicalBouton(physicalCylinder,coord,2);
		physicalCylinder.addExcrescence(pBouton);
		// create the biological part and set call backs
		BiologicalBouton bBouton = new BiologicalBouton();
		pBouton.setBiologicalBouton(bBouton);
		bBouton.setPhysicalBouton(pBouton);
	}
	
	

	/**
	 * Links the free boutons of this neurite element to adjacents free spines
	 * @param probabilityToSynapse probability to make the link.
	 * @return
	 */
	public int synapseBetweenExistingBS(double probabilityToSynapse){
		int synapseMade = 0;

		for (PhysicalNode pn : physicalCylinder.getSoNode().getNeighbors()) {
			// For all PhysicalObjects around
			if(!pn.isAPhysicalObject()){
				continue;
			}
			// with a certain probability
			if(ECM.getRandomDouble()>probabilityToSynapse){
				continue;
			}


			PhysicalObject po = (PhysicalObject)pn;
			// for all Excrescence pair :
			ext:	for (Excrescence e1 : physicalCylinder.getExcrescences()) {
				// only if this one is a free bouton:
				if(e1.getEx()!=null || e1.getType()!=Excrescence.BOUTON){
					continue ext;
				}
				inner :		for (Excrescence e2 : po.getExcrescences()) {
					// only if the other is a free spine:
					if(e2.getEx()!=null || e2.getType() != Excrescence.SPINE){
						continue inner;
					}

					// Find origin of the two Excrescences
					double[] o1 = e1.getProximalEnd();
					double[] o2 = e2.getProximalEnd();
					// vector from o1 to o2
					double[] oo = subtract(o2,o1);
					// synapse possible only if close enough
					double distoo = norm(oo);
					double tol = 0;
					if(distoo > e1.getLength() + e2.getLength() + tol)
						continue inner;
					// synapse only possible if these two excresscences are pointing toward each other
					oo = scalarMult(1/distoo, oo); // normalize oo
					if(	dot(oo,physicalCylinder.getUnitNormalVector( e1.getPositionOnPO())) > 0 &&
							dot(oo,po.getUnitNormalVector( e2.getPositionOnPO())) < 0 )	
					{

						e1.synapseWith(e2, true);
						synapseMade++;
						continue ext; // if we made it, now we test the next one

					}

				}
			}

		}
		return synapseMade;
		// TODO : outer most loop should be e1 (if no excresscence, no check)
		// and : calculation of physical.getUnitNormalVector outside inner most loop
	}

	// *************************************************************************************
	//   Getters & Setters
	// *************************************************************************************


	public PhysicalObject getPhysical() {
		return physicalCylinder;
	}

	public void setPhysical(PhysicalObject physical) {
		this.physicalCylinder = (PhysicalCylinder) physical;
		this.physicalCylinder.setNeuriteElement(this); // callback
	}

	public PhysicalCylinder getPhysicalCylinder(){
		return (PhysicalCylinder)physicalCylinder;
	} 

	public void setPhysicalCylinder(PhysicalCylinder physicalcylinder){
		physicalCylinder = physicalcylinder;
		physicalCylinder.setNeuriteElement(this);
	}

	/** Returns true if this NeuriteElement is an axon. Hence false if it is a dendrite*/
	public boolean isAnAxon(){
		return isAnAxon;
	}

	/** True means that this NeuriteElement is an axon. Hence false means it is a dendrite*/
	public void setIsAnAxon(boolean isAnAxon){
		this.isAnAxon = isAnAxon;
	}

	@Override
	public boolean isASomaElement() {
		return false;
	}
	@Override
	public boolean isANeuriteElement(){
		return true;
	}

	/**
	 * @return the (first) distal <code>NeuriteElement</code>, if it exists,
	 * i.e. if this is not the terminal segment (otherwise returns <code>null</code>).  
	 */
	public NeuriteElement getDaughterLeft() {
		if (physicalCylinder.getDaughterLeft() == null) { 
			return null;
		} else {
			return physicalCylinder.getDaughterLeft().getNeuriteElement();
		}
	}

	/**
	 * @return the second distal <code>NeuriteElement</code>, if it exists 
	 * i.e. if there is a branching point just after this element (otherwise returns <code>null</code>).  
	 */
	public NeuriteElement getDaughterRight() {
		if (physicalCylinder.getDaughterRight() == null) { return null;
		} else {
			NeuriteElement ne = physicalCylinder.getDaughterRight().getNeuriteElement();
			return ne; 
		}
	}

	// *************************************************************************************
	//   Traverse Tree
	// *************************************************************************************

	/**
	 * Adds to a Vector of NeuriteElements (NE) all the NE distal to this particular NE (including it). 
	 * @param elements the vector where it should be added.
	 * @return
	 */
	public Vector<NeuriteElement> AddYourselfAndDistalNeuriteElements(Vector<NeuriteElement> elements){
		elements.add(this);
		NeuriteElement dL = getDaughterLeft();
		if(dL!=null){
			dL.AddYourselfAndDistalNeuriteElements(elements);
			NeuriteElement dR = getDaughterRight();
			if(dR!=null){
				dR.AddYourselfAndDistalNeuriteElements(elements);
			}
		}
		return elements;
	}

}
