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

import static ini.cx3d.utilities.Matrix.randomNoise;

import ini.cx3d.BaseSimulationTest;
import ini.cx3d.Param;
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.physics.interfaces.PhysicalCylinder;
import ini.cx3d.simulations.ECM;
import ini.cx3d.simulations.Scheduler;
import ini.cx3d.synapses.factory.BiologicalBoutonFactory;
import ini.cx3d.synapses.factory.PhysicalBoutonFactory;
import ini.cx3d.synapses.factory.PhysicalSpineFactory;
import ini.cx3d.synapses.interfaces.BiologicalSpine;
import ini.cx3d.synapses.PhysicalBouton;
import ini.cx3d.synapses.PhysicalSpine;
import ini.cx3d.synapses.factory.BiologicalSpineFactory;

public class SimpleSynapseTest extends BaseSimulationTest {

	public SimpleSynapseTest() {
		super(SimpleSynapseTest.class);
	}

	@Override
	public void simulate() throws Exception {
		ECM.setRandomSeed(1L);
		ECM ecm = ECM.getInstance();
		int nbOfAdditionalNodes = 10;
		for (int i = 0; i < nbOfAdditionalNodes; i++) {
			double[] coord = randomNoise(500, 3);
			ecm.getPhysicalNodeInstance(coord);
		}

		double[] up = {0.0,0.0,1.0}, down = {0.0,0.0,-1.0};
		// 1) two cells : and excitatory (down) and an inhibitory one (up)
		Cell excit = CellFactory.getCellInstance(new double[] {-2.5,0,-30});
		excit.setNeuroMLType(Cell.ExcitatoryCell);
		excit.setColorForAllPhysicalObjects(Param.GREEN);
		Cell inhib = CellFactory.getCellInstance(new double[] {2.5,0,30});
		inhib.setNeuroMLType(Cell.InhibitoryCell);
		inhib.setColorForAllPhysicalObjects(Param.RED);
		// 2) excitatory cell makes an axon, inhibitory cell makes a dendrite
		ini.cx3d.localBiology.interfaces.NeuriteElement axon = excit.getSomaElement().extendNewNeurite(up);
		axon.setAxon(true);
		PhysicalCylinder axonCyl = axon.getPhysicalCylinder();
		ini.cx3d.localBiology.interfaces.NeuriteElement dendrite = inhib.getSomaElement().extendNewNeurite(down);
		dendrite.setAxon(false);
		PhysicalCylinder dendriteCyl = dendrite.getPhysicalCylinder();
		//		elongate them
		while (axon.getLocation()[2]<dendrite.getLocation()[2]) {
			axon.elongateTerminalEnd(1/Param.SIMULATION_TIME_STEP, up);
			dendrite.elongateTerminalEnd(1/Param.SIMULATION_TIME_STEP, down);
			Scheduler.simulateOneStep();
		}
		// 3) a bouton on the axon:
		// 		create the physical part
		double[] globalCoord = new double[] {axon.getLocation()[2] + dendrite.getLocation()[2],0,0};
		double[] polarAxonCoord = axonCyl.transformCoordinatesGlobalToPolar(globalCoord);
		polarAxonCoord = new double[] {polarAxonCoord[0], polarAxonCoord[1]}; // so r is implicit

		ini.cx3d.synapses.interfaces.PhysicalBouton pBouton = PhysicalBoutonFactory.create(axonCyl, polarAxonCoord, 3);
		axonCyl.addExcrescence(pBouton);
		// 		create the biological part and set call backs
		ini.cx3d.synapses.interfaces.BiologicalBouton bBouton = BiologicalBoutonFactory.create();
		pBouton.setBiologicalBouton(bBouton);
		bBouton.setPhysicalBouton(pBouton);

		// 4) a spine on the dendrite:
		// 		create the physical part
		double[] polarDendriteCoord = dendriteCyl.transformCoordinatesGlobalToPolar(globalCoord);
		polarDendriteCoord = new double[] {polarDendriteCoord[0], polarDendriteCoord[1]}; // so r is implicit

		ini.cx3d.synapses.interfaces.PhysicalSpine pSpine = PhysicalSpineFactory.create(dendriteCyl, polarDendriteCoord, 3);
		dendriteCyl.addExcrescence(pSpine);
		// 		create the biological part and set call backs
		BiologicalSpine bSpine = BiologicalSpineFactory.create();
		pSpine.setBiologicalSpine(bSpine);
		bSpine.setPhysicalSpine(pSpine);

		// 5) synapse formation
		pBouton.synapseWith(pSpine, true);

//		Thread.sleep(Integer.MAX_VALUE);
	}
}
