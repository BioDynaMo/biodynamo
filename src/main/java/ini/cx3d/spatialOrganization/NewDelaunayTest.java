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

package ini.cx3d.spatialOrganization;

import static ini.cx3d.utilities.Matrix.*;
import ini.cx3d.parallelSpatialOrganization.SimpleAssignmentPolicy;
import ini.cx3d.parallelSpatialOrganization.SpatialOrganizationManager;
import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.physics.PhysicalNodeMovementListener;
import ini.cx3d.simulations.ECM;

import java.io.PrintStream;
import java.util.LinkedList;
import java.util.Random;

/**
 * This is a testing package used to simulate the creation of a random
 * triangulation and successive movements of points.
 * 
 * Used for debugging purposes. Needs to be removed at some time. 
 * 
 * @author Dennis Goehlsdorf
 *
 */
public class NewDelaunayTest {
	
	final static boolean useParallelSpatialOrganization = false;
	
//	public final int initialNodeCount = 0;
//	public final int insertNodeCount = 500;
//	public final int volumeConstant = 30;
//	public final int movesPerRun = 1000;
	
	public final int initialNodeCount = 0;
	public final int insertNodeCount = 10000;
	public final int volumeConstant = 30;
	public final int movesPerRun = 20000;

	
//	public final long initialSeed = -4399871388000337276l; 
//	public final long initialSeed = -2727759062064147475l;
//	public final long initialSeed =  3189782250572290986l; ;
//	public final long initialSeed =  -3202748494647697604l; 
//	public final long initialSeed =  -1518617032522254875l;
	public final long initialSeed = -8282765079149723159l;
	
	
//	public final long initialSeed = 6344771121160675034l;

	
	//	public final long initialSeed =  3041808783187395313l;
	
//	public final long initialSeed =  -1804927093357226749l; 
//	public final long initialSeed =  6646313007125023494l;
//	public final long initialSeed = -2061350812195586211l; 
//	public final long initialSeed = 1016542914281514000l;
	
//*	
	public final boolean useInitialSeed = true; /*/
	public final boolean useInitialSeed = true; //*/
	public final int printRun = -1;//16742;
	public final int[] checks = new int[] {};
	public final boolean repair = false;
	public static Random rand = new Random(1312);
	Tetrahedron<PhysicalNode> outerTetrahedron;
	int totalCounter = 0;
	public static PrintStream out;
	private static boolean createOutPut = false;
	
//	private class T {
//		
//	}
	public static boolean createOutput() {
		return createOutPut;
	}
	public static void changeOutputCreation() {
		createOutPut ^= true;
	}

	public static int rand(int min, int max) {
		
		int ret = (int)(rand.nextDouble() * (double)(max - min))+min;
		//System.out.println(ret);
		return ret;
	}
	
	public static void out(String s) {
		if (createOutPut)
			System.out.println(s);
	}
	public static void out(double d) {
		out(Double.toString(d));
	}
	
	public void analyzeDoubleArray(double[] a, String subject) {
		double sum = 0.0;
		for (int i = 0; i < a.length; i++) {
			sum += a[i];
		}
		double average = sum / a.length;
		double var = 0.0;
		double min = Double.POSITIVE_INFINITY;
		double max = Double.NEGATIVE_INFINITY;
		for (int i = 0; i < a.length; i++) {
			var += (average - a[i])*(average - a[i]);
			if (a[i] > max) {
				max = a[i];
			}
			if (a[i] < min) {
				min = a[i];
			}
		}
		var /= a.length;
		var = Math.sqrt(var);
		System.out.println("\nAnalysis of "+subject);
		System.out.println("Average: "+average);
		System.out.println("StdDev: "+var);
		System.out.println("Intervall: "+min+" - "+max);
	}
	
