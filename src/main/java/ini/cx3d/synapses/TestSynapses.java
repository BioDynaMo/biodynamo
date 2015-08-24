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

package ini.cx3d.synapses;

import static ini.cx3d.utilities.Matrix.add;
import static ini.cx3d.utilities.Matrix.randomNoise;
import ini.cx3d.Param;
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.localBiology.NeuriteElement;
import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.simulations.ECM;
import ini.cx3d.simulations.Scheduler;

import java.awt.Color;




public class TestSynapses {
	

	public static void buildLine(){
		// 1) Prepare the environment :
		// 		get a reference to the extracelular matrix (ECM)
		ECM ecm = ECM.getInstance();
		// 		add additional PhysicalNodes (for diffusion)
		int nbOfAdditionalNodes = 100;
		for (int i = 0; i < nbOfAdditionalNodes; i++) {
			double[] coord = randomNoise(500, 3);
			ecm.getPhysicalNodeInstance(coord);
		}

		// 3) two cells
		double[] cellLocation = new double[] {2,0,60};
		Cell cell_1 = CellFactory.getCellInstance(cellLocation);
		cell_1.setColorForAllPhysicalObjects(Color.RED);

		cellLocation = new double[] {0,0,-60};
		Cell cell_2 = CellFactory.getCellInstance(cellLocation);
		cell_2.setColorForAllPhysicalObjects(Param.VIOLET);


		// 4) Extend an neuron from each cell
		NeuriteElement neurite_1 = cell_1.getSomaElement().extendNewNeurite(new double[] {0,0,-1});
		neurite_1.getPhysical().setDiameter(1.0);
		neurite_1.getPhysicalCylinder().setDiameter(4.0);
		neurite_1.setIsAnAxon(true);
		PhysicalCylinder pc_1 = neurite_1.getPhysicalCylinder();
		NeuriteElement neurite_2 = cell_2.getSomaElement().extendNewNeurite(new double[] {0,0,11});
		neurite_2.getPhysical().setDiameter(1.0);
		neurite_2.getPhysicalCylinder().setDiameter(4.0);
		PhysicalCylinder pc_2 = neurite_2.getPhysicalCylinder();

		// elongate 3 neurites
		for (int i = 0; i < 45; i++) {
			pc_1.movePointMass(200, add(pc_1.getSpringAxis(), randomNoise(0.1,3) ) );
			pc_2.movePointMass(200, add(pc_2.getSpringAxis(), randomNoise(0.1,3) ) );
			Scheduler.simulateThatManyTimeSteps(1);
		}

		extendExcressencesAndSynapseOnEveryNeuriteElement();
	}










	public static void extendExcressencesAndSynapseOnEveryNeuriteElement(){
		extendExcressencesAndSynapseOnEveryNeuriteElement(1.0);
	}

	public static void extendExcressencesAndSynapseOnEveryNeuriteElement(double probaBilityToSynapse){
		ECM ecm = ECM.getInstance();
		for (int i = 0; i < ecm.neuriteElementList.size(); i++) {
			NeuriteElement ne = ecm.neuriteElementList.get(i);
			if(ne.isAnAxon()==true){
				ne.makeBoutons(2);
			}else{
				ne.makeSpines(5);
			}
		}
		for (int i = 0; i < ecm.neuriteElementList.size(); i++) {
			NeuriteElement ne = ecm.neuriteElementList.get(i);
			if(ne.isAnAxon()==true){
				ne.synapseBetweenExistingBS(probaBilityToSynapse);
			}
		}

	}
}
