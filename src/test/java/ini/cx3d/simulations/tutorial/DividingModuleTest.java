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
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.cells.CellModule;
import ini.cx3d.physics.PhysicalSphere;
import ini.cx3d.simulations.ECM;
import ini.cx3d.simulations.Scheduler;

import static ini.cx3d.SimStateSerializationUtil.keyValue;

public class DividingModuleTest extends BaseSimulationTest {

	public DividingModuleTest() {
		super(DividingCellTest.class);
	}

	@Override
	public void simulate() {
		ECM.setRandomSeed(2L);
		Cell c = CellFactory.getCellInstance(new double[] {0.0,0.0,0.0});
		c.addCellModule(new DividingModule());

		Scheduler.simulateOneStep();
		Scheduler.simulateOneStep();
		ini.cx3d.utilities.SystemUtilities.tic();
		for (int i = 0; i < 5000; i++) {
			Scheduler.simulateOneStep();
		}
		ini.cx3d.utilities.SystemUtilities.tac();
	}
}

class DividingModule implements CellModule {

	Cell cell; 
	
	public Cell getCell() {
		return cell;
	}

	public void setCell(Cell cell) {
		this.cell = cell;
	}
	
	public void run() {

		PhysicalSphere sphere = cell.getSomaElement().getPhysicalSphere();		
		if(sphere.getDiameter()>20){
			cell.divide();
		}else{
			sphere.changeVolume(300);
		}
	}

	
	public DividingModule getCopy(){
		return new DividingModule();
	}
	
	public boolean isCopiedWhenCellDivides() {
		return true;
	}

	@Override
	public ini.cx3d.swig.StringBuilder simStateToJson(ini.cx3d.swig.StringBuilder sb) {
		sb.append("{}");
		return sb;
	}
}
