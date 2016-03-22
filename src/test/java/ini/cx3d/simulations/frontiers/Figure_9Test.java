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

import static ini.cx3d.utilities.Matrix.*;

import java.awt.Color;


import ini.cx3d.BaseSimulationTest;
import ini.cx3d.Param;
import ini.cx3d.cells.*;
import ini.cx3d.localBiology.*;
import ini.cx3d.physics.factory.PhysicalObjectFactory;
import ini.cx3d.simulations.*;
import ini.cx3d.synapses.TestSynapses;

/**
 * This class was used to produce Figure 9 of the paper
 * "A framework for modeling the growth and development of neurons and networks", Zubler & Douglas 2009.
 * 
 * By setting ECM boundaries, we for cells to stay in a 2.5D environment. 
 * Physical Objects use another type of inter object Force.
 * It needs the following classes : X_Bifurcation_Module, X_Movement_Module, X_Adhesive_Force
 * 
 * @author fredericzubler
 *
 */
public class Figure_9Test extends BaseSimulationTest {

	public Figure_9Test() {
		super(Figure_9Test.class);
	}

	@Override
	public void simulate() throws Exception {
		ECM.setRandomSeed(1L);

		Param.NEURITE_MAX_LENGTH = 20;
		double pi = Math.PI;
		// get a 2.5D ECM
		ECM ecm = ECM.getInstance();
		ECM.setRandomSeed(5L);
		ecm.setArtificialWallsForCylinders(true);
		ecm.setArtificialWallsForSpheres(true);
		ecm.setBoundaries(-10000, 10000, -10000, 10000, -5, 5);
		
		
		// eighteen extra PhysicalNodes :
		for (int i = 0; i < 12; i++) {
			double[] loc = concat(randomNoise(600,2), randomNoise(100,1));
			ecm.getPhysicalNodeInstance(loc);
		}
	
		// set the inter object force
		X_Adhesive_Force nogo = new X_Adhesive_Force();
		nogo.setAttractionRange(3);
		nogo.setAttractionStrength(5);
		
		PhysicalObjectFactory.setInterObjectForce(nogo);
		
		// generate cells
		int nbOfCells = 20;   
		int minNbOfNeurites = 4;
		int maxNbOfNeurites = 8;
		
		for (int i = 0; i < nbOfCells; i++) {
			
			Color c = Param.X_SOLID_GRAY;

			double[] cellLocation = new double[] {-200+ECM.getRandomDouble()*400, -200+ECM.getRandomDouble()*400, -5+ECM.getRandomDouble()*10 };
			
			if(i==0){
				c= Param.X_SOLID_RED;
				cellLocation = new double[] {0,0,0};
				
			}
			
			Cell cell = CellFactory.getCellInstance( cellLocation );
			SomaElement soma = cell.getSomaElement();
			ini.cx3d.physics.interfaces.PhysicalSphere sphere = soma.getPhysicalSphere();
			if(i==0){
			cell.setNeuroMLType(Cell.InhibitoryCell);
			}else{
				cell.setNeuroMLType(Cell.ExcitatoryCell);
			}
			sphere.setColor(c);
			sphere.setAdherence(100);
			
			int nbOfNeurites = minNbOfNeurites + ((int)((maxNbOfNeurites-minNbOfNeurites)*ECM.getRandomDouble()));
			
			for (int j = 0; j < nbOfNeurites; j++) {
				double angleOfAxon = pi*2*ECM.getRandomDouble();
				double growthSpeed = 75;
				double probaToBranch = 0.003;
				double linearDiameterDecrease = 0.001;
				NeuriteElement ne;
				if(j==0){
					ne = cell.getSomaElement().extendNewNeurite(3.0, Math.PI*0.5, angleOfAxon);
					ne.setIsAnAxon(true);
					growthSpeed = 150;
					probaToBranch = 0.009;
					linearDiameterDecrease = 0;
					ne.getPhysicalCylinder().setDiameter(1.5);
				}else if (j==1){
					ne = cell.getSomaElement().extendNewNeurite(3.0, Math.PI*0.5, angleOfAxon + Math.PI -0.5+ECM.getRandomDouble());
					ne.setIsAnAxon(false);
				}else{
					ne = cell.getSomaElement().extendNewNeurite(3.0, Math.PI*0.5, Math.PI*2*ECM.getRandomDouble());
					ne.setIsAnAxon(false);
				}
				
				X_Bifurcation_Module br = new X_Bifurcation_Module();
				br.shift = probaToBranch;
				ne.addLocalBiologyModule(br);
				X_Movement_Module mr = new X_Movement_Module();
				mr.setRandomness(0.7);
				mr.setSpeed(growthSpeed);
				mr.setLinearDiameterDecrease(linearDiameterDecrease);
				ne.addLocalBiologyModule(mr);				

			}
		}
		
		for (int i = 0; i < 350; i++) { // 350
			Scheduler.simulateOneStep();
		}
		
		TestSynapses.extendExcressencesAndSynapseOnEveryNeuriteElement(0.4);
//		Exporter.saveExport();
	}
}
