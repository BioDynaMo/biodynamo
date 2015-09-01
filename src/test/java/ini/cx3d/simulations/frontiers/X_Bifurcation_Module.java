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

import static ini.cx3d.utilities.Matrix.randomNoise;
import ini.cx3d.Param;
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.localBiology.CellElement;
import ini.cx3d.localBiology.LocalBiologyModule;
import ini.cx3d.localBiology.NeuriteElement;
import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.physics.Substance;
import ini.cx3d.simulations.ECM;
import ini.cx3d.simulations.Scheduler;

import java.awt.Color;
import java.util.Vector;

/**
 * This class is used in Figure 8 : 
 * it defines a local biology module for bifurcation of the growth cone
 * @author fredericzubler
 *
 */
public class X_Bifurcation_Module implements LocalBiologyModule{
	
	public static int nbrOfGC = 0;
	
	/* The CellElement this module lives in.*/
	CellElement cellElement;
	/* Whether copied or not in branching.*/
	boolean copiedWhenNeuriteBranches = true;
	
	/* The chemical that activate branching.*/
	final Vector<String> branchingFactors = new Vector<String>();
	
	/* Slope of the probability to branch*/
	final public double slope;
	/* shift in the probability */
	public double shift;
	
	public double minConcentration = 0;
	public double maxProba = 1;
	
	//private double diameterOfDaughter = 0.9;
	private double diameterOfDaughter = 0.7;
	
	/* Stop if diameter smaller than this..*/
	public double minimalBranchDiameter = 0.0;
	

	/* minimum interval before branching */
	//double freeInterval = 5+ 15*ECM.getRandomDouble();
	double freeInterval = 5+ 1*ECM.getRandomDouble();
	
	public X_Bifurcation_Module() {
		nbrOfGC ++;
		slope = 0.03;
		shift = -0.01;
	}
	
	public X_Bifurcation_Module(double a,double b) {
		nbrOfGC++;
		this.slope = a;
		this.shift = b;
	}
	
	// --- LocalBiologyModule interface ----------------------------
	public CellElement getCellElement() {
		return cellElement;
	}

	public boolean isCopiedWhenNeuriteElongates() {
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
	public void setCopiedWhenNeuriteBranches(final boolean copiedWhenNeuriteBranches) {
		this.copiedWhenNeuriteBranches = copiedWhenNeuriteBranches;
	}

	final public boolean isCopiedWhenNeuriteExtendsFromSoma() {
		return false;
	}

	public boolean isCopiedWhenSomaDivides() {
		return false;
	}
	public boolean isDeletedAfterNeuriteHasBifurcated() {
		return false;
	}
	public void setCellElement(CellElement cellElement) {
		this.cellElement = cellElement;
	}
	
	public X_Bifurcation_Module getCopy(){
		X_Bifurcation_Module bf = new X_Bifurcation_Module(slope, shift);
		bf.branchingFactors.addAll(this.branchingFactors);
		bf.copiedWhenNeuriteBranches = this.copiedWhenNeuriteBranches;
		bf.maxProba = this.maxProba;
		bf.minConcentration = this.minConcentration;
		bf.diameterOfDaughter = this.diameterOfDaughter;
		bf.minimalBranchDiameter = this.minimalBranchDiameter;
		return bf; 
	}
	
	/** Add a chemical that will make this receptor branch.*/
	public void addBranchingFactor(String bf){
		branchingFactors.add(bf);
	}
	
	

	// --- LocalBiologyModule interface ----------------------------
	
	public void run() {
		
		PhysicalCylinder cyl = (PhysicalCylinder) cellElement.getPhysical();
		if(cyl.getActualLength()<freeInterval)
			return;
		
		// only terminal cylinders branch
		if(cyl.getDaughterLeft() != null)
			return;
		
		// if not too thin
		if(cyl.getDiameter()<minimalBranchDiameter){
			return;
		}
		
		double totalConcentration = 0.0;
		for (String s : branchingFactors) {
			double concentr = cyl.getExtracellularConcentration(s);
			totalConcentration += concentr;
		}
		
		if(totalConcentration<minConcentration)
			return;
		double y = slope*totalConcentration + shift;
		if(y>maxProba)
			y=maxProba;
		if(ECM.getRandomDouble()<y){
			NeuriteElement[] daughters =  ((NeuriteElement)cellElement).bifurcate();
			PhysicalCylinder cyl0 = daughters[0].getPhysicalCylinder();
			PhysicalCylinder cyl1 = daughters[1].getPhysicalCylinder();
			cyl0.setDiameter(cyl.getDiameter()*diameterOfDaughter);
			cyl1.setDiameter(cyl.getDiameter()*diameterOfDaughter);
		}
	}
	
	/** 1: no decrease in diameter, 0.5: decrease of fifty percent of the daughter branches.*/
	public void setDiameterOfDaughter(double diameterOfDaughter) {
		this.diameterOfDaughter = diameterOfDaughter;
	}
	
	/* Diameter under which it will never branch.*/
	public void setMinimalBranchDiameter(double minimalBranchDiameter) {
		this.minimalBranchDiameter = minimalBranchDiameter;
	}

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
		ecm.addArtificialGaussianConcentrationZ(new Substance("A", Color.red), 1, 300, 100); 
		ecm.addArtificialGaussianConcentrationZ(new Substance("B", Color.blue), 1, 00, 100);
		ecm.addArtificialGaussianConcentrationZ(new Substance("C", Color.green), 1, -300, 100);
		//		horizontal linerar gradient
		ecm.addArtificialLinearConcentrationZ(new Substance("D", Color.cyan), 1, 300, -300);
		//		vertical gaussian
		ecm.addArtificialGaussianConcentrationX(new Substance("E", Color.red), 1, 0, 100);
		ecm.addArtificialGaussianConcentrationX(new Substance("F", Color.green), 1, 300, 100);
		
		// 3) Create a 4-uple Cell-SomaElement-PhysicalSphere-SpaceNode at the desired location
		double[] cellLocation = new double[] {0,0,0};
		Cell cell = CellFactory.getCellInstance(cellLocation);
		cell.setColorForAllPhysicalObjects(Param.RED);
		
		// 4) Extend an axon from the cell
		NeuriteElement neurite = cell.getSomaElement().extendNewNeurite(new double[] {0,0,1});
		neurite.getPhysical().setDiameter(1.0);
		// 5) Put a movementReceptor
		X_Movement_Module mr = new X_Movement_Module();
		mr.addAttractant("A");
		neurite.addLocalBiologyModule(mr);
		X_Bifurcation_Module br = new X_Bifurcation_Module();
		br.addBranchingFactor("A");
		neurite.addLocalBiologyModule(br);
		System.out.println("BranchingReceptor.main()");
		// 6) Simulate
		Scheduler.simulate();
	}

	@Override
	public ini.cx3d.swig.StringBuilder simStateToJson(ini.cx3d.swig.StringBuilder sb) {
		sb.append("{}");
		return sb;
	}

}
