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

import ini.cx3d.BaseSimulationTest;
import ini.cx3d.Param;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.simulations.ECM;
import ini.cx3d.simulations.Scheduler;

/**
 *	This class was used to produce Figure 5 of the paper
 * "A framework for modeling the growth and development of neurons and networks", Zubler & Douglas 2009.
 * 
 * This class demonstrates the mechanical interactions between a chain of
 * PhysicalCylinders and a three PhysicalSpheres. The simulation starts by growing an axon,
 * then adds three overlapping spheres and pause. If un-paused, the mechanical interactions
 * push the objects apart.
 * @author fredericzubler
 *
 */
public class Figure_5Test extends BaseSimulationTest {

	public Figure_5Test() {
		super(Figure_5Test.class);
	}

	@Override
	public void simulate() throws Exception {
		// 1) Prepare the environment :
		// 		a reference to ECM, the extra-cellular-matrix
		ECM ecm = ECM.getInstance();
		// 		eight extra PhysicalNodes :
		for (int i = 0; i < 18; i++) {
			double angle = 2*Math.PI*ECM.getRandomDouble();
			double[] loc = {200*Math.sin(angle), 200*Math.cos(angle), -20+300*ECM.getRandomDouble()};
			ecm.getPhysicalNodeInstance(loc);
		}
		
		
		// 2) creating a first cell, with a neurite going straight up.
		// 		creating a 4-uple Cell-SomaElement-PhysicalSphere-SpatialOrganizerNode
		ini.cx3d.cells.interfaces.Cell cellA = CellFactory.getCellInstance(new double[] {0,0,-100});
		cellA.setColorForAllPhysicalObjects(Param.X_SOLID_RED);
		//		cretaing a single neurite
		ini.cx3d.localBiology.interfaces.NeuriteElement ne = cellA.getSomaElement().extendNewNeurite(2.0,0,0);
//		ne.setHasCytoskeletonMotor(false);
		// 		elongating the neurite :
		double[] directionUp = {0,0,1};
		ini.cx3d.physics.interfaces.PhysicalCylinder pc = ne.getPhysicalCylinder();
		for (int i = 0; i < 103; i++) {
			pc.movePointMass(300, directionUp);
			Scheduler.simulateOneStep();
		}
//		ecm.pause(3000);
		
		// 3) creating three additional spheres:
		ini.cx3d.cells.interfaces.Cell cellB = CellFactory.getCellInstance(new double[] {10,0,0});
		ini.cx3d.physics.interfaces.PhysicalSphere psB = cellB.getSomaElement().getPhysicalSphere();

		psB.setMass(3);
		psB.setColor(Param.X_SOLID_YELLOW);
		psB.setColor(Param.YELLOW);
		ini.cx3d.cells.interfaces.Cell cellC = CellFactory.getCellInstance(new double[] {-10,0,100});
		ini.cx3d.physics.interfaces.PhysicalSphere psC = cellC.getSomaElement().getPhysicalSphere();

		psC.setMass(3);
		psC.setColor(Param.X_SOLID_YELLOW);
		psC.setColor(Param.YELLOW);
		ini.cx3d.cells.interfaces.Cell cellD = CellFactory.getCellInstance(new double[] {10,0,160});
		ini.cx3d.physics.interfaces.PhysicalSphere psD = cellD.getSomaElement().getPhysicalSphere();

		psD.setColor(Param.X_SOLID_YELLOW);
		psD.setColor(Param.YELLOW);
		psD.setMass(2);
		
		// 4) setting a large diameter OR letting them grow
		boolean growing = true;
		if(growing){
			for (int i = 0; i < 30; i++) {
				psB.changeDiameter(400);
				psC.changeDiameter(300);
				psD.changeDiameter(200);
				Scheduler.runEveryBodyOnce(0);
			}
		}else{
			psB.setDiameter(140);
			psC.setDiameter(100);
			psD.setDiameter(50);
			ecm.setSimulationOnPause(true);
			ecm.view.repaint();
			
		}

		// 5) running the simulation slowly 
		for (int i = 0; i < 1000; i++) {
			Scheduler.runEveryBodyOnce(0);
		}
		
	}
}
