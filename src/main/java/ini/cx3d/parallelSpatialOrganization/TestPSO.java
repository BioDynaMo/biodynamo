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

package ini.cx3d.parallelSpatialOrganization;

import static ini.cx3d.utilities.Matrix.*;

import java.io.PrintStream;
import java.util.LinkedList;
import java.util.Random;

import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.spatialOrganization.PositionNotAllowedException;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;

public class TestPSO {
	public final int initialNodeCount = 0;
	public final int insertNodeCount = 10000;
	public final int volumeConstant = 30;
	public final int movesPerRun = 10000;
	public double avgnodedistance = ((double)volumeConstant) / Math.pow(initialNodeCount+insertNodeCount, 1.0/3.0);
	// public final long initialSeed = -4399871388000337276l;
	// public final long initialSeed = -2727759062064147475l;
	// public final long initialSeed = 3189782250572290986l; ;
	// public final long initialSeed = -3202748494647697604l;
	// public final long initialSeed = -1518617032522254875l;
	public final long initialSeed = -8282765079149723159l;

	// public final long initialSeed = -1804927093357226749l;
	// public final long initialSeed = 6646313007125023494l;
	// public final long initialSeed = -2061350812195586211l;
	// public final long initialSeed = 1016542914281514000l;
	public final boolean useInitialSeed = true;
	public final int printRun = -1;// 16742;
	public final int[] checks = new int[] {};
	public final boolean repair = false;
	public static Random rand = new Random(1312);
	Tetrahedron<PhysicalNode> outerTetrahedron;
	int totalCounter = 0;
	private boolean runElaborateTests = false;
	public static PrintStream out;
	private static boolean createOutPut = true;
	LinkedList<SpatialOrganizationManager<Integer>> soms = new LinkedList<SpatialOrganizationManager<Integer>>();
	private int somCount = 1;

	public static int rand(int min, int max) {

		int ret = (int) (rand.nextDouble() * (double) (max - min)) + min;
		// System.out.println(ret);
		return ret;
	}

	public static void out(String s) {
		if (createOutPut)
			System.out.println(s);
	}

	public static void out(double d) {
		out(Double.toString(d));
	}

	public void checkConvexHull(SpatialOrganizationManager<Integer> som) throws NodeLockedException,
			ManagedObjectDoesNotExistException {
		if (runElaborateTests)
//		if (Tetrahedron.allTetrahedra != null)
			for (Tetrahedron<Integer> tet : som.tetrahedra.values()) {
				if (tet.isInfinite()) {
					Triangle3D<Integer> convexhullTriangle = tet
							.getAdjacentTriangles()[0];
					convexhullTriangle.updatePlaneEquationIfNecessary();
					if (convexhullTriangle.getOppositeTetrahedron(tet) != null) {
						SpaceNode<Integer> innerNode = convexhullTriangle
								.getOppositeTetrahedron(tet).getOppositeNode(
										convexhullTriangle);
						for (SpaceNode<Integer> node : som.nodes.values()) {
							if (!tet.isAdjacentTo(node)) {
								if (!convexhullTriangle.onSameSide(innerNode
										.getPosition(), node.getPosition())) {
									System.out
											.println("The node "
													+ node
													+ " doesn't lie on the inner side of "
													+ convexhullTriangle + "!");
								}
							}
						}
					}
				}
			}
	}

	public static boolean checkTetrahedronForDelaunayViolation(
			Tetrahedron<Integer> tetrahedron, SpatialOrganizationManager<Integer> som) throws NodeLockedException,
			ManagedObjectDoesNotExistException {
		boolean problems = false;
		for (SpaceNode<Integer> node : som.nodes.values()) {
			if (tetrahedron.isFlat())
				throw new RuntimeException(
						"There is a flat tetrahedron left over!");
			if (!tetrahedron.isAdjacentTo(node) && !tetrahedron.isFlat())
				if (tetrahedron.isTrulyInsideSphere(node.getPosition())) {
					System.out.print("The Node " + node
							+ "lies inside the circumsphere of tetrahedron "
							+ tetrahedron + "!");
					if (tetrahedron.orientationExact(node.getPosition()) <= 0)
						System.out.print(" (but not truly!)");
					System.out.println("");
					double[] distVector = subtract(tetrahedron.circumCenter,
							node.getPosition());
					problems = true;
					tetrahedron.isTrulyInsideSphere(node.getPosition());
				}
		}
		return problems;
	}