	public void checkConvexHull() {
		if (Tetrahedron.allTetrahedra != null)
		for (Tetrahedron<PhysicalNode> tet : Tetrahedron.allTetrahedra) {
			if (tet.isInfinite()) {
				Triangle3D<PhysicalNode> convexhullTriangle = tet.getAdjacentTriangles()[0];
				convexhullTriangle.updatePlaneEquationIfNecessary();
				if (convexhullTriangle.getOppositeTetrahedron(tet) != null) {
					SpaceNode<PhysicalNode> innerNode = convexhullTriangle.getOppositeTetrahedron(tet).getOppositeNode(convexhullTriangle);
					if (SpaceNode.allNodes != null) 
					for (SpaceNode<PhysicalNode> node : SpaceNode.allNodes) {
						if (!tet.isAdjacentTo(node)) {
							if (!convexhullTriangle.onSameSide(innerNode.getPosition(), node.getPosition())) {
								System.out.println("The node "+node+" doesn't lie on the inner side of "+convexhullTriangle+"!");
							}
						}
					}
				}
			}
		}
	}
	
	public static boolean checkTetrahedronForDelaunayViolation(Tetrahedron tetrahedron) {
		boolean problems = false;
		if (SpaceNode.allNodes != null) 
		for (SpaceNode<PhysicalNode> node : SpaceNode.allNodes) {
			if (tetrahedron.isFlat())
				throw new RuntimeException("There is a flat tetrahedron left over!");
			if (!tetrahedron.isAdjacentTo(node) && !tetrahedron.isFlat())
			if (tetrahedron.isTrulyInsideSphere(node.getPosition())) {
				System.out.print("The Node "+node+"lies inside the circumsphere of tetrahedron "+tetrahedron+"!");
				if (tetrahedron.orientationExact(node.getPosition()) <= 0)
					System.out.print(" (but not truly!)");
				System.out.println("");
				double[] distVector = subtract(tetrahedron.circumCenter,node.getPosition());
				problems = true;
				tetrahedron.isTrulyInsideSphere(node.getPosition());
			}
		}
		return problems;
	}
	
	public SpatialOrganizationManager<PhysicalNode> initializeSOMs() {
		double[] minCoordinate = { -volumeConstant, -volumeConstant, -volumeConstant };
		double[] maxCoordinate = { volumeConstant, volumeConstant, volumeConstant };

		SimpleAssignmentPolicy<PhysicalNode> policy = new SimpleAssignmentPolicy<PhysicalNode>(
				minCoordinate, maxCoordinate);
		SpatialOrganizationManager<PhysicalNode> firstSOM = new SpatialOrganizationManager<PhysicalNode>(
				policy, 0, 1000000000L);
		policy.registerNewSOM(firstSOM);
		return firstSOM;
	}
	
