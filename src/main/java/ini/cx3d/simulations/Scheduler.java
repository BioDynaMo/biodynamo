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


package ini.cx3d.simulations;


import static ini.cx3d.utilities.StringUtilities.doubleToString;
import ini.cx3d.Param;
import ini.cx3d.graphics.View;
import ini.cx3d.physics.ECMChemicalReaction;
import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.physics.PhysicalSphere;

/**
 * This class contains static methods to loop through all the "runnable" CX3D objects
 * stored in a list in ECM to call their run() method. As far as physics is concerned,
 * the call to the run() method is only made if the object has some of it's state variables
 * being modified, or is in a situation where it might occur. 
 * 
 * @author rjd & fredericzubler
 *
 */
public class Scheduler {
	/* Reference to the ECM.*/
	protected static ECM ecm = ECM.getInstance();
	/* Reference to the ECM display window */
	protected static View view = ecm.createGUI();
	/* static counter, needed in case where we want to make regular snapshots.*/
	protected static int cycle_counter = 0; 	
	protected static int inter_snapshot_time_steps = 30; 	


	/* if false, the physics is not computed......*/
	public static boolean runPhyics = true;
	public static boolean runDiffusion = true;
	
	protected static boolean printCurrentECMTime = true;
	protected static boolean printCurrentStep = false;

	/** Runs all the CX3D elements for one time step, and pauses for a few ms.
	 * @param pauseTime the pause time in milliseconds.
	 */
	public static void runEveryBodyOnce(int pauseTime){
		ECM.pause(pauseTime);
		simulateOneStep();
	}

