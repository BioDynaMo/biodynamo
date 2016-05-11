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
import ini.cx3d.cells.CellFactory;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.Scheduler;

import static ini.cx3d.SimStateSerializationUtil.keyValue;

public class DividingModuleTest extends BaseSimulationTest {

	public DividingModuleTest() {
		super(DividingCellTest.class);
	}

	@Override
	public void simulate() {
		new ini.cx3d.swig.simulation.DividingModuleTest().simulate(ECMFacade.getInstance(), new JavaUtil2());

//		JavaUtil2.setRandomSeed(2L);
//		initPhysicalNodeMovementListener();
//		ini.cx3d.cells.interfaces.Cell c = CellFactory.getCellInstance(new double[] {0.0,0.0,0.0});
//		c.addCellModule(new DividingModule());
//
//		Scheduler.simulateOneStep();
//		Scheduler.simulateOneStep();
//		ini.cx3d.utilities.SystemUtilities.tic();
//		for (int i = 0; i < 5000; i++) {
//			Scheduler.simulateOneStep();
//		}
//		ini.cx3d.utilities.SystemUtilities.tac();
	}
}

class DividingModule extends ini.cx3d.swig.simulation.CellModule {

	ini.cx3d.cells.interfaces.Cell cell;

	public DividingModule() {
		registerJavaObject(this);
	}
	
	public ini.cx3d.cells.interfaces.Cell getCell() {
		return cell;
	}

	public void setCell(ini.cx3d.cells.interfaces.Cell cell) {
		this.cell = cell;
	}
	
	public void run() {

		ini.cx3d.physics.interfaces.PhysicalSphere sphere = cell.getSomaElement().getPhysicalSphere();
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
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		sb.append("{}");
		return sb;
	}
}
