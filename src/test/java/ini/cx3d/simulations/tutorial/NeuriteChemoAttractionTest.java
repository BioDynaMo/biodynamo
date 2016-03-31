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

package ini.cx3d.simulations.tutorial;

import static ini.cx3d.utilities.Matrix.add;
import static ini.cx3d.utilities.Matrix.normalize;
import static ini.cx3d.utilities.Matrix.randomNoise;
import static ini.cx3d.utilities.Matrix.scalarMult;

import ini.cx3d.BaseSimulationTest;
import ini.cx3d.Param;
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.localBiology.AbstractLocalBiologyModule;
import ini.cx3d.localBiology.interfaces.CellElement;
import ini.cx3d.localBiology.NeuriteElement;
import ini.cx3d.physics.factory.SubstanceFactory;
import ini.cx3d.simulations.ECM;
import ini.cx3d.simulations.Scheduler;

import java.awt.Color;

public class NeuriteChemoAttractionTest extends BaseSimulationTest {
	public NeuriteChemoAttractionTest() {
		super(NeuriteChemoAttractionTest.class);
	}

	@Override
	public void simulate() {
		ECM ecm = ECM.getInstance();
		ECM.setRandomSeed(1L);
		ini.cx3d.physics.interfaces.Substance attractant = SubstanceFactory.create("A", Color.red);
		ecm.addArtificialGaussianConcentrationZ(attractant, 1.0, 400.0, 160.0);

		int nbOfAdditionalNodes = 10;
		for (int i = 0; i < nbOfAdditionalNodes; i++) {
			double[] coord = randomNoise(500, 3);
			ecm.getPhysicalNodeInstance(coord);
		}

		Cell c = CellFactory.getCellInstance(new double[]{0.0, 0.0, 0.0});
		c.setColorForAllPhysicalObjects(Param.VIOLET);
		NeuriteElement neurite = c.getSomaElement().extendNewNeurite();
		neurite.getPhysicalCylinder().setDiameter(2.0);
		neurite.addLocalBiologyModule(new NeuriteChemoAttraction("A"));

		for (int i = 0; i < 1000; i++) {
			Scheduler.simulateOneStep();
		}
	}
}

class NeuriteChemoAttraction extends AbstractLocalBiologyModule {


	static ECM ecm = ECM.getInstance();
	
	private double[] direction;

	private String substanceID;

	private double branchingFactor = 0.005;
	
	public NeuriteChemoAttraction(String substanceID) {
		this.substanceID = substanceID;
	}
	
	public NeuriteChemoAttraction(String substanceID, double branchingFactor) {
		this.substanceID = substanceID;
		this.branchingFactor = branchingFactor;
	}

	@Override
	public void setCellElement(CellElement cellElement) {
		super.setCellElement(cellElement);
		if(cellElement.isANeuriteElement())
			direction = cellElement.getPhysical().getAxis();
	}

	
	@Override
	public boolean isCopiedWhenNeuriteBranches() {
		return true;
	}
	
	@Override
	public boolean isDeletedAfterNeuriteHasBifurcated() {
		return true;
	}
	
	public AbstractLocalBiologyModule getCopy() {
		return new NeuriteChemoAttraction(substanceID);
	}

	public void run() {		
		ini.cx3d.physics.interfaces.PhysicalObject physical = super.cellElement.getPhysical();
		double concentration = physical.getExtracellularConcentration(substanceID);
		double[] grad = physical.getExtracellularGradient(substanceID);
		
		// 1) movement
		double oldDirectionWeight = 1.0;
		double gradientWeight = 0.2;
		double randomnessWeight = 0.6;
		
		if(physical.getExtracellularConcentration(substanceID)>0.3)
			grad = new double[] {0.0, 0.0, 0.0};
		
		double[] newStepDirection = add(
				scalarMult(oldDirectionWeight, direction),
				scalarMult(gradientWeight, normalize(grad)),
				randomNoise(randomnessWeight,3));
		double speed = 100;
		physical.movePointMass(speed, newStepDirection);

		direction = normalize(add(scalarMult(5,direction),newStepDirection));

		// 2) branching based on concentration:
		if(ecm.getRandomDouble()<concentration*branchingFactor){
			((NeuriteElement)cellElement).bifurcate();
		}
	}

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		sb.append("{}");
		return sb;
	}
}
