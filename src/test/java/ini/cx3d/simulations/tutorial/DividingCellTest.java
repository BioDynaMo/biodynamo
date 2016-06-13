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

import java.awt.Color;

import ini.cx3d.BaseSimulationTest;
import ini.cx3d.Param;
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.localBiology.SomaElement;
import ini.cx3d.physics.PhysicalSphere;
import ini.cx3d.simulations.ECM;
import ini.cx3d.simulations.Scheduler;

public class DividingCellTest extends BaseSimulationTest {

	public DividingCellTest() {
		super(DividingCellTest.class);
	}

	@Override
	public void simulation() {
		ECM.setRandomSeed(1L);

		double[] cellOrigin = {0.0, 3.0, 5.0};			
		Cell cell = CellFactory.getCellInstance(cellOrigin);		
		cell.setColorForAllPhysicalObjects(Param.RED);
		SomaElement soma = cell.getSomaElement();
		PhysicalSphere sphere = soma.getPhysicalSphere();



		for (int i = 0; i < 466032; i++) {
			Scheduler.simulateOneStep();		// run the simulation
			if(sphere.getDiameter()<20){		// if small..
				sphere.changeVolume(350);		// .. increase volume
			}else{
				Cell c2 = cell.divide();		// otherwise divide
				c2.setColorForAllPhysicalObjects(Param.BLUE);
			}
		}
		
	}
}
