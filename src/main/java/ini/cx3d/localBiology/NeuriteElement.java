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

import static ini.cx3d.SimStateSerializationUtil.keyValue;
import static ini.cx3d.SimStateSerializationUtil.removeLastChar;
import static ini.cx3d.utilities.Matrix.add;
import static ini.cx3d.utilities.Matrix.dot;
import static ini.cx3d.utilities.Matrix.norm;
import static ini.cx3d.utilities.Matrix.perp3;
import static ini.cx3d.utilities.Matrix.randomNoise;
import static ini.cx3d.utilities.Matrix.rotAroundAxis;
import static ini.cx3d.utilities.Matrix.scalarMult;
import static ini.cx3d.utilities.Matrix.subtract;
import static ini.cx3d.utilities.Matrix.printlnLine;
import static ini.cx3d.utilities.StringUtilities.toStr;

import java.util.AbstractSequentialList;

import ini.cx3d.JavaUtil2;
import ini.cx3d.Param;
import ini.cx3d.physics.interfaces.PhysicalCylinder;
import ini.cx3d.physics.interfaces.PhysicalNode;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.synapses.factory.PhysicalBoutonFactory;
import ini.cx3d.synapses.factory.PhysicalSpineFactory;
import ini.cx3d.synapses.interfaces.BiologicalSpine;
import ini.cx3d.synapses.factory.BiologicalBoutonFactory;
import ini.cx3d.synapses.factory.BiologicalSpineFactory;
import ini.cx3d.synapses.interfaces.Excrescence;
import ini.cx3d.synapses.PhysicalBouton;
import ini.cx3d.synapses.PhysicalSpine;
import ini.cx3d.utilities.Matrix;

/**
 * Class defining the biological properties of a neurite segment, if it contains
 * a <code>LacalBiologyModule</code>. This class is associated with a <code>PhysicalCylinder</code>.
 * @author fredericzubler
 *
 */

public class NeuriteElement extends ini.cx3d.swig.biology.biology.NeuriteElementBase implements ini.cx3d.localBiology.interfaces.NeuriteElement {

	static {
		ini.cx3d.swig.biology.CellElement.setECM(ECMFacade.getInstance());
	}

	/* The PhysicalObject this NeuriteElement is associated with.*/
	private PhysicalCylinder physicalCylinder = null;

	/* true if part of an axon, false if dendrite.*/
	private boolean isAnAxon = false;

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		superSuperSimStateToJson(sb);

		keyValue(sb, "physicalCylinder", physicalCylinder);
		keyValue(sb, "isAnAxon", Boolean.toString(isAnAxon));

