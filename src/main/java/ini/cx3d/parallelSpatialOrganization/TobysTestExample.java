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

/**
 * 
 */
package ini.cx3d.parallelSpatialOrganization;

import static ini.cx3d.utilities.Matrix.norm;
import static ini.cx3d.utilities.Matrix.subtract;
import ini.cx3d.spatialOrganization.PositionNotAllowedException;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;

import java.util.LinkedList;
import java.util.Random;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * @author Dennis
 *
 */
public class TobysTestExample {
	public static long initialSeed = -8282765079149723159l;

	public final int nodeCount = 100;
	public final int volumeConstant = 30;
	public final int movesPerRun = 200;
	public final int somCount = 1;
	LinkedList<SpatialOrganizationManager<Integer>> soms = new LinkedList<SpatialOrganizationManager<Integer>>();
	public static Random rand = new Random();

	public TobysTestExample() {
		runSingleTest(initialSeed, initializeSOMs());
	}
	
	public static int rand(int min, int max) {
		int ret = (int) (rand.nextDouble() * (double) (max - min)) + min;
		// System.out.println(ret);
		return ret;
	}

	
	
	public int runSingleTest(long seed, SpatialOrganizationManager<Integer> firstSOM) {
		rand = new Random(seed);
		System.out.println("\nTrying seed "+ seed);
		SpaceNode.clear();
		Tetrahedron.clear();

		SpatialOrganizationNode<Integer>[] innerNodes = new SpatialOrganizationNode[nodeCount];
		double[][] coordinates = new double[nodeCount][];
		
		// insert nodes:
		System.out.println("insert nodes...");
		for (int i = 0; i < innerNodes.length; i++) {
			// this is just to see that something is going on...
			if (i % 10 == 0) {
				System.out.print(".");
			}
			if (i % 10000 == 0)
				System.out.println("");
			
			double[] position = new double[] {
					rand.nextDouble() * 2 * volumeConstant - volumeConstant,
					rand.nextDouble() * 2 * volumeConstant - volumeConstant,
					rand.nextDouble() * 2 * volumeConstant - volumeConstant };
			coordinates[i] = position;

			// This code is not really necessary... it searches for the node among the last 100 inserted that's
			// closest to the position where you're about to insert another node:
			SpatialOrganizationNode<Integer> closest = null;
			double minDistance = Double.MAX_VALUE;
			for (int j = i - 100; j < i; j++) {
				if (j >= 0) {
					double dummy = norm(subtract(coordinates[j],
							position));
					if (dummy < minDistance) {
						minDistance = dummy;
						closest = innerNodes[j];
					}
				}
			}
			
			try {
				if (closest != null) {
					// This is the normal case: the node is not the first one to be inserted:
					innerNodes[i] = closest.getNewInstance(position, new Integer(0));
				} else {

					System.out.println("Creating Initial Node...\n");
					// the special case for the first node: ask the first available SOM to create an initial node:
					 innerNodes[i] = firstSOM.createInitialNode(position, new Integer(0));
				}
			} catch (PositionNotAllowedException e) {
				i--;
			}
		}


		// now move!
		int runNumber = 0;
		try {
			for (runNumber = 0; runNumber < this.movesPerRun; runNumber++) {
				
				// pick a random node:
				SpatialOrganizationNode<Integer> anyNode = innerNodes[rand(0,innerNodes.length)];

				// compute a random movement for that node:
				double[] delta = new double[3];
				delta[0] = rand.nextDouble()-0.5;
				delta[1] = rand.nextDouble()-0.5;
				delta[2] = rand.nextDouble()-0.5;

				// to see that something happening...
				if (runNumber % 10 == 0) {
					System.out.print(".");
				}
				if (runNumber % 1000 == 0)
					System.out.println("\nRun number " + runNumber);
				
				try {
					// move!
					anyNode.moveFrom(delta);
				} catch (PositionNotAllowedException e) {
					// this is something we'll have to deal with in the future...
				} 
			}
		} catch (Exception e) {
			System.out.println("Run number: " + runNumber);
			System.out.println("Seed was " + seed);
			e.printStackTrace();
			return runNumber;
		}
		return runNumber;
	}

	
	
	/**
	 * Creates a set of SOMs. See {@link #somCount}.
	 * @return One of the created SOMs.
	 */
	public SpatialOrganizationManager<Integer> initializeSOMs() {
		double[] minCoordinate = { -volumeConstant, -volumeConstant, -volumeConstant };
		double[] maxCoordinate = { volumeConstant, volumeConstant, volumeConstant };
		soms.clear();
		
		LinkedList<SimpleAssignmentPolicy<Integer>> policies = new LinkedList<SimpleAssignmentPolicy<Integer>>();
		long addressRange = 100000000000l / (long)somCount;
		long pos = 0;
		for (int i = 0; i < somCount ; i++) {
			SimpleAssignmentPolicy<Integer> policy1 = new SimpleAssignmentPolicy<Integer>(
					minCoordinate, maxCoordinate);
			policies.add(policy1);
			soms.add(new SpatialOrganizationManager<Integer>(
				policy1, pos, pos+addressRange-1));
			pos+=addressRange-1;
		}
		
		// register all the created SOMs:
		for (SimpleAssignmentPolicy<Integer> policy : policies) {
			for (SpatialOrganizationManager<Integer> som : soms) 
				policy.registerNewSOM(som);
		}
		return soms.getFirst();
	}
	
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		
		  // Set Log Level for the Logging Handlers.
	    Handler[] handlers =
	      Logger.getLogger( "" ).getHandlers();
	    for ( int index = 0; index < handlers.length; index++ ) {
	      handlers[index].setLevel( Level.FINER );
	    }
	    
		// turn off multithreading:
		SpatialOrganizationManager.javaMultiThreaded = false;
		
		
		new TobysTestExample();
	}

}