	public int runSingleTest(long seed, int printAt, LinkedList<Integer> checkAt) {
		System.gc();
		rand = new Random(seed);
		System.out.println("\nTest case #"+totalCounter+": Trying seed "+seed);
		totalCounter++;
		createOutPut = false;
		SpaceNode.clear();
		Tetrahedron.clear();
		ini.cx3d.parallelSpatialOrganization.SpaceNode.clear();
		ini.cx3d.parallelSpatialOrganization.Tetrahedron.clear();
//		Tetrahedron<T> startTetrahedron = Tetrahedron.createInitialTetrahedron(new SpaceNode<T>(
//				0.0, 0.0, 0.0, null), new SpaceNode<T>(1.0,
//				0.0, 0.0, null), new SpaceNode<T>(0.0, 1.0,
//				0.0, null), new SpaceNode<T>(0.0, 0.0, 1.0, null));
//		Tetrahedron<T> startTetrahedron = null;
		SpatialOrganizationNode<PhysicalNode>[] innerNodes = new SpatialOrganizationNode[initialNodeCount+insertNodeCount];
		PhysicalNode dummyNode = new PhysicalNode();
		// insert extra nodes:
		System.out.println("insert nodes...");
		for (int i = initialNodeCount; i < innerNodes.length; i++) {
			if (i % 100 == 0)
				System.out.print(".");
			if (i % 10000 == 0)
				System.out.println("");
			double[] position = new double[] { 
					rand.nextDouble()*2*volumeConstant - volumeConstant,
					rand.nextDouble()*2*volumeConstant - volumeConstant,
					rand.nextDouble()*2*volumeConstant - volumeConstant					
			};
//			innerNodes[i] = new SpaceNode<T>(
//, null);
//					rand(-volumeConstant, volumeConstant + 1),
//					rand(-volumeConstant,volumeConstant + 1), 
//					rand(-volumeConstant,volumeConstant + 1));
			SpatialOrganizationNode<PhysicalNode> closest = null;
			double minDistance = Double.MAX_VALUE;
			for (int j = i-100; j < i; j++) {
				if (j >= 0) {
					double dummy = norm(subtract(innerNodes[j].getPosition(), position));
					if (dummy < minDistance) {
						minDistance = dummy;
						closest = innerNodes[j];
					}
				}
					
			}
//
//				startTetrahedron = closest.getAdjacentTetrahedra().first();
			try {
				if (closest != null) {
					innerNodes[i] = closest.getNewInstance(position, dummyNode);
				}
				else {
					/* FRED (12 march 2008) :
					// I COMMENTED THIS OUT, WHEN I DELETED THE FILED ECM.triangulationType
					*/
//					if (ECM.triangulationType == ECM.OLD_TRIANGULATION) {
//						innerNodes[i] = (FredNode)FredNode.getInitialNode(position, dummyNode);
//					}
//					else {
					if (!useParallelSpatialOrganization)
						innerNodes[i] = new SpaceNode<PhysicalNode>(position,dummyNode);
					else 
						innerNodes[i] = initializeSOMs().createInitialNode(position, dummyNode);
//					}
					
				}
//				startTetrahedron = innerNodes[i].insert(startTetrahedron);
			}
			catch (PositionNotAllowedException e) {
				if (ini.cx3d.parallelSpatialOrganization.SpaceNode.allNodes != null)
					ini.cx3d.parallelSpatialOrganization.SpaceNode.allNodes.remove(innerNodes[i]);
				i--;
			}
		}
		
		checkConvexHull();

		if (checkAt != null && checkAt.contains(0)) {
			boolean problems = false;
			if (Tetrahedron.allTetrahedra != null)
			for (Tetrahedron<PhysicalNode> tetrahedron : Tetrahedron.allTetrahedra) {
				tetrahedron.calculateCircumSphere();
				if (checkTetrahedronForDelaunayViolation(tetrahedron)) break;
			}
			if (problems) {
				System.out.println("Run number: "+0);
				if (repair) {
					System.out.println("repairing by flipping...");
					if (SpaceNode.allNodes != null) 
					for (SpaceNode<PhysicalNode> node : SpaceNode.allNodes)
						node.restoreDelaunay();
				}
				throw new RuntimeException("Delaunay criterion is not fullfilled!");
			}
		}
		
		
		// now move!
		int runNumber = 0;
		try {
		boolean done = false;
		while (!done) {
			SpatialOrganizationNode<PhysicalNode> anyNode = innerNodes[rand(0,innerNodes.length)];
			double[] delta = new double[3];
			delta[rand(0,3)] = rand(-1,2);
			
			int rand2 = rand(1,50);
			
			for (int i = 0; i < rand2; i++) {
				try {
					if (runNumber % 1000 == 0)
						System.out.println("Run number "+runNumber);
					out("Run number "+runNumber);
					if (runNumber == movesPerRun) {
						done = true;
						break;
					}
					if (runNumber == printAt)
						createOutPut = true;
					else 
						createOutPut = false;

					runNumber++;
					anyNode.moveFrom(delta);
					
//					if (SpaceNode.allNodes.size() < innerNodes.length)
//						throw new RuntimeException("Knoten verloren!");
					// reflect if boundary reached:
					boolean anyProblem = false;
					if (Math.abs(anyNode.getPosition()[0]) > volumeConstant) {
						delta[0] = -1*Math.signum(anyNode.getPosition()[0]);
						anyProblem = true;
					}
					if (Math.abs(anyNode.getPosition()[1]) > volumeConstant) {
						delta[1] = -1*Math.signum(anyNode.getPosition()[1]);
						anyProblem = true;
					}
					if (Math.abs(anyNode.getPosition()[2]) > volumeConstant) {
						delta[2] = -1*Math.signum(anyNode.getPosition()[2]);
						anyProblem = true;
					}
					if (!anyProblem) delta[rand(0,3)] += rand.nextDouble()*2-1;//rand(-1,2);
					else i--;
					if (checkAt != null && checkAt.contains(runNumber)) {
						boolean problems = false;
						if (Tetrahedron.allTetrahedra != null)
						for (Tetrahedron<PhysicalNode> tetrahedron : Tetrahedron.allTetrahedra) {
							tetrahedron.calculateCircumSphere();
//							SpaceNode node = anyNode;
							for (SpaceNode<PhysicalNode> node : SpaceNode.allNodes) {
								if (tetrahedron.isFlat())
									throw new RuntimeException("There is a flat tetrahedron left over!");
								if (!tetrahedron.isAdjacentTo(node) && !tetrahedron.isFlat())
								if (tetrahedron.isTrulyInsideSphere(node.getPosition())) {
//									System.out.print("The Node "+node+"lies inside the circumsphere of tetrahedron "+tetrahedron+"!");
//									if (tetrahedron.orientationExact(node.getPosition()) <= 0)
//										System.out.print(" (but not truly!)");
//									System.out.println("");
									double[] distVector = subtract(tetrahedron.circumCenter,node.getPosition());
									problems = true;
									tetrahedron.isTrulyInsideSphere(node.getPosition());
								}
							}
						}
						if (problems) {
							done = true;
							System.out.println("Run number: "+runNumber);
							if (repair) {
								System.out.println("repairing by flipping...");
								for (SpaceNode<PhysicalNode> node : SpaceNode.allNodes)
									node.restoreDelaunay();
							}
							throw new RuntimeException("Delaunay criterion is not fullfilled!");
						}
					}
				} 
				catch (PositionNotAllowedException e) {
				}
			}
		}
		
		System.out.println("interrupted!");
		} catch (Exception e) {
			System.out.println("Run number: "+runNumber);
			System.out.println("Seed was "+seed);
			e.printStackTrace();
			return runNumber;
		}
		analyzeDoubleArray(OpenTriangleOrganizer.toleranceValues, "SD-Distance tolerance-values");
		displayStats();
		return runNumber;
	}
	