		removeLastChar(sb);
		sb.append("}");
		return sb;
	}

	// *************************************************************************************
	//   Constructor & stuff
	// *************************************************************************************

	public NeuriteElement() {
		super();
		ini.cx3d.swig.biology.CellElement.registerJavaObject(this);
		ini.cx3d.swig.biology.NeuriteElement.registerJavaObject(this);
		ECMFacade.getInstance().addNeuriteElement(this);
	}

	/** Note : doesn't copy the <code>LocalBiologyModule</code> list 
	 * (this is done in <code>branch()</code> etc.).*/
	public ini.cx3d.localBiology.interfaces.NeuriteElement getCopy() {
		NeuriteElement ne = new NeuriteElement();
		ne.isAnAxon = isAnAxon;
		ne.setCell(getCell());
		return ne;
	}


	/** <b>Users should not use this method!</b> 
	 * It is called by the physicalObject associated with this neuriteElement, when it is deleted.*/
	public void removeYourself(){
		// remove from the NeuriteElementList in ECM
		ECMFacade.getInstance().removeNeuriteElement(this);
		// Yet the SomaElement doesn't contain a list of the NeuriteElements
		// but if it does in the future, we'll have to remove this NeuriteElement from there also.
	}


	// *************************************************************************************
	//   Run
	// *************************************************************************************


	@Override
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
	@Override
	public void retractTerminalEnd(double speed) {
		physicalCylinder.retractCylinder(speed);
	}

	/** Moves the point mass of the Cylinder associated with this NeuriteElement, if it is a terminal one.
	 *  BUT : if "direction" points in an opposite direction than the cylinder axis, i.e.
	 *  if the dot product is negative, there is no movement (only elongation is possible).
	 * @param speed
	 * @param direction
	 */
	@Override
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
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement branch(double newBranchDiameter, double[] growthDirection) {
		// create a new NeuriteElement for side branch
		ini.cx3d.localBiology.interfaces.NeuriteElement ne = getCopy();
		// define direction if not defined
		if(growthDirection==null){
			growthDirection = perp3(add(physicalCylinder.getUnitaryAxisDirectionVector(), randomNoise(0.1, 3)));
			throw new RuntimeException("foo");
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
		for (LocalBiologyModule m : getLocalBiologyModulesList()) {
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
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement branch(double[] growthDirection) {
		return branch(physicalCylinder.getDiameter(),growthDirection);
	}

	/**
	 * Makes a side branch, i.e. splits this cylinder into two and puts a daughter right at the proximal half.
	 * @param diameter of the side branch
	 * @return
	 */
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement branch(double diameter) {
		double[] growthDirection = perp3(add(physicalCylinder.getUnitaryAxisDirectionVector(), randomNoise(0.1, 3)));
		growthDirection = Matrix.normalize(growthDirection);
		//		growthDirection = add(
		//				physicalCylinder.getUnitaryAxisDirectionVector(),
		//				scalarMult(1, growthDirection));
		return branch(diameter,growthDirection);
	}

	/**
	 * Makes a side branch, i.e. splits this cylinder into two and puts a daughter right at the proximal half.
	 * @return
	 */
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement branch() {
		double newBranchDiameter = physicalCylinder.getDiameter();
		double[] rand_noise = randomNoise(0.1, 3);
		double[] growthDirection = perp3(add(physicalCylinder.getUnitaryAxisDirectionVector(), rand_noise));
		return branch(newBranchDiameter,growthDirection);
	}

	/**
	Returns <code>true</code> if it is a terminal cylinder with length of at least 1micron.
	 * @return
	 */
	@Override
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
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement[] bifurcate(
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
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement[] bifurcate(
			double length,
			double diameter_1,
			double diameter_2,
			double[] direction_1,
			double[] direction_2) {
		// 1) physical bifurcation
		PhysicalCylinder[] pc = physicalCylinder.bifurcateCylinder(length, direction_1, direction_2);
		// if bifurcation is not allowed...
		if(pc == null){
			throw new RuntimeException();
//			(new RuntimeException("Bifurcation not allowed!")).printStackTrace();
//			return null;
		}

		// 2) creating the first daughter branch
		ini.cx3d.localBiology.interfaces.NeuriteElement ne1 = getCopy();
		PhysicalCylinder pc1 = pc[0];
		ne1.setPhysical(pc1);
		pc1.setDiameter(diameter_1);
		pc1.setBranchOrder(physicalCylinder.getBranchOrder() + 1);

		// 3) the second one
		ini.cx3d.localBiology.interfaces.NeuriteElement ne2 = getCopy();
		PhysicalCylinder pc2 = pc[1];
		ne2.setPhysical(pc2);
		pc2.setDiameter(diameter_2);
		pc2.setBranchOrder(physicalCylinder.getBranchOrder() + 1);

		// 4) the local biological modules :
		for (int i = 0; i<getLocalBiologyModulesList().size(); i++) {
			LocalBiologyModule m = getLocalBiologyModulesList().get(i);
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
				removeLocalBiologyModule(m);
			}
		}
		return new ini.cx3d.localBiology.interfaces.NeuriteElement[] {ne1, ne2};
	}

	/**
	 * 
	 * @param direction_1
	 * @param direction_2
	 * @return
	 */
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement[] bifurcate(double[] direction_1, double[] direction_2) {
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
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement[] bifurcate() {
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
	@Override
	public void makeSpines(double interval){
		// how many spines for this NeuriteElement ?
		double length = physicalCylinder.getActualLength();
		double spineOnThisSegment = length/interval;
		int nb = (int)Math.round(spineOnThisSegment);
		// TODO : better way to define number (ex : if interval >> length -> no spine at all)
		for (int i = 0; i < nb; i++) {
			// create the physical part
			double[] coord = {length* JavaUtil2.getRandomDouble(), 6.28*JavaUtil2.getRandomDouble()};
			ini.cx3d.synapses.interfaces.PhysicalSpine pSpine = PhysicalSpineFactory.create(physicalCylinder,coord,3);
			physicalCylinder.addExcrescence(pSpine);
			// create the biological part and set call backs
			BiologicalSpine bSpine = BiologicalSpineFactory.create();
			pSpine.setBiologicalSpine(bSpine);
			bSpine.setPhysicalSpine(pSpine);
		}
	}

	/**
	 * Makes a single spine (the physical and the biological part) randomly on this NeuriteElement.
	 */
	@Override
	public void makeSingleSpine(){
		double length = physicalCylinder.getActualLength();
		// create the physical part
		double[] coord = {length*JavaUtil2.getRandomDouble(), 6.28*JavaUtil2.getRandomDouble()};
		ini.cx3d.synapses.interfaces.PhysicalSpine pSpine = PhysicalSpineFactory.create(physicalCylinder, coord, 3);
		physicalCylinder.addExcrescence(pSpine);
		// create the biological part and set call backs
		BiologicalSpine bSpine = BiologicalSpineFactory.create();
		pSpine.setBiologicalSpine(bSpine);
		bSpine.setPhysicalSpine(pSpine);
	}

	/**
	 * Makes a single spine (the physical and the biological part) randomly on this NeuriteElement.
	 */
	@Override
	public void makeSingleSpine(double distFromProximalEnd){
		double length = physicalCylinder.getActualLength();
		if(distFromProximalEnd>length){
			System.out.println("NeuriteElement.makeSingleSpine(): no spine formed 'cause this cylinder is shorter" +
					"than "+distFromProximalEnd+" microns. I suggest you learn to count, dumbhead !");
			return;
		}
		// create the physical part
		double[] coord = {distFromProximalEnd, 6.28*JavaUtil2.getRandomDouble()};
		ini.cx3d.synapses.interfaces.PhysicalSpine pSpine = PhysicalSpineFactory.create(physicalCylinder,coord,3);
		physicalCylinder.addExcrescence(pSpine);
		// create the biological part and set call backs
		BiologicalSpine bSpine = BiologicalSpineFactory.create();
		pSpine.setBiologicalSpine(bSpine);
		bSpine.setPhysicalSpine(pSpine);
	}

	/**
	 * Make boutons (the physical and the biological part) on this NeuriteElement.
	 * @param interval the average interval between the boutons.
	 */
	@Override
	public void makeBoutons(double interval){
		// how many boutons for this NeuriteElement ?
		double length = physicalCylinder.getActualLength();
		double boutonsOnThisSegment = length/interval;
		int nb = (int)Math.round(boutonsOnThisSegment);

		// TODO : better way to define number (ex : if interval >> length -> no spine at all) 
		for (int i = 0; i < nb; i++) {
			// create the physical part
			double[] coord = {length*JavaUtil2.getRandomDouble(), -3.14 + 6.28*JavaUtil2.getRandomDouble()};
			ini.cx3d.synapses.interfaces.PhysicalBouton pBouton = PhysicalBoutonFactory.create(physicalCylinder, coord, 2);
			physicalCylinder.addExcrescence(pBouton);
			// create the biological part and set call backs
			ini.cx3d.synapses.interfaces.BiologicalBouton bBouton = BiologicalBoutonFactory.create();
			pBouton.setBiologicalBouton(bBouton);
			bBouton.setPhysicalBouton(pBouton);
		}
	}

	/**
	 * Adds one bouton (the physical and the biological part) randomly on this NeuriteElement.
	 */
	@Override
	public void makeSingleBouton(double distFromProximalEnd){
		double length = physicalCylinder.getActualLength();
		if(distFromProximalEnd>length){
			System.out.println("NeuriteElement.makeSingleBouton(): no spine formed 'cause this cylinder is shorter" +
					"than "+distFromProximalEnd+" microns. I suggest you learn to count, dumbhead !");
			return;
		}
		// create the physical part
		double[] coord = {distFromProximalEnd, 6.28*JavaUtil2.getRandomDouble()};
		ini.cx3d.synapses.interfaces.PhysicalBouton pBouton = PhysicalBoutonFactory.create(physicalCylinder,coord,2);
		physicalCylinder.addExcrescence(pBouton);
		// create the biological part and set call backs
		ini.cx3d.synapses.interfaces.BiologicalBouton bBouton = BiologicalBoutonFactory.create();
		pBouton.setBiologicalBouton(bBouton);
		bBouton.setPhysicalBouton(pBouton);
	}
	
	/**
	 * Adds one bouton (the physical and the biological part) randomly on this NeuriteElement.
	 */
	@Override
	public void makeSingleBouton(){
		// how many boutons for this NeuriteElement ?
		double length = physicalCylinder.getActualLength();
		// create the physical part
		double[] coord = {length*JavaUtil2.getRandomDouble(), -3.14 + 6.28*JavaUtil2.getRandomDouble()};
		ini.cx3d.synapses.interfaces.PhysicalBouton pBouton = PhysicalBoutonFactory.create(physicalCylinder,coord,2);
		physicalCylinder.addExcrescence(pBouton);
		// create the biological part and set call backs
		ini.cx3d.synapses.interfaces.BiologicalBouton bBouton = BiologicalBoutonFactory.create();
		pBouton.setBiologicalBouton(bBouton);
		bBouton.setPhysicalBouton(pBouton);
	}
	
	

	/**
	 * Links the free boutons of this neurite element to adjacents free spines
	 * @param probabilityToSynapse probability to make the link.
	 * @return
	 */
	@Override
	public int synapseBetweenExistingBS(double probabilityToSynapse){
		int synapseMade = 0;

		AbstractSequentialList<PhysicalNode> neighbors = physicalCylinder.getSoNode().getNeighbors();
		for (int i = 0; i < neighbors.size(); i++) {
			ini.cx3d.physics.interfaces.PhysicalNode pn = neighbors.get(i);
			// For all PhysicalObjects around
			if(!pn.isAPhysicalObject()){
				continue;
			}
			// with a certain probability
			if(JavaUtil2.getRandomDouble()>probabilityToSynapse){
				continue;
			}


			ini.cx3d.physics.interfaces.PhysicalObject po = (ini.cx3d.physics.interfaces.PhysicalObject)pn;
			// for all Excrescence pair :
			ext:	for (ini.cx3d.synapses.interfaces.Excrescence e1 : physicalCylinder.getExcrescences()) {
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
					double[] e1Pos = e1.getPositionOnPO();
					double[] e2Pos = e2.getPositionOnPO();
					if(	dot(oo,physicalCylinder.getUnitNormalVector( new double[]{e1Pos[0], e1Pos[1], 0.0})) > 0 &&
							dot(oo,po.getUnitNormalVector( new double[]{e2Pos[0], e2Pos[1], 0.0})) < 0 )
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


	@Override
	public ini.cx3d.physics.interfaces.PhysicalObject getPhysical() {
		return physicalCylinder;
	}

	@Override
	public void setPhysical(ini.cx3d.physics.interfaces.PhysicalObject physical) {
		this.physicalCylinder = (PhysicalCylinder) physical;
		this.physicalCylinder.setNeuriteElement(this); // callback
	}

	@Override
	public PhysicalCylinder getPhysicalCylinder(){
		return (PhysicalCylinder)physicalCylinder;
	} 

	@Override
	public void setPhysicalCylinder(PhysicalCylinder physicalcylinder){
		physicalCylinder = physicalcylinder;
		physicalCylinder.setNeuriteElement(this);
	}

	/** Returns true if this NeuriteElement is an axon. Hence false if it is a dendrite*/
	@Override
	public boolean isAxon(){
		return isAnAxon;
	}

	/** True means that this NeuriteElement is an axon. Hence false means it is a dendrite*/
	@Override
	public void setAxon(boolean isAnAxon){
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
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement getDaughterLeft() {
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
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement getDaughterRight() {
		if (physicalCylinder.getDaughterRight() == null) { return null;
		} else {
			ini.cx3d.localBiology.interfaces.NeuriteElement ne = physicalCylinder.getDaughterRight().getNeuriteElement();
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
	@Override
	public AbstractSequentialList<ini.cx3d.localBiology.interfaces.NeuriteElement> addYourselfAndDistalNeuriteElements(AbstractSequentialList<ini.cx3d.localBiology.interfaces.NeuriteElement> elements){
		elements.add(this);
		ini.cx3d.localBiology.interfaces.NeuriteElement dL = getDaughterLeft();
		if(dL!=null){
			dL.addYourselfAndDistalNeuriteElements(elements);
			ini.cx3d.localBiology.interfaces.NeuriteElement dR = getDaughterRight();
			if(dR!=null){
				dR.addYourselfAndDistalNeuriteElements(elements);
			}
		}
		return elements;
	}

}