	protected static long physics_time;
	protected static long module_time;
	protected static long total_time;
	/** Runs all the CX3D runnable objects for one time step.*/
	public static void simulateOneStep(){
		long start_time = System.currentTimeMillis(); 
		
		
		try {
			
			ECM.getInstance().canRun.acquire();
			

			Thread.sleep(1);


		

			if(printCurrentECMTime){
				System.out.println("time = "+doubleToString(ecm.getECMtime(), 2));
			}

			if(printCurrentStep){
				System.out.println("step = "+cycle_counter);	
			}

			// if we make a pause at each time step
//			while (ecm.isSimulationOnPause()) {

//			} 

			// GUI rotation
			if (ecm.isContinuouslyRotating()) {
				ecm.view.rotateAroundZ(ecm.getView().getRotationSpeed());
			}
			long phystemptime = System.currentTimeMillis(); 
			if(runPhyics){
				// PhysicalNode (diffusion & degradation of Substances)
				int totalNbOfPhysicalNode =  ecm.physicalNodeList.size();
				int runPhyisicalNodes = 0;
				if(runDiffusion){
					for (int i = 0; i < ecm.physicalNodeList.size(); i++) {
						PhysicalNode pn = ecm.physicalNodeList.get(i);
						if(pn.isOnTheSchedulerListForPhysicalNodes()){
							pn.runExtracellularDiffusion(); 
							runPhyisicalNodes +=1;
						}
					}
				}
				
				if(!ecm.ecmChemicalReactionList.isEmpty()){
					for (int i = 0; i < ecm.physicalNodeList.size(); i++) {
						PhysicalNode pn = ecm.physicalNodeList.get(i);
						for (ECMChemicalReaction chemicalReaction : ecm.ecmChemicalReactionList) {
							chemicalReaction.run(pn);
						}
					}
				}
				
				
				//		System.out.println("PhysicalNodes : \ttotal = "+totalNbOfPhysicalNode+" ; \tRun = "+runPhyisicalNodes);

				// Physical objects : PhysicalCylinders
				int totalNbOfPhysicalCylinders =  ecm.physicalCylinderList.size();
				int runPhysicalCylinder = 0;
				for (int i = 0; i < ecm.physicalCylinderList.size(); i++) {
					PhysicalCylinder pc = ecm.physicalCylinderList.get(i);
					if(pc.isOnTheSchedulerListForPhysicalObjects()){
						pc.runPhysics();
						runPhysicalCylinder++;
						//				pc.setColor(Param.VIOLET);
					}else{
						//				pc.setColor(Param.BLUE);
					}
					pc.runIntracellularDiffusion();

				}
				//		System.out.println("PhysicalCylinders : \ttotal = "+totalNbOfPhysicalCylinders+" ; \tRun = "+runPhysicalCylinder);

				// Physical objects : PhysicalSpheres
				int totalNbOfPhysicalSpheres =  ecm.physicalSphereList.size();
				int runPhyisicalSpheres = 0;
				for (int i = 0; i < ecm.physicalSphereList.size(); i++) {
					PhysicalSphere ps = ecm.physicalSphereList.get(i);
					if(ps.isOnTheSchedulerListForPhysicalObjects()){
						ps.runPhysics();
						runPhyisicalSpheres ++;
						//				ps.setColor(Param.VIOLET);
					}else{
						//				ps.setColor(Param.BLUE);
					}
					ps.runIntracellularDiffusion();
				}
			}
			physics_time +=System.currentTimeMillis()-phystemptime;
			
			//		System.out.println("PhysicalSpheres : \ttotal = "+totalNbOfPhysicalSpheres+" ; \tRun = "+runPhyisicalSpheres);
			// cellList
			
			// Modified by Sabina: the new cells should not be run in the same time step as they are created!!!
			int size = ecm.cellList.size();
			for (int i = 0; i < size; i++) {
				ecm.cellList.get(i).run();
			}
			
//			for (int i = 0; i < ecm.cellList.size(); i++) {
//				ecm.cellList.get(i).run();
//			}
			
			// somata
			
			long moduletime = System.currentTimeMillis(); 
			for (int i = 0; i < ecm.somaElementList.size(); i++) {
				ecm.somaElementList.get(i).run();
			}
			// neurites
			for (int i = 0; i < ecm.neuriteElementList.size(); i++) {
				ecm.neuriteElementList.get(i).run();
			}
			module_time += System.currentTimeMillis()-moduletime;
			
			// update values in substances
//			for (int i = 0; i < ecm.physicalNodeList.size(); i++) {
//				PhysicalNode pn = ecm.physicalNodeList.get(i);
//			//	if(pn.isOnTheSchedulerListForPhysicalNodes())
//			//	{
//					for(Substance s: pn.getExtracellularSubstances().values()){
//						s.updateNewValues();
//					}
//					if(pn instanceof PhysicalObject)
//					{
//						for(Substance s: ((PhysicalObject)pn).getIntracellularSubstances().values())
//						{
//							s.updateNewValues();
//						}
//					}
//			//	}
//			}
			
			// possibility to make a movie
			if (ecm.isTakingSnapshotAtEachTimeStep() == true) {
				ecm.dumpImage();
			} else if (cycle_counter % inter_snapshot_time_steps == 0  && ecm.isTakingSnapshotEach100TimeSteps() == true) {
				ecm.dumpImage();
			}
			// updating the picture on the GUI
			view.repaint();
			// ticking ECM's time
			cycle_counter += 1;
			ecm.increaseECMtime(Param.SIMULATION_TIME_STEP);
			//System.out.println("Machine executed"+Machine.elementsexecuted);
			ecm.getInstance().canRun.release();
			
			
			
		} catch (InterruptedException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		total_time += System.currentTimeMillis()-start_time;
//		System.out.println("total Time: "+total_time);
//		System.out.println("physics % : "+100.0/(module_time+physics_time)*physics_time);
//		System.out.println("module  % : "+100.0/(module_time+physics_time)*module_time);
	}

	/** Runs the simulation, i.e. runs each active CX3D runnable objects endlessly.*/
	public static void simulate(){
		while(true)
			simulateOneStep();
	}

	/** Runs the simulation for a given number of time steps, i.e. runs each active CX3D 
	 * runnable objects.
	 * @param steps nb of steps that the simulation is run.
	 */
	public static void simulateThatManyTimeSteps(int steps){
		for (int i = 0; i < steps; i++) {
			simulateOneStep();
		}
	}

	public static void setPrintCurrentECMTime(boolean printCurrentECMTime) {
		Scheduler.printCurrentECMTime = printCurrentECMTime;
	}

	public static void setPrintCurrentStep(boolean printCurrentStep) {
		Scheduler.printCurrentStep = printCurrentStep;
	}

	public static void reset() {
		// TODO Auto-generated method stub
		
	}
	
	public static void PauseAndDraw(boolean val) {
		
		if(val!=ecm.myGuiCreator.pause) ecm.myGuiCreator.togglePauseSim();
		if(val!=ecm.myGuiCreator.isPainted()) ecm.myGuiCreator.togglePaint();
	}


	public static int getInter_snapshot_time_steps() {
		return inter_snapshot_time_steps;
	}

	public static void setInter_snapshot_time_steps(int inter_snapshot_time_steps) {
		Scheduler.inter_snapshot_time_steps = inter_snapshot_time_steps;
	}

}