	public static void displayStats() {
		System.out.println("Maximal tolerance intervall needed for SD-Distance: "+OpenTriangleOrganizer.maxToleranceNeeded);
		System.out.println("\nMaximal tolerance intervall needed for Tetrahedron-orientation: "+Tetrahedron.maxToleranceNeeded);
		System.out.println("Exact calculation for Tetrahedron-orientation was calculated "+Tetrahedron.toleranceUsages+" times.");
		System.out.println("Flip-Movements: "+SpaceNode.flipMovements);
		System.out.println("Delete & Insert-Movements: "+SpaceNode.deleteAndInsertMovements);
		System.out.println("SD-Distance-calculations normal: "+OpenTriangleOrganizer.normalCalculations);
		System.out.println("SD-Distance-calculations exact: "+OpenTriangleOrganizer.exactCalculations);
	}
	
	public int searchError(long seed, int runningTill) {
		Tetrahedron.allTetrahedra = new LinkedList<Tetrahedron>();
		int stepSize = (runningTill+4)/5;
		int start = 0, end = runningTill;
		while (stepSize > 0) {
			LinkedList<Integer> checkAt = new LinkedList<Integer>();
			for (int pos = start; pos < end; pos += stepSize) {
				checkAt.add(pos);
			}
			end = runSingleTest(seed, -1, checkAt);
			start = end - stepSize;
			stepSize = (stepSize+3)/5;
		}
		return end;
	}
	public boolean runSeed(long seed) {
		Tetrahedron.allTetrahedra = null;
		int completedTill = runSingleTest(seed, -1, null);
		if (completedTill == this.movesPerRun) return true;
		else {
			System.out.println("Error at run number "+completedTill+" running seed "+seed);
			int problemRound = searchError(seed,completedTill);
			System.out.println("Error running seed "+seed+" was originally caused in run #"+problemRound); ;
			return false;
		}
	}
	public void run() {
		if (useInitialSeed) {
			if (this.printRun < 0)
				runSeed(initialSeed);
			else if ((checks == null) || (checks.length == 0))
				runSingleTest(initialSeed, printRun, null);
			else {
				LinkedList<Integer> checks = new LinkedList<Integer>();
				for (int i = 0; i < this.checks.length; i++) {
					checks.add(this.checks[i]);
				}
				Tetrahedron.allTetrahedra = new LinkedList<Tetrahedron>(); 
				runSingleTest(initialSeed, printRun, checks);
			}
		}
		else {
			Tetrahedron.allTetrahedra = null;
			Random generator = new Random();
			int completedTill = this.movesPerRun;
			long seed = generator.nextLong();
			while (runSeed(seed)) {
				seed = generator.nextLong();
			}
		}
	}
	
