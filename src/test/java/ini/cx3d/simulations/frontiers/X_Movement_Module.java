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

package ini.cx3d.simulations.frontiers;

import static ini.cx3d.utilities.Matrix.add;
import static ini.cx3d.utilities.Matrix.normalize;
import static ini.cx3d.utilities.Matrix.randomNoise;
import static ini.cx3d.utilities.Matrix.scalarMult;
import ini.cx3d.Param;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.localBiology.interfaces.CellElement;
import ini.cx3d.physics.interfaces.PhysicalCylinder;
import ini.cx3d.physics.factory.SubstanceFactory;
import ini.cx3d.simulations.ECM;
import ini.cx3d.simulations.Scheduler;

import java.awt.Color;
import java.util.Vector;

/**
 * This class is used in Figure 8 : 
 * it defines a local module for moving the growth cone up a gradient
 * @author fredericzubler
 *
 */
public class X_Movement_Module extends ini.cx3d.swig.biology.LocalBiologyModule{

	/* The CellElement this module lives in.*/
	CellElement cellElement;
	/* Whether copied or not in branching.*/
	boolean copiedWhenNeuriteBranches = true;

	/* The chemical this Receptor is attracted by.*/
	Vector<String> attractants = new Vector<String>();

	/* The chemical this Receptor is repelled by.*/
	Vector<String> repellents = new Vector<String>();

	/* The chemical this Receptor dies if it goes opposite to.*/
	String cantLeave = null;
	double maxConcentration;

	/* Stop if diameter smaller than this..*/
	public double minimalBranchDiameter = 0.7;

	private double linearDiameterDecrease = 0.001;

	double[] movementDirection = {0,0,0};
	double randomness = 0.3;
	double directionWeight = 0.1;

	double speed = 100;

	// getters and setters
	public X_Movement_Module() {
		registerJavaObject(this);
	}

	public double getMaxConcentration() {
		return maxConcentration;
	}

	public void setMaxConcentration(double maxConcentration) {
		this.maxConcentration = maxConcentration;
	}

	public double getMinimalBranchDiameter() {
		return minimalBranchDiameter;
	}

	public void setMinimalBranchDiameter(double minimalBranchDiameter) {
		this.minimalBranchDiameter = minimalBranchDiameter;
	}

	public double getLinearDiameterDecrease() {
		return linearDiameterDecrease;
	}

	public void setLinearDiameterDecrease(double linearDiameterDecrease) {
		this.linearDiameterDecrease = linearDiameterDecrease;
	}

	public double getRandomness() {
		return randomness;
	}

	public void setRandomness(double randomness) {
		this.randomness = randomness;
	}
	

	public double getSpeed() {
		return speed;
	}

	public void setSpeed(double speed) {
		this.speed = speed;
	}

	// --- LocalBiologyModule interface ----------------------------
	public CellElement getCellElement() {
		return cellElement;
	}


	public boolean isCopiedWhenNeuriteElongates() {
		return false;
	}
	
	public boolean isDeletedAfterNeuriteHasBifurcated() {
		return false;
	}
	
	public boolean isCopiedWhenNeuriteBranches() {
		return copiedWhenNeuriteBranches;
	}

	/**
	 * Specifies if this receptor is copied.
	 * @param copiedWhenNeuriteBranches
	 */
	// This method is not part of the LocalBiologyModule
	public void setCopiedWhenNeuriteBranches(boolean copiedWhenNeuriteBranches) {
		this.copiedWhenNeuriteBranches = copiedWhenNeuriteBranches;
	}

	public boolean isCopiedWhenNeuriteExtendsFromSoma() {
		return false;
	}

	public boolean isCopiedWhenSomaDivides() {
		return false;
	}

	public void setCellElement(CellElement cellElement) {
		this.cellElement = cellElement;
		this.movementDirection = cellElement.getPhysical().getXAxis();
	}

	public X_Movement_Module getCopy(){
		X_Movement_Module r = new X_Movement_Module();
		r.attractants = (Vector<String>) this.attractants.clone();
		r.repellents = (Vector<String>) this.repellents.clone();
		r.randomness = this.randomness;
		r.movementDirection = this.movementDirection.clone();
		r.directionWeight = this.directionWeight;
		r.copiedWhenNeuriteBranches = this.copiedWhenNeuriteBranches;
		r.minimalBranchDiameter = this.minimalBranchDiameter;
		r.cantLeave = this.cantLeave;
		r.maxConcentration = this.maxConcentration;
		r.linearDiameterDecrease = this.linearDiameterDecrease;
		return r; 
	}



	/** Add a chemical this receptor will follows.*/
	public void addAttractant(String attractant){
		attractants.add(attractant);
	}

