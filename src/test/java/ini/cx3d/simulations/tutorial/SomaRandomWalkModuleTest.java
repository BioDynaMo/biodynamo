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

import ini.cx3d.BaseSimulationTest;
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.localBiology.AbstractLocalBiologyModule;
import ini.cx3d.localBiology.LocalBiologyModule;
import ini.cx3d.simulations.ECM;
import ini.cx3d.simulations.Scheduler;

public class SomaRandomWalkModuleTest extends BaseSimulationTest {
	public SomaRandomWalkModuleTest() {
		super(SomaRandomWalkModuleTest.class);
	}

	@Override
	public void simulate() throws Exception {
		ECM.setRandomSeed(1L);
		for(int i = 0; i<5; i++){
			Cell c = CellFactory.getCellInstance(randomNoise(40, 3));
			c.getSomaElement().addLocalBiologyModule(new SomaRandomWalkModule());
//			c.addCellModule(new DividingModule());  // un-comment to have the cells divide
		}
		for (int i = 0; i < 1000; i++) {
			Scheduler.simulateOneStep();
		}
	}
}

class SomaRandomWalkModule extends ini.cx3d.swig.biology.biology.AbstractLocalBiologyModuleBase {

	double direction[] = randomNoise(1.0, 3); // initial direction

	public SomaRandomWalkModule() {
		super();
		ini.cx3d.swig.biology.AbstractLocalBiologyModule.registerJavaObject(this);
		ini.cx3d.swig.biology.LocalBiologyModule.registerJavaObject(this);
	}

	public LocalBiologyModule getCopy() {
		return new SomaRandomWalkModule();
	}

	public void run() {
		double speed = 50;
		double[] deltaDirection = randomNoise(0.1, 3);
		direction = add(direction, deltaDirection);
		direction = normalize(direction);
		getCellElement().move(speed, direction);
	}
	
	public boolean isCopiedWhenSomaDivides() {
		return true;
	}

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		sb.append("{}");
		return sb;
	}

}