	public NewDelaunayTest() {
		run();
		
	}
	
	public void abstellGleis() {
		System.out.println("Version 124.0");
//		oto.putTriangle(startTetrahedron.getAdjacentTriangles()[0]);
//		oto.putTriangle(startTetrahedron.getAdjacentTriangles()[1]);
//		oto.putTriangle(startTetrahedron.getAdjacentTriangles()[2]);
//		oto.putTriangle(startTetrahedron.getAdjacentTriangles()[3]);
//		AbstractTriangulationNodeOrganizer tno = oto.getTriangulationNodeOrganizer();
//		
//		int insertNodeCount = 50000;
//		int volumeConstant = 100;
		Random generator = new Random();
		long seed = 0;
		int totalCounter = 0;
		while (true) {
			seed = generator.nextLong();
//			long seed = 318763603437864272l; -8332458767730160183
			seed = 7078964004630023721l;
			rand = new Random(seed);
			System.out.println("\nTest case #"+totalCounter+": Trying seed "+seed);
			totalCounter++;
			createOutPut = false;
			SpaceNode.clear();
			Tetrahedron.clear();
			Tetrahedron<PhysicalNode> startTetrahedron = Tetrahedron.createInitialTetrahedron(new SpaceNode<PhysicalNode>(
					0.0, 0.0, 0.0, null), new SpaceNode<PhysicalNode>(1.0,
					0.0, 0.0, null), new SpaceNode<PhysicalNode>(0.0, 1.0,
					0.0, null), new SpaceNode<PhysicalNode>(0.0, 0.0, 1.0, null));
			OpenTriangleOrganizer oto = OpenTriangleOrganizer.createSimpleOpenTriangleOrganizer();
			SpaceNode<PhysicalNode>[] innerNodes = new SpaceNode[initialNodeCount+insertNodeCount];
	//		// lets create some random nodes in a cubic volume, all nodes sitting at integer positions:
	//		for (int i = 0; i < initialNodeCount; i++) {
	//			innerNodes[i] = new SpaceNode(rand(
	//					-volumeConstant, volumeConstant + 1),
	//					rand(-volumeConstant,
	//							volumeConstant + 1), rand(
	//							-volumeConstant,
	//							volumeConstant + 1));
	//			boolean add = true;
	//			for (SpaceNode node2 : SpaceNode.allNodes) {
	//				if (node2 != innerNodes[i])
	//				if (norm(subtract(innerNodes[i].getPosition(),node2.getPosition())) < 0.00000001) {
	//					add = false;
	//					i--;
	//					break;
	//				}
	//			}
	//			if (add)
	//				tno.addNode(innerNodes[i]);
	//			else {
	//				System.out.println(""+SpaceNode.allNodes.remove(innerNodes[i+1]));
	//			}
	//		}
	//		for (SpaceNode node : SpaceNode.allNodes) {
	//			for (SpaceNode node2 : SpaceNode.allNodes) {
	//				if (node != node2)
	//				if (norm(subtract(node.getPosition(),node2.getPosition())) < 0.00000001) {
	//					System.out.println("Node "+node+" and node "+node2+" lie at the same position");
	//				}
	//			}
	//		}
	//
	//		System.out.println("\ntriangulate...");
	//		oto.triangulate();
			
			// insert extra nodes:
			System.out.println("insert nodes...");
			SpaceNode<PhysicalNode>[] last100 = new SpaceNode[100];
			for (int i = initialNodeCount; i < innerNodes.length; i++) {
				if (i % 100 == 0)
					System.out.print(".");
				if (i % 10000 == 0)
					System.out.println("");
	//			checkConvexHull();
				innerNodes[i] = new SpaceNode<PhysicalNode>(
						rand.nextDouble()*2*volumeConstant - volumeConstant,
						rand.nextDouble()*2*volumeConstant - volumeConstant,
						rand.nextDouble()*2*volumeConstant - volumeConstant, null	);
//						rand(-volumeConstant, volumeConstant + 1),
//						rand(-volumeConstant,volumeConstant + 1), 
//						rand(-volumeConstant,volumeConstant + 1));
//				if (innerNodes[i].getId() == 13) {
//					System.out.println("Now!");
//				}
//				Tetrahedron someTetrahedron = Tetrahedron.allTetrahedra.get(rand(1,Tetrahedron.allTetrahedra.size()));
				SpaceNode<PhysicalNode> closest = null;
				double minDistance = Double.MAX_VALUE;
				for (int j = i-100; j < i; j++) {
					if (j >= 0) {
						double dummy = norm(subtract(innerNodes[j].getPosition(), innerNodes[i].getPosition()));
						if (dummy < minDistance) {
							minDistance = dummy;
							closest = innerNodes[j];
						}
					}
						
				}
				if (closest != null) {
					startTetrahedron = closest.getAdjacentTetrahedra().getFirst();
				}
				try {
					startTetrahedron = innerNodes[i].insert(startTetrahedron);
				}
				catch (PositionNotAllowedException e) {
					if (SpaceNode.allNodes != null) 
						SpaceNode.allNodes.remove(innerNodes[i]);
					i--;
				}
			}
//			for (SpaceNode node : SpaceNode.allNodes) {
//				out("Position von Knoten "+node+": ("+node.getPosition()[0]+", "+node.getPosition()[1]+", "+node.getPosition()[2]+")");
//			}
			
			// now move!
			int runNumber = 0;
			try {
			boolean done = false;
			while (!done) {
				SpaceNode<PhysicalNode> anyNode = innerNodes[rand(0,innerNodes.length)];
	//			System.out.println("\nmoving point "+anyNode+" a little bit...");
				double[] delta = new double[3];
				delta[rand(0,3)] = rand(-1,2);
				
				int rand2 = rand(1,50);
				
				for (int i = 0; i < rand2; i++) {
					try {
						if (runNumber % 1000 == 0)
							System.out.println("Run number "+runNumber);
						out("Run number "+runNumber);
						if (runNumber == 20000) {
//							throw new RuntimeException("finish!");
							done = true;
							break;
						}
						if (runNumber == 5743)
							createOutPut = true;
						else 
							createOutPut = false;
	
						runNumber++;
						anyNode.moveFrom(delta);
						if (SpaceNode.allNodes != null) 
							if (SpaceNode.allNodes.size() < innerNodes.length+4)
								throw new RuntimeException("Knoten verloren!");
						// reflect if boundary reached:
						boolean anyProblem = false;
						if (Math.abs(anyNode.getPosition()[0]) > volumeConstant) {
							delta[0] = -1*Math.signum(anyNode.getPosition()[0]);
							anyProblem = true;
						}
						if (Math.abs(anyNode.getPosition()[1]) > volumeConstant) {
							delta[1] = -1*Math.signum(anyNode.getPosition()[1]);
							anyProblem = true;
						}
						if (Math.abs(anyNode.getPosition()[2]) > volumeConstant) {
							delta[2] = -1*Math.signum(anyNode.getPosition()[2]);
							anyProblem = true;
						}
						if (!anyProblem) delta[rand(0,3)] += rand.nextDouble()*2-1;//rand(-1,2);
						else i--;
						if (runNumber > 115742) {
							boolean problems = false;
							if (Tetrahedron.allTetrahedra != null)
							for (Tetrahedron<PhysicalNode> tetrahedron : Tetrahedron.allTetrahedra) {
								tetrahedron.calculateCircumSphere();
	//							SpaceNode node = anyNode;
								if (SpaceNode.allNodes != null) 
								for (SpaceNode<PhysicalNode> node : SpaceNode.allNodes) {
									if (tetrahedron.isFlat())
										throw new RuntimeException("There is a flat tetrahedron left over!");
									if (!tetrahedron.isAdjacentTo(node) && !tetrahedron.isFlat())
									if (tetrahedron.isTrulyInsideSphere(node.getPosition())) {
										System.out.println("The Node "+node+"lies inside the circumsphere of tetrahedron "+tetrahedron+"!");
										
										double[] distVector = subtract(tetrahedron.circumCenter,node.getPosition());
//										System.out.println("The distance to the circumcenter is "+norm(distVector)+", the radius is "+Math.sqrt(tetrahedron.squaredRadius));
										problems = true;
										tetrahedron.isTrulyInsideSphere(node.getPosition());
										//break;
									}
								}
							}
							if (problems) {
								done = true;
								System.out.println("Run number: "+runNumber);
								for (SpaceNode<PhysicalNode> node : SpaceNode.allNodes)
									node.restoreDelaunay();
								throw new RuntimeException("Delaunay criterion is not fullfilled!");
							}
						}
	//					checkConvexHull();
	//					System.out.println("");
					} 
					catch (PositionNotAllowedException e) {
						
	//					System.out.println("Position not allowed!");
					}
				}
			}
			
			System.out.println("interrupted!");
			} catch (Exception e) {
				System.out.println("Run number: "+runNumber);
				System.out.println("Seed was "+seed);
				e.printStackTrace();
				break;
			}
//			for (SpaceNode node : SpaceNode.allNodes) {
//				for (SpaceNode node2 : SpaceNode.allNodes) {
//					if (node != node2)
//					if (norm(subtract(node.getPosition(),node2.getPosition())) < 0.00000001) {
//						System.out.println("Node "+node+" and node "+node2+" lie at the same position");
//					}
//				}
//			}

			//			break;
		}
//		for (SpaceNode node : SpaceNode.allNodes) {
//			System.out.println("Position von Knoten "+node+": ("+node.getPosition()[0]+", "+node.getPosition()[1]+", "+node.getPosition()[2]+")");
//		}
		System.out.println("Seed was "+seed);
		
//		tno.addNode(new SpaceNode(-10,0,0));
//		tno.addNode(new SpaceNode(10,0,0));
//		tno.addNode(new SpaceNode(0,-10,0));
//		tno.addNode(new SpaceNode(0,10.0,0));
//		tno.addNode(new SpaceNode(0,0,-10));
//		tno.addNode(new SpaceNode(0,0,100.0));

		
//		double sqrt2 = Math.sqrt(2);
//		tno.addNode(new SpaceNode(10/sqrt2,10/sqrt2,0));
//		tno.addNode(new SpaceNode(10/sqrt2,-10/sqrt2,0));
//		tno.addNode(new SpaceNode(-10/sqrt2,-10/sqrt2,0));
//		tno.addNode(new SpaceNode(-10/sqrt2,10/sqrt2,0));

//		System.out.println("triangulating...");
//		oto.triangulate();
//		Tetrahedron someValidTetrahedron = oto.getANewTetrahedron();
//		someValidTetrahedron = (new SpaceNode(0,0,0)).insert(someValidTetrahedron);
//		
//		System.out.println("\nall Tetrahedra:");
//		for (Tetrahedron tetrahedron : Tetrahedron.allTetrahedra) {
//			System.out.println(" "+tetrahedron);
//		}
//		
//		System.out.println("\nmoving point #10 to 0/7/0");
//		SpaceNode.allNodes.get(10).moveTo(new double[] {0.0,7.0,0.0});
//		System.out.println("\nmoving point #5 to 1/0/0");
//		SpaceNode.allNodes.get(5).moveTo(new double[] {1.0,0.0,0.0});
		//		for (int i = 0; i < 200; i++) {
//			System.out.println("Inserting node "+(i+10000));
//			someValidTetrahedron = (new SpaceNode(rand.nextDouble()*200.0-100.0,rand.nextDouble()*200.0-100.0,rand.nextDouble()*200.0-100.0)).insert(someValidTetrahedron);
//		}
//		new SpaceNode(rand.nextDouble()*200.0-100.0,rand.nextDouble()*200.0-100.0,rand.nextDouble()*200.0-100.0);
//		(new SpaceNode(0,0,0)).insert(oto.getANewTetrahedron());
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		new NewDelaunayTest();
	}

}
