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

import static ini.cx3d.SimStateSerializationUtil.keyValue;
import static ini.cx3d.SimStateSerializationUtil.removeLastChar;
import static ini.cx3d.utilities.Matrix.randomNoise;
import static ini.cx3d.utilities.Matrix.add;
import static ini.cx3d.utilities.StringUtilities.toStr;

import ini.cx3d.BaseSimulationTest;
import ini.cx3d.JavaUtil2;
import ini.cx3d.Param;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.localBiology.LocalBiologyModule;
import ini.cx3d.localBiology.interfaces.CellElement;
import ini.cx3d.physics.factory.IntracellularSubstanceFactory;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.simulations.Scheduler;
import ini.cx3d.swig.NativeStringBuilder;
import ini.cx3d.utilities.Matrix;


public class IntracellularDiffusionTest extends BaseSimulationTest{

	public IntracellularDiffusionTest() {
		super(IntracellularDiffusionTest.class);
	}

	static JavaUtil2 java = new JavaUtil2();

	@Override
	public void simulate() {

		JavaUtil2.setRandomSeed(1L);
		initPhysicalNodeMovementListener();


		new ini.cx3d.swig.simulation.IntracellularDiffusionTest().simulate(ECMFacade.getInstance(), java);
		if(true) return;

		ECM ecm = ECMFacade.getInstance();
		for (int i = 0; i < 18; i++) {
			ecm.getPhysicalNodeInstance(randomNoise(500,3));
		}

		// defining the templates for the intracellular substance
		double D = 1000; // diffusion cst
		double d = 0.01;	// degradation cst
		ini.cx3d.physics.interfaces.IntracellularSubstance tubulin = IntracellularSubstanceFactory.create("tubulin", D, d);
		tubulin.setVolumeDependant(false);
		ecm.addNewIntracellularSubstanceTemplate(tubulin);
		// getting a cell
		ini.cx3d.cells.interfaces.Cell c = CellFactory.getCellInstance(new double[] {0,0,0});
		c.setColorForAllPhysicalObjects(Param.RED);
		// insert production module
		ini.cx3d.localBiology.interfaces.SomaElement soma = c.getSomaElement();
		soma.addLocalBiologyModule(new InternalSecretor());
		//insert growth cone module
		ini.cx3d.localBiology.interfaces.NeuriteElement ne = c.getSomaElement().extendNewNeurite(new double[] {0,0,1});
		ne.getPhysical().setDiameter(1.0);
		ne.addLocalBiologyModule(new GrowthCone());

		// run, Forrest, run..
		ini.cx3d.utilities.SystemUtilities.tic();
		for (int i = 0; i < 226; i++) { //2001
//			if(i%50 == 0)
				System.out.println(i);
			Scheduler.simulateOneStep();
		}
		ini.cx3d.utilities.SystemUtilities.tac();
		
	}

	private static class InternalSecretor extends ini.cx3d.swig.simulation.simulation.AbstractLocalBiologyModuleBase {

		public InternalSecretor(){
			super();
			ini.cx3d.swig.simulation.AbstractLocalBiologyModule.registerJavaObject(this);
			ini.cx3d.swig.simulation.LocalBiologyModule.registerJavaObject(this);
		}
		// secretion rate (quantity/time)
		private double secretionRate = 60;  
		
		// needed for copy in the cell in case of division
		public LocalBiologyModule getCopy() {
			return new InternalSecretor();
		}
		
		// method called at each time step: secretes tubulin in the extracellular space 
		public void run() {
			getCellElement().getPhysical().modifyIntracellularQuantity(
					"tubulin", secretionRate);
		}

		@Override
		public NativeStringBuilder simStateToJson(NativeStringBuilder sb) {
			sb.append("{");

			keyValue(sb, "secretionRate", secretionRate);

			removeLastChar(sb);
			sb.append("}");
			return sb;
		}
	}
	
	public static class GrowthCone extends ini.cx3d.swig.simulation.simulation.AbstractLocalBiologyModuleBase{
		
		// some parameters
		private static int counter = 0;
		private static double speedFactor = 5000;	
		private static double consumptionFactor = 100;
		private static double bifurcationProba = 0.003;
		// direction at previous time step:
		private double[] previousDir;

		public GrowthCone() {
			super();
			counter++;
			ini.cx3d.swig.simulation.AbstractLocalBiologyModule.registerJavaObject(this);
			ini.cx3d.swig.simulation.LocalBiologyModule.registerJavaObject(this);
			if(counter%100 == 0) System.out.println("#"+counter);
		}

//		@Override
//		protected void finalize() {
//			counter--;
//		}

		// initial direction is parallel to the cylinder axis
		// therefore we overwrite this method from the superclass:
		public void setCellElement(CellElement cellElement){
			super.setCellElement(cellElement);
			this.previousDir = cellElement.getPhysical().getAxis();
			System.out.println("setCellElement " + toStr(previousDir));
		}
		// to ensure distribution in all terminal segments:
		public LocalBiologyModule getCopy() {
//			System.out.println("getCopy");
			return new GrowthCone();
		}

		public boolean isCopiedWhenNeuriteBranches() {return true;}
		
		public boolean isDeletedAfterNeuriteHasBifurcated() {return true;}
		static double bifurcate = 0;
		// growth cone model
		public void run() {
			// getting the concentration and defining the speed
			ini.cx3d.physics.interfaces.PhysicalObject cyl = getCellElement().getPhysical();
			double concentration = cyl.getIntracellularConcentration("tubulin");
			double speed = concentration*speedFactor;
			if(speed>100)  // can't be faster than 100
				speed = 100;
			// movement and consumption
			double[] noise = randomNoise(0.1, 3);
			double[] dirBef = previousDir.clone();
			double[] direction = Matrix.add(previousDir, noise);
			previousDir = Matrix.normalize(direction);
			cyl.movePointMass(speed, direction);
			cyl.modifyIntracellularQuantity("tubulin", -concentration*consumptionFactor);
			// test for bifurcation
			double rand = ECMFacade.getInstance().getRandomDouble1();
			System.out.println(" - dirBef "+ toStr(dirBef) + " - dirAft "+ toStr(previousDir) + " - direction "+ toStr(direction)+"rand "+toStr(rand) + " - c "+toStr(concentration) + " - speed "+toStr(speed));
			if(rand < bifurcationProba) {
				System.out.println("bifurcate");
				((ini.cx3d.localBiology.interfaces.NeuriteElement) (getCellElement())).bifurcate();
			}
		}

		@Override
		public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
			super.simStateToJson(sb);

			keyValue(sb, "speedFactor", speedFactor);
			keyValue(sb, "consumptionFactor", consumptionFactor);
			keyValue(sb, "bifurcationProba", bifurcationProba);
			keyValue(sb, "previousDir", previousDir);

			removeLastChar(sb);
			sb.append("}");
			return sb;
		}
	}
}