	/** Add a chemical this receptor will avoid.*/
	public void addRepellent(String repellent){
		repellents.add(repellent);
	}



	// Run ========================================================================

	public void run() {

		PhysicalCylinder cyl = (PhysicalCylinder) cellElement.getPhysical();
		
		// Juste temporary for getting a picture :
//		if(cyl.getMassLocation()[2]<-100){
//			return;
//		}
		
		
		double lengthBefore = cyl.getLength();
		// not to thin?
		if(cyl.getDiameter()<minimalBranchDiameter){
			if(cyl.lengthToProximalBranchingPoint()>7+10*ECM.getRandomDouble()){ // so we don't end with short segments
				return;
			}
		}

		// can't leave
		if(cantLeave != null){
			double currentCantLeaveConcntration = cyl.getExtracellularConcentration(cantLeave);
			if(currentCantLeaveConcntration>maxConcentration){
				maxConcentration = currentCantLeaveConcntration;
			}
			if(currentCantLeaveConcntration<0.95*maxConcentration){
				cyl.setColor(Color.black);
				return;
			}
		}

		// find the gradients
		double[] totalGradient = {0.0, 0.0, 0.0};
		for (String s : attractants) {
			double[] normalizedGrad = normalize(cyl.getExtracellularGradient(s));
			totalGradient = add(totalGradient, normalizedGrad);
		}
		for (String s : repellents) {
			double[] normalizedAntiGrad = scalarMult(-1.0,normalize(cyl.getExtracellularGradient(s)));
			totalGradient = add(totalGradient, normalizedAntiGrad);
		}
		// if no gradient : go straight
		if(attractants.isEmpty() && repellents.isEmpty()){
			totalGradient = movementDirection;
		}

		double[] newStepDirection = add(movementDirection,
				scalarMult(directionWeight, totalGradient),
				randomNoise(randomness,3));

		cyl.movePointMass(speed, newStepDirection);


		movementDirection = normalize(add(scalarMult(5,movementDirection),newStepDirection));

		// decrease diameter setDiameter
		double lengthAfter = cyl.getLength();
		double deltaL = lengthAfter-lengthBefore;
		if(deltaL<0)
			deltaL=0;
		cyl.setDiameter(cyl.getDiameter()*(1-deltaL*linearDiameterDecrease));
	}

	// ==============================================================================================

	public static void main(String[] args) {
		// 1) Prepare the environment :
		// 		get a reference to the extracelular matrix (ECM)
		ECM ecm = ECM.getInstance();
		// 		add additional PhysicalNodes (for diffusion)
		int nbOfAdditionalNodes = 100;
		for (int i = 0; i < nbOfAdditionalNodes; i++) {
			double[] coord = randomNoise(500, 3);
			ecm.getPhysicalNodeInstance(coord);
		}

		// 2) Create some artificial chemical gradients
		// 		horizontal gaussian (peak concentration, peak coordinate, variance)
		ecm.addArtificialGaussianConcentrationZ(SubstanceFactory.create("A", Color.red), 1, 300, 100);
		ecm.addArtificialGaussianConcentrationZ(SubstanceFactory.create("B", Color.blue), 1, 00, 100);
		ecm.addArtificialGaussianConcentrationZ(SubstanceFactory.create("C", Color.green), 1, -300, 100);
		//		horizontal linerar gradient
		ecm.addArtificialLinearConcentrationZ(SubstanceFactory.create("D", Color.cyan), 1, 300, -300);
		//		vertical gaussian
		ecm.addArtificialGaussianConcentrationX(SubstanceFactory.create("E", Color.red), 1, 0, 100);
		ecm.addArtificialGaussianConcentrationX(SubstanceFactory.create("F", Color.green), 1, 300, 100);

		// 3) Create a 4-uple Cell-SomaElement-PhysicalSphere-SpaceNode at the desired location
		double[] cellLocation = new double[] {0,0,0};
		ini.cx3d.cells.interfaces.Cell cell = CellFactory.getCellInstance(cellLocation);
		cell.setColorForAllPhysicalObjects(Param.RED);

		// 4) Extend an axon from the cell
		ini.cx3d.localBiology.interfaces.NeuriteElement neurite = cell.getSomaElement().extendNewNeurite(new double[] {0,0,1});
		neurite.getPhysical().setDiameter(1.0);
		neurite.getPhysicalCylinder().setDiameter(3);
		// 5) Put a movementReceptor
		X_Movement_Module r = new X_Movement_Module();
		r.linearDiameterDecrease = 0.01;
		r.addAttractant("A");

		neurite.addLocalBiologyModule(r);

		// 6) Simulate
		Scheduler.simulate();
	}

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		sb.append("{}");
		return sb;
	}

}
