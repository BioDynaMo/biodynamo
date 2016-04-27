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

import ini.cx3d.BaseSimulationTest;
import ini.cx3d.JavaUtil2;
import ini.cx3d.Param;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.Scheduler;

public class DividingCellTest extends BaseSimulationTest {

	public DividingCellTest() {
		super(DividingCellTest.class);
	}

	@Override
	public void simulate() {
		JavaUtil2.setRandomSeed(1L);
		initPhysicalNodeMovementListener();

		new ini.cx3d.swig.simulation.DividingCellTest().simulate(ECMFacade.getInstance());

//		double[] cellOrigin = {0.0, 3.0, 5.0};
//		ini.cx3d.cells.interfaces.Cell cell = CellFactory.getCellInstance(cellOrigin);
//		cell.setColorForAllPhysicalObjects(Param.RED);
//		ini.cx3d.localBiology.interfaces.SomaElement soma = cell.getSomaElement();
//		ini.cx3d.physics.interfaces.PhysicalSphere sphere = soma.getPhysicalSphere();
//
//
//
//		for (int i = 0; i < 5000; i++) {
//			Scheduler.simulateOneStep();		// run the simulation
//			if(sphere.getDiameter()<20){		// if small..
//				sphere.changeVolume(350);		// .. increase volume
//			}else{
//				ini.cx3d.cells.interfaces.Cell c2 = cell.divide();		// otherwise divide
//				c2.setColorForAllPhysicalObjects(Param.BLUE);
//			}
//		}
		
	}
}