	public int runSingleTest(long seed, int printAt, LinkedList<Integer> checkAt, SpatialOrganizationManager<Integer> firstSOM) {
		rand = new Random(seed);
		System.out.println("\nTest case #" + totalCounter + ": Trying seed "
				+ seed);
		totalCounter++;
		createOutPut = false;
		SpaceNode.clear();
		Tetrahedron.clear();
		// Tetrahedron<T> startTetrahedron =
		// Tetrahedron.createInitialTetrahedron(new SpaceNode<T>(
		// 0.0, 0.0, 0.0, null), new SpaceNode<T>(1.0,
		// 0.0, 0.0, null), new SpaceNode<T>(0.0, 1.0,
		// 0.0, null), new SpaceNode<T>(0.0, 0.0, 1.0, null));
		// Tetrahedron<T> startTetrahedron = null;
		SpatialOrganizationNode<Integer>[] innerNodes = new SpatialOrganizationNode[initialNodeCount
				+ insertNodeCount];
		double[][] coordinates = new double[initialNodeCount + insertNodeCount][];
		Integer dummyNode = new Integer(0);
		// insert extra nodes:
		System.out.println("insert nodes...");
		for (int i = initialNodeCount; i < innerNodes.length; i++) {
			if (i % 100 == 0) {
				System.out.print(".");
//				System.out.println();
				firstSOM.printStats();
//				System.out.println("Test-Int is: "+rand.nextInt());
				System.out.println("");
//				System.gc();
			}
			if (i % 10000 == 0)
				System.out.println("");
			double[] position = new double[] {
					rand.nextDouble() * 2 * volumeConstant - volumeConstant,
					rand.nextDouble() * 2 * volumeConstant - volumeConstant,
					rand.nextDouble() * 2 * volumeConstant - volumeConstant };
			coordinates[i] = position;
			// innerNodes[i] = new SpaceNode<T>(
			// , null);
			// rand(-volumeConstant, volumeConstant + 1),
			// rand(-volumeConstant,volumeConstant + 1),
			// rand(-volumeConstant,volumeConstant + 1));
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
			//
			// startTetrahedron = closest.getAdjacentTetrahedra().first();
			try {
				if (closest != null) {
//					synchronized (firstSOM.stopLock) {
						
//					}
					innerNodes[i] = closest.getNewInstance(position, new Integer(0));
				} else {
					/*
					 * FRED (12 march 2008) : // I COMMENTED THIS OUT, WHEN I
					 * DELETED THE FILED ECM.triangulationType
					 */
					// if (ECM.triangulationType == ECM.OLD_TRIANGULATION) {
					// innerNodes[i] =
					// (FredNode)FredNode.getInitialNode(position, dummyNode);
					// }
					// else {
					 innerNodes[i] = firstSOM.createInitialNode(
								position, new Integer(0));
				}
				// startTetrahedron = innerNodes[i].insert(startTetrahedron);
			} catch (PositionNotAllowedException e) {
//				SpaceNode.allNodes.remove(innerNodes[i]);
				i--;
			}
		}

		try {
			checkConvexHull(firstSOM);
		} catch (NodeLockedException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		} catch (ManagedObjectDoesNotExistException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}

		if (checkAt != null && checkAt.contains(0)) {
			boolean problems = false;
			if (runElaborateTests)
//			if (Tetrahedron.allTetrahedra != null)
				for (Tetrahedron<Integer> tetrahedron : firstSOM.tetrahedra.values()) {
					try {
						tetrahedron.calculateCircumSphere();
						if (checkTetrahedronForDelaunayViolation(tetrahedron, firstSOM))
							break;
					} catch (NodeLockedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					} catch (ManagedObjectDoesNotExistException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			if (problems) {
				System.out.println("Run number: " + 0);
// Repairing would have to be cached, too. Therefore, I commented this out:
//				if (repair) {
//					System.out.println("repairing by flipping...");
//					for (SpaceNode<Integer> node : SpaceNode.allNodes) {
//						// TODO: handle exceptions carefully!
//						try {
//							node.restoreDelaunay();
//						} catch (NodeLockedException e) {
//						} catch (ManagedObjectDoesNotExistException e) {
//						}
//					}
//				}
				throw new RuntimeException(
						"Delaunay criterion is not fullfilled!");
			}
		}

		// now move!
		int runNumber = 0;
		try {
			boolean done = false;
			while (!done) {
				SpatialOrganizationNode<Integer> anyNode = innerNodes[rand(0,
						innerNodes.length)];
				double[] delta = new double[3];
				delta[0] = (rand.nextDouble()-0.5)*2*avgnodedistance;
				delta[1] = (rand.nextDouble()-0.5)*2*avgnodedistance;
				delta[2] = (rand.nextDouble()-0.5)*2*avgnodedistance;
//				delta[rand(0, 3)] = rand(-1, 2);

				int rand2 = rand(1, 50);

//				for (int i = 0; i < rand2; i++) {
					try {
						if (runNumber % 100 == 0) {
//							System.gc();
							firstSOM.printStats();
//							System.out.println("Run: "+runNumber);
	//						System.out.println("Test-Int is: "+rand.nextInt());
							System.out.println("");
						}
						if (runNumber % 1000 == 0)
							System.out.println("Run number " + runNumber);
						if (runNumber == 538)
							System.out.println("Stop...");
						out("Run number " + runNumber);
						if (runNumber == movesPerRun) {
							done = true;
							break;
						}
						if (runNumber == printAt)
							createOutPut = true;
						else
							createOutPut = false;

						runNumber++;
//						synchronized (firstSOM.stopLock) {
							
//						}
						anyNode.moveFrom(delta);

						// if (SpaceNode.allNodes.size() < innerNodes.length)
						// throw new RuntimeException("Knoten verloren!");
						// reflect if boundary reached:
						boolean anyProblem = false;
						if (Math.abs(anyNode.getPosition()[0]) > volumeConstant) {
							delta[0] = -1
									* Math.signum(anyNode.getPosition()[0]);
							anyProblem = true;
						}
						if (Math.abs(anyNode.getPosition()[1]) > volumeConstant) {
							delta[1] = -1
									* Math.signum(anyNode.getPosition()[1]);
							anyProblem = true;
						}
						if (Math.abs(anyNode.getPosition()[2]) > volumeConstant) {
							delta[2] = -1
									* Math.signum(anyNode.getPosition()[2]);
							anyProblem = true;
						}
						if (!anyProblem)
							delta[rand(0, 3)] += rand.nextDouble() * 2 - 1;// rand(-1,2);
//						else
//							i--;
						if (checkAt != null && checkAt.contains(runNumber)) {
							boolean problems = false;
							if (runElaborateTests)
//							if (Tetrahedron.allTetrahedra != null)
								for (Tetrahedron<Integer> tetrahedron : firstSOM.tetrahedra.values()) {
									tetrahedron.calculateCircumSphere();
									// SpaceNode node = anyNode;
									for (SpaceNode<Integer> node : firstSOM.nodes.values()) {
										if (tetrahedron.isFlat())
											throw new RuntimeException(
													"There is a flat tetrahedron left over!");
										if (!tetrahedron.isAdjacentTo(node)
												&& !tetrahedron.isFlat())
											try {
												if (tetrahedron
														.isTrulyInsideSphere(node
																.getPosition())) {
													// System.out.print("The
													// Node "+node+"lies inside
													// the circumsphere of
													// tetrahedron
													// "+tetrahedron+"!");
													// if
													// (tetrahedron.orientationExact(node.getPosition())
													// <= 0)
													// System.out.print(" (but
													// not truly!)");
													// System.out.println("");
													double[] distVector = subtract(
															tetrahedron.circumCenter,
															node.getPosition());
													problems = true;
													tetrahedron
															.isTrulyInsideSphere(node
																	.getPosition());
												}
											} catch (NodeLockedException e) {
												// TODO Auto-generated catch
												// block
												e.printStackTrace();
											} catch (ManagedObjectDoesNotExistException e) {
												// TODO Auto-generated catch
												// block
												e.printStackTrace();
											}
									}
								}
							if (problems) {
								done = true;
								System.out.println("Run number: " + runNumber);
//								if (repair) {
//									System.out
//											.println("repairing by flipping...");
//									// TODO: handle exceptions carefully!
//									try {
//										for (SpaceNode<Integer> node : SpaceNode.allNodes)
//											node.restoreDelaunay();
//									} catch (NodeLockedException e) {
//									} catch (ManagedObjectDoesNotExistException e) {
//									}
//								}
								throw new RuntimeException(
										"Delaunay criterion is not fullfilled!");
							}
						}
					} catch (PositionNotAllowedException e) {
					} catch (NodeLockedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					} catch (ManagedObjectDoesNotExistException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
	//			}
			}

			System.out.println("interrupted!");
		} catch (Exception e) {
			System.out.println("Run number: " + runNumber);
			System.out.println("Seed was " + seed);
			e.printStackTrace();
			return runNumber;
		}
		// analyzeDoubleArray(OpenTriangleOrganizer.toleranceValues,
		// "SD-Distance tolerance-values");
		// displayStats();
		return runNumber;
	}

	
	
	public boolean runSeed(long seed) {
		Tetrahedron.allTetrahedra = null;
		int completedTill = runSingleTest(seed, -1, null, initializeSOMs());
		if (completedTill == this.movesPerRun) return true;
		else {
			System.out.println("Error at run number "+completedTill+" running seed "+seed);
			int problemRound = searchError(seed,completedTill);
			System.out.println("Error running seed "+seed+" was originally caused in run #"+problemRound); ;
			return false;
		}
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
//		Tetrahedron.allTetrahedra = new LinkedList<Tetrahedron>();
		int stepSize = (runningTill+4)/5;
		int start = 0, end = runningTill;
		while (stepSize > 0) {
			LinkedList<Integer> checkAt = new LinkedList<Integer>();
			for (int pos = start; pos < end; pos += stepSize) {
				checkAt.add(pos);
			}
			end = runSingleTest(seed, -1, checkAt, initializeSOMs());
			start = end - stepSize;
			stepSize = (stepSize+3)/5;
		}
		return end;
	}
	
	
	public void run() {
		if (useInitialSeed) {
			if (this.printRun < 0)
				runSeed(initialSeed);
			else if ((checks == null) || (checks.length == 0))
				runSingleTest(initialSeed, printRun, null, initializeSOMs());
			else {
				LinkedList<Integer> checks = new LinkedList<Integer>();
				for (int i = 0; i < this.checks.length; i++) {
					checks.add(this.checks[i]);
				}
//				SpaceNode.allNodes = new LinkedList<SpaceNode>();
//				Tetrahedron.allTetrahedra = new LinkedList<Tetrahedron>(); 
				runSingleTest(initialSeed, printRun, checks, initializeSOMs());
			}
		}
		else {
			synchronized (this) {
				try {
					this.wait(100);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			SpaceNode.allNodes = null;
			Tetrahedron.allTetrahedra = null;
			Random generator = new Random();
			int completedTill = this.movesPerRun;
			long seed = generator.nextLong();
			while (runSeed(seed)) {
				seed = generator.nextLong();
			}
		}
	}
	
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
		for (SimpleAssignmentPolicy<Integer> policy : policies) {
			for (SpatialOrganizationManager<Integer> som : soms) 
				policy.registerNewSOM(som);
		}
		
		return soms.getFirst();
	}
	
	public TestPSO() {
		run();
		
	}
	
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		new TestPSO();
	}

}
