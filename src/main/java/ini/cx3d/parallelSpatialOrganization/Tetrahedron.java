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

import ini.cx3d.spatialOrganization.PositionNotAllowedException;

import java.lang.reflect.Array;
import java.util.Arrays;
import static ini.cx3d.utilities.Matrix.*;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import sun.security.action.GetLongAction;

/**
 * This class is used to represent a tetrahedron. Each instance saves references
 * to 4 incident nodes and 4 incident triangles. Additionally,
 * <code>Tetrahedron</code> stores information about the volume and the
 * circumsphere of the tetrahedron defined by the incident nodes.
 * 
 * Tetrahedra can either be finite or infinite. In the latter case, the first
 * node incident to this tetrahedron is set to <code>null</code>, indicating
 * that the other three endpoints of this tetrahedron are part of the convex
 * hull of all points in the current triangulation.
 * 
 * @author Dennis Goehlsdorf
 * 
 * @param <T> The type of the user objects stored in endpoints of a tetrahedron.
 */
//@SuppressWarnings("unchecked")
public class Tetrahedron<T>  extends ManagedObject<T> {
	private static final long serialVersionUID = -4705940971839404233L;

	public static final double TOLERANCE_SETTING = 0.001;
	// public static LinkedList<Tetrahedron> allTetrahedra = new
	// LinkedList<Tetrahedron>();

	/**
	 * Exclusively used for debugging purposes. Needs to be removed.
	 */
	public static LinkedList<Tetrahedron> allTetrahedra = null;
	/**
	 * Exclusively used for debugging purposes. Needs to be removed.
	 */
	public static int toleranceUsages = 0;
	/**
	 * Exclusively used for debugging purposes. Needs to be removed.
	 */
	public static double maxToleranceNeeded = 0.0;

	/**
	 * A small list containing a permutation of the number 0-3. This list is
	 * used to randomize the visibility walk implemented in
	 * {@link #walkToPoint(double[])}.
	 */
	private static List<Integer> triangleOrder = Arrays.asList(new Integer[] {
			0, 1, 2, 3 });

	// /**
	// * A global variable that keeps track of the number of Tetrahedra
	// generated since the start
	// * of the simulation.
	// */
	// private static int IDCOUNTER = 0;

	/**
	 * Exclusively used for debugging purposes. Needs to be removed.
	 */
	public static void clear() {
		if (allTetrahedra != null)
			allTetrahedra.clear();
		// IDCOUNTER = 0;
		triangleOrder = Arrays.asList(new Integer[] { 0, 1, 2, 3 });
	}

	// /**
	// * Each tetrahedron is assigned a unique ID. Not used.
	// */
	// private int id = IDCOUNTER++;

	/**
	 * Contains references to the nodes incident to this tetrahedron.
	 */
	ManagedObjectReference<T>[] adjacentNodes = new ManagedObjectReference[4];
//	SpaceNode<T>[] adjacentNodes = new SpaceNode[4];

	/**
	 * Contains references to the 6 edges incident to this tetrahedron.
	 */
	ManagedObjectReference<T>[] adjacentEdges = new ManagedObjectReference[6];
//	Edge[] adjacentEdges = new Edge[6];

	/**
	 * Contains references to the 4 triangles incident to this tetrahedron.
	 */
	ManagedObjectReference<T>[] adjacentTriangles = new ManagedObjectReference[4];
//	Triangle3D<T>[] adjacentTriangles = new Triangle3D[4];

	/**
	 * Saves for each incident edge this tetrahedron's contribution to the
	 * cross-section area of that edge.
	 */
	double[] crossSectionAreas = new double[6];

	/**
	 * The center of the circumsphere of this tetrahedron.
	 */
	double[] circumCenter = new double[3];

	/**
	 * The square of the radius of this tetrahedron's circumsphere.
	 */
	double squaredRadius = 0.0;

	/**
	 * Defines a tolerance intervall which is used in
	 * {@link #orientation(double[])}. If the distance of a given point to the
	 * center of the circumsphere of this tetrahedron has a difference to
	 * <code>squaredRadius</code> smaller than <code>tolerance</code>,
	 * exact mathematics are used to reliably calculate a decision whether that
	 * point lies inside, outside or on the circumsphere.
	 */
	double tolerance = 0.0000001;

	/**
	 * The volume of this tetrahedron.
	 */
	double volume = 0.0;

//	/**
//	 * The unique address of this ManagedObject
//	 */
//	protected long address = 0;
//	
//	/**
//	 * The active objects that owns this ManagedObject
//	 */
//	protected SpatialOrganizationManager som= null;
//
//	
//	//Identifies which transaction (if any) this node is included in.
//	private Integer transactionID = null;


	// /**
	// * Creates an outer tetrahedron that defines a valid triangulation area.
	// * A triangulation box will be created which has the sidelength
	// 'boxSideLength' and the center 'center'.
	// * @param boxSideLength The sidelength of the triangulation box.
	// * @param center The center of the triangulation box.
	// * @return An outer Tetrahedron.
	// */
	// public static Tetrahedron createOuterTetrahedron(double boxSideLength,
	// double[] center) {
	// double triangleSideLength = boxSideLength*3.0 / 4.0;
	// final SpaceNode nodeA = new
	// SpaceNode(center[0],triangleSideLength+center[1],
	// -triangleSideLength+center[2]);
	// final SpaceNode nodeB = new
	// SpaceNode(center[0],triangleSideLength+center[1],triangleSideLength+center[2]);
	// final SpaceNode nodeC = new
	// SpaceNode(-triangleSideLength+center[0],-triangleSideLength+center[1],center[2]);
	// final SpaceNode nodeD = new
	// SpaceNode(triangleSideLength,-triangleSideLength,0.0);
	// Triangle3D triangleA = new Triangle3D(nodeB, nodeC, nodeD, null, null) {
	// public void orientToOpenSide() { defineUpperSide(nodeA.getPosition());}
	// public boolean isOpenToSide(double[] point) { return !isClosed();} };
	// Triangle3D triangleB = new Triangle3D(nodeA, nodeC, nodeD, null, null) {
	// public void orientToOpenSide() { defineUpperSide(nodeA.getPosition());}
	// public boolean isOpenToSide(double[] point) { return !isClosed();} };
	// Triangle3D triangleC = new Triangle3D(nodeB, nodeA, nodeD, null, null) {
	// public void orientToOpenSide() { defineUpperSide(nodeA.getPosition());}
	// public boolean isOpenToSide(double[] point) { return !isClosed();} };
	// Triangle3D triangleD = new Triangle3D(nodeB, nodeC, nodeA, null, null) {
	// public void orientToOpenSide() { defineUpperSide(nodeA.getPosition());}
	// public boolean isOpenToSide(double[] point) { return !isClosed();} };
	// return new Tetrahedron(triangleA, triangleB, triangleC, triangleD, nodeA,
	// nodeB, nodeC, nodeD) {
	// public boolean isTrulyInsideSphere(double[] point) {
	// return false;
	// }
	// public boolean isInsideSphere(double[] point) {
	// return false;
	// }
	// protected boolean isPointInConvexPosition(double[] point, int
	// connectingTriangleNumber) {
	// return true;
	// }
	// public boolean isNeighbor(Tetrahedron otherTetrahedron) {
	// return false;
	// }
	// };
	// }

	/**
	 * Generates an initial tetrahedron for a new triangulation. A tetrahedron
	 * is generated which has the four given nodes as endpoints and is adjacent
	 * to four infinite tetrahedra.
	 * 
	 * @param <T>
	 *            The type of the user object stored in the incident nodes.
	 * @param a
	 *            The first incident node.
	 * @param b
	 *            The second incident node.
	 * @param c
	 *            The third incident node.
	 * @param d
	 *            The fourth incident node.
	 * @return A tetrahedron which is made out of the four points passed to this
	 *         function. This tetrahedron will be neighbor to 4 infinite
	 *         tetrahedra.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public static <T> Tetrahedron<T> createInitialTetrahedron(SpaceNode<T> a,
			SpaceNode<T> b, SpaceNode<T> c, SpaceNode<T> d, CacheManager<T> cm) throws NodeLockedException, ManagedObjectDoesNotExistException {
		SpatialOrganizationManager<T> som = a.getSOM();
		Triangle3D<T> triangleA = new Triangle3D<T>(b, c, d, null, null, som, cm);
		Triangle3D<T> triangleB = new Triangle3D<T>(a, c, d, null, null, som, cm);
		Triangle3D<T> triangleC = new Triangle3D<T>(a, b, d, null, null, som, cm);
		Triangle3D<T> triangleD = new Triangle3D<T>(a, b, c, null, null, som, cm);
		Tetrahedron<T> ret = new Tetrahedron<T>(triangleA, triangleB,
				triangleC, triangleD, a, b, c, d, cm);
		OpenTriangleOrganizer<T> oto = OpenTriangleOrganizer
				.createSimpleOpenTriangleOrganizer(som, cm);
		new Tetrahedron<T>(triangleA, null, oto, som, cm);
		new Tetrahedron<T>(triangleB, null, oto, som, cm);
		new Tetrahedron<T>(triangleC, null, oto, som, cm);
		new Tetrahedron<T>(triangleD, null, oto, som, cm);
		return ret;
	}

	/**
	 * Constructs a new tetrahedron from a given triangle and a fourth point.
	 * Missing triangles are created.
	 * The SOM responsible for this object is chosen based on the SOMs responsible for the incident objects.
	 * 
	 * @param oneTriangle
	 *            The triangle delivering 3 of the new tetrahedron's endpoints.
	 * @param fourthPoint
	 *            The fourth endpoint of the new tetrahedron.
	 * @param org
	 *            An organizer for open triangles which is used to keep track of
	 *            newly created triangles.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public Tetrahedron(Triangle3D<T> oneTriangle, SpaceNode<T> fourthPoint,
			OpenTriangleOrganizer<T> org, CacheManager<T> cm) throws NodeLockedException, ManagedObjectDoesNotExistException {
		this(oneTriangle, fourthPoint, org, (fourthPoint != null)?fourthPoint.getSOM():oneTriangle.getSOM(),cm);
	}

	/**
	 * Creates a new tetrahedron from four triangles and four points.
	 * The SOM responsible for this object is chosen based on the SOMs responsible for the incident objects.
	 * 
	 * @param triangleA
	 *            The first triangle.
	 * @param triangleB
	 *            The second triangle.
	 * @param triangleC
	 *            The third triangle.
	 * @param triangleD
	 *            The fourth triangle.
	 * @param nodeA
	 *            The first point, must lie opposite to triangleA.
	 * @param nodeB
	 *            The first point, must lie opposite to triangleB.
	 * @param nodeC
	 *            The first point, must lie opposite to triangleC.
	 * @param nodeD
	 *            The first point, must lie opposite to triangleD.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public Tetrahedron(Triangle3D<T> triangleA, Triangle3D<T> triangleB,
			Triangle3D<T> triangleC, Triangle3D<T> triangleD,
			SpaceNode<T> nodeA, SpaceNode<T> nodeB, SpaceNode<T> nodeC,
			SpaceNode<T> nodeD, CacheManager<T> cm) throws NodeLockedException, ManagedObjectDoesNotExistException {
		this(triangleA, triangleB, triangleC, triangleD, nodeA, nodeB, nodeC, nodeD, null, cm);
	}

	/**
	 * Constructs a new tetrahedron from a given triangle and a fourth point.
	 * Missing triangles are created.
	 * 
	 * @param oneTriangle
	 *            The triangle delivering 3 of the new tetrahedron's endpoints.
	 * @param fourthPoint
	 *            The fourth endpoint of the new tetrahedron.
	 * @param org
	 *            An organizer for open triangles which is used to keep track of
	 *            newly created triangles.
	 * @param som
	 *            The SpatialOrganizationManager that own this ManagedObject.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public Tetrahedron(Triangle3D<T> oneTriangle, SpaceNode<T> fourthPoint,
			OpenTriangleOrganizer<T> org, SpatialOrganizationManager<T> som, CacheManager<T> cm) throws NodeLockedException, ManagedObjectDoesNotExistException {
		initTracker(cm, som);
		cm.registerNewTetrahedron(this); 

		//		this.tracker = new ManagedObjectReference<T>(som);
		if (oneTriangle.isInfinite()) {
			SpaceNode<T>[] triangleNodes = oneTriangle.getNodes();
			oneTriangle = org.getTriangleWithoutRemoving(
					(triangleNodes[0] == null) ? triangleNodes[1]
							: triangleNodes[0],
					(triangleNodes[2] == null) ? triangleNodes[1]
							: triangleNodes[2], fourthPoint);
			fourthPoint = null;
		}
		if (fourthPoint != null) {
			adjacentNodes[0] = fourthPoint.getReference();
			// positionsInNodeLists[0] =
			// fourthPoint.getAdjacentTetrahedra().add(this);
			fourthPoint.addAdjacentTetrahedron(this);
		}
		else adjacentNodes[0] = null;
		// else
		// positionsInNodeLists[0] = null;

		SpaceNode<T>[] triangleNodes = oneTriangle.getNodes();
		for (int i = 0; i < triangleNodes.length; i++) {
			adjacentNodes[i + 1] = (triangleNodes[i]==null)?null:triangleNodes[i].getReference();
			// positionsInNodeLists[i+1] =
			// triangleNodes[i].getAdjacentTetrahedra().add(this);
			triangleNodes[i].addAdjacentTetrahedron(this);
		}

		Triangle3D<T>[] triangleCopies = new Triangle3D[4];
		
		triangleCopies[0] = oneTriangle;
		// add triangle and make sure that adjacentTriangles[i] lies opposite to
		// adjacentNodes[i]:

		if (!oneTriangle.isCompletelyOpen())
			org.removeTriangle(oneTriangle);

		triangleCopies[1] = org.getTriangle(fourthPoint, triangleNodes[1],triangleNodes[2]);
		triangleCopies[2] = org.getTriangle(fourthPoint, triangleNodes[0],triangleNodes[2]);
		triangleCopies[3] = org.getTriangle(fourthPoint, triangleNodes[0],triangleNodes[1]);

		for (int i = 0; i < 4; i++)
			adjacentTriangles[i] = (triangleCopies[i] == null)?null:triangleCopies[i].getReference();

		for (int i = 0; i < 4; i++)
			triangleCopies[i].addTetrahedron(this);

		registerEdges();

		// if ((this.getId() == 409331) || (this.getId() == 409224)) {
		// System.out.println("Tetrahedron.init");
		// }
		calculateCircumSphere();
		if (allTetrahedra != null)
			allTetrahedra.add(this);
		if (NewDelaunayTest.createOutput())
			NewDelaunayTest.out("created tetrahedron " + this);

	}

	/**
	 * Creates a new tetrahedron from four triangles and four points.
	 * 
	 * @param triangleA
	 *            The first triangle.
	 * @param triangleB
	 *            The second triangle.
	 * @param triangleC
	 *            The third triangle.
	 * @param triangleD
	 *            The fourth triangle.
	 * @param nodeA
	 *            The first point, must lie opposite to triangleA.
	 * @param nodeB
	 *            The first point, must lie opposite to triangleB.
	 * @param nodeC
	 *            The first point, must lie opposite to triangleC.
	 * @param nodeD
	 *            The first point, must lie opposite to triangleD.
	 * @param som
	 *            The SpatialOrganizationManager that own this ManagedObject.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public Tetrahedron(Triangle3D<T> triangleA, Triangle3D<T> triangleB,
			Triangle3D<T> triangleC, Triangle3D<T> triangleD,
			SpaceNode<T> nodeA, SpaceNode<T> nodeB, SpaceNode<T> nodeC,
			SpaceNode<T> nodeD, SpatialOrganizationManager<T> som, CacheManager<T> cm) throws NodeLockedException, ManagedObjectDoesNotExistException {
		// no som specified...
		if (som == null) {
			// find the SOM that's responsible for most of the incident nodes:
			som = (nodeA!=null)?nodeA.getSOM():
				((nodeB!=null)?nodeB.getSOM():
					((nodeC!=null)?nodeC.getSOM():
						nodeD.getSOM()));
			if (nodeB != null) {
				if (((nodeC !=null) && (nodeB.getSOM().equals(nodeC.getSOM()))) ||
						((nodeD !=null) && (nodeB.getSOM().equals(nodeD.getSOM()))))
					som = nodeB.getSOM();
			}
			if ((nodeC != null) && (nodeD != null) && (nodeC.getSOM().equals(nodeD.getSOM())))
				som = nodeC.getSOM();
		}

		initTracker(cm, som);
		cm.registerNewTetrahedron(this); 

		Triangle3D<T>[] triangles = new Triangle3D[4];

		triangles[0] = triangleA;
		triangles[1] = triangleB;
		triangles[2] = triangleC;
		triangles[3] = triangleD;
		for (int i = 0; i < 4; i++) 
			adjacentTriangles[i] = (triangles[i]==null)?null:triangles[i].getReference();
		adjacentNodes[0] = (nodeA==null)?null:nodeA.getReference();
		adjacentNodes[1] = (nodeB==null)?null:nodeB.getReference();
		adjacentNodes[2] = (nodeC==null)?null:nodeC.getReference();
		adjacentNodes[3] = (nodeD==null)?null:nodeD.getReference();
		SpaceNode<T>[] nodeCopies = getAdjacentNodes();
		triangles[0].addTetrahedron(this);
		if (nodeCopies[0] != null)
			nodeCopies[0].addAdjacentTetrahedron(this);
		// this.positionsInNodeLists[0] =
		// (adjacentNodes[0]!=null)?adjacentNodes[0].getAdjacentTetrahedra().add(this):null;
		for (int i = 1; i < 4; i++) {
			triangles[i].addTetrahedron(this);
			nodeCopies[i].addAdjacentTetrahedron(this);
			// this.positionsInNodeLists[i] =
			// adjacentNodes[i].getAdjacentTetrahedra().add(this);
		}
		registerEdges();
		calculateCircumSphere();
		if (allTetrahedra != null)
			allTetrahedra.add(this);
		if (NewDelaunayTest.createOutput())
			NewDelaunayTest.out("created tetrahedron " + this);
		
	}
	
	/**
	 * Copy constructor, creates a new Tetrahedron identical to the one passed.
	 * 
	 * @param origObj the object to copy from
	 */
	public Tetrahedron(Tetrahedron<T> origObj) {
		initTracker(origObj);
		adjacentTriangles[0] = origObj.adjacentTriangles[0];
		adjacentTriangles[1] = origObj.adjacentTriangles[1];
		adjacentTriangles[2] = origObj.adjacentTriangles[2];
		adjacentTriangles[3] = origObj.adjacentTriangles[3];
		adjacentNodes[0] = origObj.adjacentNodes[0];
		adjacentNodes[1] = origObj.adjacentNodes[1];
		adjacentNodes[2] = origObj.adjacentNodes[2];
		adjacentNodes[3] = origObj.adjacentNodes[3];
		
		// The edges simply have to be copied, 'registering' the edges is not necessary
		for (int i = 0; i < adjacentEdges.length; i++) 
			adjacentEdges[i] = origObj.adjacentEdges[i];
//		calculateCircumSphere();
//		if (allTetrahedra != null)
//			allTetrahedra.add(this);
		if (NewDelaunayTest.createOutput())
			NewDelaunayTest.out("created tetrahedron " + this);

		
		// also copy everything else:
		for (int i = 0; i < 6; i++)
			crossSectionAreas[i] = origObj.crossSectionAreas[i];
		for (int i = 0; i < 3; i++) 
			circumCenter[i] = origObj.circumCenter[i];
		this.squaredRadius = origObj.squaredRadius;
	}
	
	
	/**
	 * Extracts the user objects associated with the four endpoints of this
	 * tetrahedron.
	 * 
	 * @return An array of objects of type <code>T</code>.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected Object[] getVerticeContents() throws NodeLockedException, ManagedObjectDoesNotExistException {

		Object[] ret = new Object[4];
		SpaceNode<T>[] nodeCopies = getAdjacentNodes();
		for (int i = 0; i < 4; i++) {
			if ((nodeCopies[i] != null)
					&& (nodeCopies[i].getUserObject() != null)) {
				if ((ret == null)) {
					// System.out.println(adjacentNodes[i].getUserObject().getClass().getName());
					ret = (T[]) Array.newInstance(nodeCopies[i]
							.getUserObject().getClass(), 4);
				}
				ret[i] = nodeCopies[i].getUserObject();
			} else
				return null;
		}
		return ret;
	}

	/**
	 * Returns whether this tetrahedron is infinite.
	 * 
	 * @return <code>true</code>, if the tetrahedron is infinite (first
	 *         endpoint is null).
	 */
	protected boolean isInfinite() {
		return adjacentNodes[0] == null;
	}

	/**
	 * Returns whether this tetrahedron is a flat tetrahedron. Used to simplify
	 * distinction between the two types <code>Tetrahedron</code> and
	 * <code>FlatTetrahedron</code>.
	 * 
	 * @return <code>false</code> for all instances of
	 *         <code>Tetrahedron</code>.
	 */
	protected boolean isFlat() {
		return false;
	}

	/**
	 * Changes the cross section area associated with one incident edge. Informs
	 * incident edges if there is a change in their cross section area.
	 * 
	 * @param number
	 *            The index of the edge which cross section area should be
	 *            changed.
	 * @param newValue
	 *            The new value for the cross section area of the specified
	 *            edge.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected void changeCrossSection(int number, double newValue) throws NodeLockedException, ManagedObjectDoesNotExistException {
		double change = newValue - crossSectionAreas[number];
		if (change != 0)
			getAdjacentEdge(number).changeCrossSectionArea(change);
		crossSectionAreas[number] = newValue;
	}

	/**
	 * Calculates all cross section areas of the six edges incident to this
	 * tetrahedron.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected void updateCrossSectionAreas() throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (isInfinite())
			for (int i = 0; i < 6; i++)
				changeCrossSection(i, 0.0);
		else {
			double[][] lineMiddles = new double[6][3];
			double[][] lineVectors = new double[6][3];
			double[][] areaMiddles = new double[4][3];
			double[] tetraMiddle = { 0.0, 0.0, 0.0 };
			double[][] positions = new double[][] {
					getAdjacentNode(0).getPosition(),
					getAdjacentNode(1).getPosition(),
					getAdjacentNode(2).getPosition(),
					getAdjacentNode(3).getPosition() };
			// i: dimension: x, y, z
			for (int i = 0; i < 3; i++) {
				for (int j = 0, lineCounter = 0; j < 4; j++) {
					tetraMiddle[i] += positions[j][i];
					for (int k = j + 1; k < 4; k++, lineCounter++) {
						lineMiddles[lineCounter][i] = (positions[j][i] + positions[k][i]) * 0.5;
						lineVectors[lineCounter][i] = (positions[j][i] - positions[k][i]);
					}
					areaMiddles[j][i] = 0.0;
					for (int k = 0; k < 4; k++)
						if (k != j)
							areaMiddles[j][i] += positions[k][i];
					areaMiddles[j][i] /= 3;
				}
				tetraMiddle[i] *= 0.25;
			}

			// now compute the areas for each pair of nodes:
			for (int j = 0, counter = 5; j < 4; j++)
				for (int k = j + 1; k < 4; k++, counter--) {
					// double[] crossProduct = crossProduct(subtract(
					// lineMiddles[counter], tetraMiddle), subtract(
					// areaMiddles[j], areaMiddles[k]));
					// double crossSection = norm(crossProduct);
					// double cos = dot(normalize(crossProduct),
					// normalize(lineVectors[counter]));
					// double crossSection = ;
					changeCrossSection(counter, Math.abs(dot(crossProduct(
							subtract(lineMiddles[counter], tetraMiddle),
							subtract(areaMiddles[j], areaMiddles[k])),
							lineVectors[counter])
							/ norm(lineVectors[counter])));
				}
		}
	}

	/**
	 * Used during initialization to make sure that edges are created only once.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	private void registerEdges() throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (!isInfinite()) {
			// int probCounter = 0;
			// for (int i = 0; i < 4; i++) {
			// if (adjacentNodes[i].getPosition()[0] == 60.0 &&
			// adjacentNodes[i].getPosition()[1] == 0.0)
			// probCounter++;
			// }
			// if (probCounter > 2)
			// System.out.println("registerEdges");
			// find Edges that already exist by asking the neighboring
			// tetrahedra:
			Tetrahedron<T> tetrahedron = null;
			for (int i = 0; i < 4; i++) {
				tetrahedron = getAdjacentTriangle(i).getOppositeTetrahedron(this);
				if ((tetrahedron != null) && (!tetrahedron.isInfinite())) {
					int n1 = tetrahedron.getNodeNumber(getAdjacentNode((i + 1) % 4)), 
					    n2 = tetrahedron.getNodeNumber(getAdjacentNode((i + 2) % 4)), 
					    n3 = tetrahedron.getNodeNumber(getAdjacentNode((i + 3) % 4));
					switch (i) {
					case 0:
						adjacentEdges[3] = tetrahedron.getEdge(n1, n2).getReference();
						adjacentEdges[4] = tetrahedron.getEdge(n1, n3).getReference();
						adjacentEdges[5] = tetrahedron.getEdge(n2, n3).getReference();
						break;
					case 1:
						adjacentEdges[1] = tetrahedron.getEdge(n1, n3).getReference();
						adjacentEdges[2] = tetrahedron.getEdge(n2, n3).getReference();
						if (adjacentEdges[5] == null)
							adjacentEdges[5] = tetrahedron.getEdge(n1, n2).getReference();
						break;
					case 2:
						adjacentEdges[0] = tetrahedron.getEdge(n2, n3).getReference();
						if (adjacentEdges[2] == null)
							adjacentEdges[2] = tetrahedron.getEdge(n1, n2).getReference();
						if (adjacentEdges[4] == null)
							adjacentEdges[4] = tetrahedron.getEdge(n1, n3).getReference();
						break;
					case 3:
						if (adjacentEdges[0] == null)
							adjacentEdges[0] = tetrahedron.getEdge(n1, n2).getReference();
						if (adjacentEdges[1] == null)
							adjacentEdges[1] = tetrahedron.getEdge(n1, n3).getReference();
						if (adjacentEdges[3] == null)
							adjacentEdges[3] = tetrahedron.getEdge(n2, n3).getReference();
					}
				}
			}
			SpaceNode<T>[] nodeCopies = getAdjacentNodes();

			// fill up the ones that are missing:
			if (nodeCopies[0] != null) {
				if (adjacentEdges[0] == null)
					adjacentEdges[0] = nodeCopies[0]
							.searchEdge(nodeCopies[1], getSOM()).getReference();
				if (adjacentEdges[1] == null)
					adjacentEdges[1] = nodeCopies[0]
							.searchEdge(nodeCopies[2], getSOM()).getReference();
				if (adjacentEdges[2] == null)
					adjacentEdges[2] = nodeCopies[0]
							.searchEdge(nodeCopies[3], getSOM()).getReference();
			}
			if (adjacentEdges[3] == null)
				adjacentEdges[3] = nodeCopies[1]
						.searchEdge(nodeCopies[2], getSOM()).getReference();
			if (adjacentEdges[4] == null)
				adjacentEdges[4] = nodeCopies[1]
						.searchEdge(nodeCopies[3], getSOM()).getReference();
			if (adjacentEdges[5] == null)
				adjacentEdges[5] = nodeCopies[2]
						.searchEdge(nodeCopies[3], getSOM()).getReference();
			// tell the edges that you are adjacent to it and remember the
			// position where you are inserted:
			for (int i = 0; i < 6; i++)
				if (adjacentEdges[i] != null)
					tracker.organizeEdge(adjacentEdges[i]).addAdjacentTetrahedron(this);
			// if (adjacentEdges[i] != null) positionsInEdgeLists[i] =
			// adjacentEdges[i].adjacentTetrahedra.add(this);
		}

	}

	/**
	 * Changes the volume associated with this tetrahedron to a new value. The
	 * incident nodes are informed about the volume change.
	 * 
	 * @param newVolume
	 *            The new volume.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	private void changeVolume(double newVolume) throws NodeLockedException, ManagedObjectDoesNotExistException {
		double changePerNode = (newVolume - this.volume) / 4.0;
		if (changePerNode != 0.0) {
			for (SpaceNode<T> node : getAdjacentNodes())
				node.changeVolume(changePerNode);
		}
		this.volume = newVolume;
	}

	/**
	 * Calculates the volume of this tetrahedron and changes the stored value.
	 * (The volume equals 1/6th of the determinant of 3 incident edges with a
	 * common endpoint.)
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected void calculateVolume() throws NodeLockedException, ManagedObjectDoesNotExistException {
		changeVolume(Math.abs(det(getPlaneNormals()) / 6.0));
	}

	/**
	 * Calculates the vectors connecting the first endpoint of this tetrahedron
	 * with the other three points.
	 * 
	 * @return A two-dimensional array of length 3x3 and type double, containing
	 *         three vectors.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	private double[][] getPlaneNormals() throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (!isInfinite()) {
			SpaceNode<T> adjacentNode0 = getAdjacentNode(0);
			return new double[][] {
					subtract(getAdjacentNode(1).getPosition(), adjacentNode0
							.getPosition()),
					subtract(getAdjacentNode(2).getPosition(), adjacentNode0
							.getPosition()),
					subtract(getAdjacentNode(3).getPosition(), adjacentNode0
							.getPosition()) };
		}
		else
			return null;
	}

	// private double[][] getNormalizedPlaneNormals() {
	// if (!isInfinite())
	// return new double[][] {
	// normalize(subtract(adjacentNodes[1].getPosition(),
	// adjacentNodes[0].getPosition())),
	// normalize(subtract(adjacentNodes[2].getPosition(),
	// adjacentNodes[0].getPosition())),
	// normalize(subtract(adjacentNodes[3].getPosition(),
	// adjacentNodes[0].getPosition())) };
	// else
	// return null;
	// }

	// private void outFieldElement(Rational a) {
	// double doubleValue = ((Rational)a).doubleValue();
	// double difference = ((Rational)a.subtract(new
	// Rational(doubleValue))).doubleValue();
	// NewDelaunayTest.out(doubleValue+" + "+difference);
	// }

	/**
	 * Determines wether a given point lies inside or outside the circumsphere
	 * of this tetrahedron or lies on the surface of this sphere. This function
	 * uses precise arithmetics to calculate a reliable answer.
	 * 
	 * @param position
	 *            The position for which the orientation should be determined.
	 * @return -1, if the point lies outside this tetrahedron's circumsphere, 1
	 *         if it is inside the sphere and 0, if it lies on the surface of
	 *         the sphere.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected int orientationExact(double[] position) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (isInfinite())
			return 1;
		else {
			ExactVector[] points = new ExactVector[4];
			// NewDelaunayTest.out("Points:");
			for (int i = 0; i < points.length; i++) {
				points[i] = new ExactVector(this.getAdjacentNode(i).getPosition());
				// for (int j = 1; j < 4; j++) {
				// outFieldElement(points[i].getEntry(j));
				// }
			}
			ExactVector[] normals = new ExactVector[] {
					points[1].subtract(points[0]),
					points[2].subtract(points[0]),
					points[3].subtract(points[0]) };

			Rational det = ExactVector.det(normals);

			Rational half = new Rational(1, 2);
			Rational[] offsets = new Rational[] {
					points[0].add(points[1]).dotProduct(normals[0]).multiplyBy(
							half),
					points[0].add(points[2]).dotProduct(normals[1]).multiplyBy(
							half),
					points[0].add(points[3]).dotProduct(normals[2]).multiplyBy(
							half) };

			ExactVector circumCenter = Triangle3D.calculate3PlaneXPoint(
					normals, offsets, det);
			ExactVector dummy = circumCenter.subtract(points[0]);
			Rational squaredRadius = dummy.dotProduct(dummy);
			dummy = circumCenter.subtract(new ExactVector(position));
			Rational distance = dummy.dotProduct(dummy);
			return (int) Math.signum(squaredRadius.compareTo(distance));
		}
	}

	/**
	 * Finds the maximal absolute value in a two-dimensional array. Used for
	 * tolerance interval calculations.
	 * 
	 * @param values
	 *            The array which should be searched.
	 * @return The entry in <code>values</code> with the highest absolute
	 *         value.
	 */
	private double maxAbs(double[][] values) {
		double ret = 0.0;
		for (int i = 0; i < values.length; i++)
			for (int j = 0; j < values[i].length; j++)
				if (Math.abs(values[i][j]) > ret)
					ret = Math.abs(values[i][j]);
		return ret;
	}

	/**
	 * Finds the maximal absolute value in four arrays. Used for tolerance
	 * interval calculations.
	 * 
	 * @param values1
	 *            The first array to be searched.
	 * @param values2
	 *            The second array to be searched.
	 * @param values3
	 *            The third array to be searched.
	 * @param values4
	 *            The fourth array to be searched.
	 * @return The entry with the highest absolute value in any of the 4 given
	 *         arrays.
	 */
	private double maxAbs(double[] values1, double[] values2, double[] values3,
			double[] values4) {
		double ret = 0.0;
		for (int i = 0; i < values1.length; i++) {
			if (Math.abs(values1[i]) > ret)
				ret = Math.abs(values1[i]);
			if (Math.abs(values2[i]) > ret)
				ret = Math.abs(values2[i]);
			if (Math.abs(values3[i]) > ret)
				ret = Math.abs(values3[i]);
			if (Math.abs(values4[i]) > ret)
				ret = Math.abs(values4[i]);
		}
		return ret;
	}

	private double multError2(double a, double aErr2, double b, double bErr2) {
		return Math.max(aErr2 * b * b + bErr2 * a * a, 0.00000000001 * a * b
				* a * b);
	}

	private double multError2(double a, double aErr2, double b, double bErr2,
			double c, double cErr2) {
		return Math.max(aErr2 * b * b * c * c + bErr2 * a * a * c * c + cErr2
				* a * a * b * b, 0.00000000001 * a * a * b * b * c * c);
	}

	private double addError2(double aErr2, double bErr2, double result) {
		return Math.max(bErr2 + aErr2, 0.00000000001 * result * result);
	}

	private double addError2(double aErr2, double bErr2, double cErr2,
			double result) {
		return Math.max(bErr2 + aErr2 + cErr2, 0.00000000001 * result * result);
	}

	private double divError2(double a, double aErr2, double b, double bErr2) {
		return aErr2 / (b * b) + a * a * bErr2 / (b * b * b * b);
	}

	private double dotError(double[] a, double[] aErr2, double[] b,
			double[] bErr2, double result) {
		return addError2(multError2(a[0], aErr2[0], b[0], bErr2[0]),
				multError2(a[1], aErr2[1], b[1], bErr2[1]), multError2(a[2],
						aErr2[2], b[2], bErr2[2]), result);
	}

	private double[] crossError(double[] a, double[] aErr2, double[] b,
			double[] bErr2, double[] result) {
		return new double[] {
				addError2(multError2(a[1], aErr2[1], b[2], bErr2[2]),
						multError2(a[2], aErr2[2], b[1], bErr2[1]), result[0]),
				addError2(multError2(a[0], aErr2[0], b[2], bErr2[2]),
						multError2(a[2], aErr2[2], b[0], bErr2[0]), result[1]),
				addError2(multError2(a[0], aErr2[0], b[1], bErr2[1]),
						multError2(a[1], aErr2[1], b[0], bErr2[0]), result[2]) };
	}

	private double detError2(double[][] vectors, double[][] errors,
			double result) {
		double addError1 = addError2(multError2(vectors[0][0], errors[0][0],
				vectors[1][1], errors[1][1], vectors[2][2], errors[2][2]),
				multError2(vectors[0][0], errors[0][0], vectors[1][2],
						errors[1][2], vectors[2][1], errors[2][1]), multError2(
						vectors[0][1], errors[0][1], vectors[1][0],
						errors[1][0], vectors[2][2], errors[2][2]), 0.0);
		double addError2 = addError2(multError2(vectors[0][1], errors[0][1],
				vectors[1][2], errors[1][2], vectors[2][0], errors[2][0]),
				multError2(vectors[0][2], errors[0][2], vectors[1][0],
						errors[1][0], vectors[2][1], errors[2][1]), multError2(
						vectors[0][2], errors[0][2], vectors[1][1],
						errors[1][1], vectors[2][0], errors[2][0]), 0.0);
		return addError2(addError1, addError2, result);
	}

	private double[] scalarMultError2(double a, double aErr2, double[] b,
			double[] bErr2) {
		return new double[] { multError2(a, aErr2, b[0], bErr2[0]),
				multError2(a, aErr2, b[1], bErr2[1]),
				multError2(a, aErr2, b[2], bErr2[2]), };
	}

	/**
	 * Calculates the center of the circumsphere of this tetrahedron and the
	 * volume. (The latter is simply convenient because the determinant needed
	 * to calculate the volume is used anyways.)
	 * 
	 * Along with the circumsphere calculation, an upper bound of the
	 * uncertainity of this value is calculated.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	private void computeCircumCenterAndVolume() throws NodeLockedException, ManagedObjectDoesNotExistException {
		double[][] normals = getPlaneNormals();
		changeVolume(Math.abs(det(normals)) / 6.0);

		// double[][] normalErrors = new double[3][3];
		// for (int i = 0; i < normalErrors.length; i++)
		// for (int j = 0; j < normalErrors[i].length; j++)
		// normalErrors[i][j] = 0.00000000001 * normals[i][j];

		double nm = maxAbs(normals);
		double maxLength2 = 0.0;
		// normalize normal-vectors
		for (int i = 0; i < 3; i++) {
			double length = normals[i][0] * normals[i][0] + normals[i][1]
					* normals[i][1] + normals[i][2] * normals[i][2];
			// double lengthError2 = dotError(normals[i], normalErrors[i],
			// normals[i], normalErrors[i], length);
			if (length > maxLength2)
				maxLength2 = length;
			length = Math.sqrt(length);

			// lengthError2 /= length;
			// normalErrors[i][0] = divError2(normals[i][0], normalErrors[i][0],
			// length, lengthError2);
			// normalErrors[i][1] = divError2(normals[i][1], normalErrors[i][1],
			// length, lengthError2);
			// normalErrors[i][2] = divError2(normals[i][2], normalErrors[i][2],
			// length, lengthError2);
			normals[i][0] /= length;
			normals[i][1] /= length;
			normals[i][2] /= length;
		}
		// double my2 = 0.0000000001;
		double my2 = 0.000000000000001;
		// maximaler quadrierter Fehler der neuen Normalen-Werte / my2
		double dns2 = Math.max(1, nm * nm
				* (1 / maxLength2 + 1 / (maxLength2 * maxLength2)));
		double ddet2 = 36 * dns2;

		double[] node0Position = getAdjacentNode(0).getPosition();
		double[] node1Position = getAdjacentNode(1).getPosition();
		double[] node2Position = getAdjacentNode(2).getPosition();
		double[] node3Position = getAdjacentNode(3).getPosition();
		double pm2 = maxAbs(node0Position, node1Position, node2Position, node3Position);
		pm2 *= pm2;
		// Offset-Fehler / My2
		double doff2 = 6 * pm2 * (dns2 + 1);
		double dscalar2 = 4 * doff2 + 36 * pm2 * dns2;
		double ddiv2 = 0.0;
		double det = det(normals);

		// double detError2 = detError2(normals, normalErrors, det);
		double[] add01 = add(node0Position, node1Position);
		double[] add02 = add(node0Position, node2Position);
		double[] add03 = add(node0Position, node3Position);
		double[] offsets = new double[] { 0.5 * dot(normals[0], add01),
				0.5 * dot(normals[1], add02), 0.5 * dot(normals[2], add03) };
		// double[] offsetErrors = new double[] {
		// 0.5*
		// dotError(normals[0],normalErrors[0],add01,scalarMult(0.00000000001,add01),
		// offsets[0]),
		// 0.5*
		// dotError(normals[1],normalErrors[1],add02,scalarMult(0.00000000001,add02),
		// offsets[1]),
		// 0.5*
		// dotError(normals[2],normalErrors[2],add03,scalarMult(0.00000000001,add03),
		// offsets[2])
		// };

		double[] cross12 = crossProduct(normals[1], normals[2]);
		double[] cross20 = crossProduct(normals[2], normals[0]);
		double[] cross01 = crossProduct(normals[0], normals[1]);
		// double[]
		// crossError0 = crossError(normals[1], normalErrors[1], normals[2],
		// normalErrors[2], cross12),
		// crossError1 = crossError(normals[2], normalErrors[2], normals[0],
		// normalErrors[0], cross20),
		// crossError2 = crossError(normals[0], normalErrors[0], normals[1],
		// normalErrors[1], cross01);
		// crossError0 = scalarMultError2(offsets[0], offsetErrors[0], cross12,
		// crossError0);
		// crossError1 = scalarMultError2(offsets[1], offsetErrors[1], cross20,
		// crossError1);
		// crossError2 = scalarMultError2(offsets[2], offsetErrors[2], cross01,
		// crossError2);
		// double[] centerError = new double[3];
		double[] add = add(scalarMult(offsets[0], cross12), scalarMult(
				offsets[1], cross20), scalarMult(offsets[2], cross01));
		circumCenter = Triangle3D.calculate3PlaneXPoint(normals, offsets, det);
		// if (det != 0) {
		// tolerance = 0.0;
		// for (int j = 0; j < centerError.length; j++)
		// tolerance += divError2(add[j], addError2(crossError0[j],
		// crossError1[j], crossError2[j], add[j]), det, detError2);
		// double[] dummy = subtract(circumCenter,
		// adjacentNodes[0].getPosition());
		// squaredRadius = dot(dummy,dummy);
		// tolerance = 2 * Math.sqrt(squaredRadius * tolerance);
		// }

		if (det != 0) {
			ddiv2 = 1 / (det * det) * 3 * dscalar2 + 324 * pm2 * ddet2
					/ (det * det * det * det);
			double[] dummy = subtract(circumCenter, node0Position);
			squaredRadius = dot(dummy, dummy);
			tolerance = Math.sqrt(12 * ddiv2 * squaredRadius) * my2;
		}
		updateCrossSectionAreas();

	}

	/**
	 * Calculates the radius of this tetrahedron's circumsphere.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	private void computeRadius() throws NodeLockedException, ManagedObjectDoesNotExistException {
		double[] dummy = subtract(circumCenter, getAdjacentNode(0).getPosition());
		squaredRadius = dot(dummy, dummy);
		// if (TOLERANCE_SETTING > 0)
		// tolerance = squaredRadius*TOLERANCE_SETTING;
	}

	/**
	 * Calculates the properties of this tetrahedron's circumsphere.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public void calculateCircumSphere() throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (!isInfinite()) {
			circumCenter = null;
			// for (int i = 0; (i < 4) && (circumCenter == null); i++)
			// circumCenter =
			// adjacentTriangles[i].calculateCircumSphereCenterIfEasy(adjacentNodes[i].getPosition());
			if (circumCenter == null) {
				computeCircumCenterAndVolume();
			} else {
				calculateVolume();
			}
			computeRadius();
		}
	}

	/**
	 * Used to calculate the properties of this tetrahedron's circumsphere after
	 * an endpoint has been moved. Originally used to increase the speed of
	 * circumsphere calculations, but now uses the same functions as
	 * {@link #calculateCircumSphere()} because the old method increased the
	 * uncertainity of the circumcenter. 
	 * 
	 * In addition to calcualting the circumsphere, all incident triangles that are
	 * incident to the moved node are informed about the movement.
	 * 
	 * @param movedNode
	 *            The node that was moved.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	void updateCirumSphereAfterNodeMovement(SpaceNode<T> movedNode) throws NodeLockedException, ManagedObjectDoesNotExistException {
		int nodeNumber = getNodeNumber(movedNode);
		if (!isInfinite()) {
			// circumCenter =
			// adjacentTriangles[nodeNumber].calculateCircumSphereCenterIfEasy(movedNode.getPosition());
			circumCenter = null;

			if (circumCenter == null)
				computeCircumCenterAndVolume();
			else
				calculateVolume();
			computeRadius();
		}
		for (int i = 0; i < 4; i++) {
			if (i != nodeNumber)
				getAdjacentTriangle(i).informAboutNodeMovement();
		}
	}

	/**
	 * Determines wether a given point lies inside or outside the circumsphere
	 * of this tetrahedron or lies on the surface of this sphere.
	 * 
	 * @param point
	 *            The point for which the orientation should be determined.
	 * @return -1, if the point lies outside this tetrahedron's circumsphere, 1
	 *         if it is inside the sphere and 0, if it lies on the surface of
	 *         the sphere.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public int orientation(double[] point) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (!isInfinite()) {
			double[] dummy = subtract(circumCenter, point);
			double dum = dot(dummy, dummy);
			// System.out.print(".");
			if (dum > squaredRadius + tolerance)
				return -1;
			else if (dum < squaredRadius - tolerance)
				return 1;
			else {
				// System.out.print("-");
				toleranceUsages++;
				int result = orientationExact(point);
				if ((result != 0) && ((result == 1) ^ (dum < squaredRadius))) {
					double difference = Math.abs(squaredRadius - dum);
					difference /= tolerance;
					if (difference > maxToleranceNeeded)
						maxToleranceNeeded = difference;
					NewDelaunayTest.changeOutputCreation();
					orientationExact(point);
					NewDelaunayTest.changeOutputCreation();
					this.calculateCircumSphere();
				}
				return result;
			}
		} else {
			Tetrahedron innerTetrahedron = getAdjacentTetrahedron(0);
			getAdjacentTriangle(0).updatePlaneEquationIfNecessary();
			int orientation;
			if (innerTetrahedron != null) {
				if (innerTetrahedron.isInfinite())
					return 1;
				orientation = getAdjacentTriangle(0).orientation(point,
						innerTetrahedron.getOppositeNode(getAdjacentTriangle(0))
								.getPosition());
			} else
				orientation = getAdjacentTriangle(0)
						.orientationToUpperSide(point);
			if (orientation == 0) {
				return getAdjacentTriangle(0).circleOrientation(point);
			} else
				return -orientation;
		}
	}

	/**
	 * Determines wether a given point lies truly inside the circumsphere of
	 * this tetrahedron
	 * 
	 * @param point
	 *            The point for which the orientation should be determined.
	 * @return <code>true</code> if the distance of the point to the center of
	 *         the circumsphere is smaller than the radius of the circumsphere
	 *         and <code>false</code> otherwise.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public boolean isTrulyInsideSphere(double[] point) throws NodeLockedException, ManagedObjectDoesNotExistException {
		return orientation(point) > 0;
	}

	/**
	 * Determines wether a given point lies truly inside the circumsphere of
	 * this tetrahedron
	 * 
	 * @param point
	 *            The point for which the orientation should be determined.
	 * @return <code>true</code> if the distance of the point to the center of
	 *         the circumsphere is smaller or equal to the radius of the
	 *         circumsphere and <code>false</code> otherwise.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public boolean isInsideSphere(double[] point) throws NodeLockedException, ManagedObjectDoesNotExistException {
		return orientation(point) >= 0;
	}

	/**
	 * Returns a String representation of this tetrahedron, consisting of the
	 * identification numbers of the endpoints.
	 * 
	 * @return A string in the format "(x1, x2, x3, x4)", where the xi denote
	 *         the ID's of the endpoints of this tetrahedron.
	 */
	public String toString() {
		return "(" + adjacentNodes[0] + "," + adjacentNodes[1] + ","
				+ adjacentNodes[2] + "," + adjacentNodes[3] + ")";
	}

	/**
	 * Removes this tetrahedron from the triangulation. All the incident nodes,
	 * edges and triangles are informed that this tetrahedron is being removed.
	 * 
	 * !IMPORTANT!: No triangle organizer is informed about the removement of
	 * this tetrahedron. A caller of this function must keep track of the new
	 * open triangles itself!
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected void remove() throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (NewDelaunayTest.createOutput())
			NewDelaunayTest.out("Removing tetrahedron " + this);

		invalidate();
		for (int i = 0; i < 4; i++) {
			SpaceNode<T> adjacentNode = getAdjacentNode(i);
			if (adjacentNode != null) {
				adjacentNode.changeVolume(-this.volume / 4.0);
				adjacentNode.removeTetrahedron(this);
				// positionsInNodeLists[i].remove();
			}
			Tetrahedron opposite = getAdjacentTetrahedron(i);
			if (opposite != null && !isInfinite() && opposite.isInfinite())
				getAdjacentTriangle(i).orientToSide(adjacentNode.getPosition());
			getAdjacentTriangle(i).removeTetrahedron(this);
		}
		for (int i = 0; i < 6; i++) {
			Edge<T> adjacentEdge = getAdjacentEdge(i);
			if (adjacentEdge != null) {
				adjacentEdge.changeCrossSectionArea(-crossSectionAreas[i]);
				adjacentEdge.removeTetrahedron(this);
			}
		}
		if (allTetrahedra != null)
			allTetrahedra.remove(this);

	}

	/**
	 * Replaces one of the incident triangles of this tetrahedron. Automatically
	 * exchanges the affected edges, too.
	 * 
	 * @param oldTriangle
	 *            The triangle that should be replaced.
	 * @param newTriangle
	 *            The new trianlge.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected void replaceTriangle(Triangle3D<T> oldTriangle,
			Triangle3D<T> newTriangle) throws NodeLockedException, ManagedObjectDoesNotExistException {
		newTriangle.addTetrahedron(this);
		Tetrahedron<T> otherTetrahedron = newTriangle.getOppositeTetrahedron(this);
		// if (otherTetrahedron == null)
		// if (NewDelaunayTest.createOutput())
		// NewDelaunayTest.out("replaceTriangle");
		int triangleNumber = getTriangleNumber(oldTriangle);
		for (int i = 0, position = (triangleNumber + 2) % 4, lastPosition = (triangleNumber + 1) % 4; i < 3; i++) {
			int edgeNumber = getEdgeNumber(lastPosition, position);
			Edge<T> otherEdge = otherTetrahedron.getEdge(
					getAdjacentNode(lastPosition), getAdjacentNode(position));
			if (otherEdge != getAdjacentEdge(edgeNumber)) {
				getAdjacentEdge(edgeNumber).removeTetrahedron(this);
				// positionsInEdgeLists[edgeNumber] =
				// otherEdge.getAdjacentTetrahedra().add(this);
				otherEdge.getAdjacentTetrahedra().add(this);
				adjacentEdges[edgeNumber] = (otherEdge==null)?null:otherEdge.getReference();
			}

			lastPosition = position;
			position = (position + 1) % 4;
			if (position == triangleNumber)
				position = (position + 1) % 4;
		}
		this.adjacentTriangles[triangleNumber] = newTriangle.getReference();
		newTriangle.wasCheckedAlready(-1);
	}

	/**
	 * Determines which index a given node has in this tetrahedron's list of
	 * endpoints.
	 * 
	 * @param node
	 *            The node of interest.
	 * @return An index between 0 and 3.
	 */
	protected int getNodeNumber(SpaceNode<T> node)  {
		if (node == null) {
			for (int i = 0; i < 4; i++) {
				if (adjacentNodes[i] == null)
					return i;
			}
		}
		else {
			ManagedObjectReference<T> ref = node.getReference();
			for (int i = 0; i < 4; i++) {
				if (ref.equals(adjacentNodes[i]))
					return i;
			}
		}
		throw new UnknownManagedObjectException("The node " + node + " is not adjacent to "
				+ this + "!");
	}

	/**
	 * Determines which index a given triangle has in this tetrahedron's list of
	 * incident triangles.
	 * 
	 * @param triangle
	 *            The triangle of interest.
	 * @return An index between 0 and 3.
	 */
	protected int getTriangleNumber(Triangle3D triangle) {
		if (triangle == null) {
			for (int i = 0; i < 4; i++) {
				if (adjacentTriangles[i] == null)
					return i;
			}
		}
		else {
			ManagedObjectReference<T> ref = triangle.getReference();
			for (int i = 0; i < 4; i++) {
				if (ref.equals(adjacentTriangles[i]))
					return i;
			}
		}
		throw new UnknownManagedObjectException("The triangle " + triangle
				+ " is not adjacent to " + this + "!");
	}

	/**
	 * Determines the index of the edge connecting two given endpoints of the
	 * tetrahedron in this tetrahedron's list of incident edges.
	 * 
	 * @param nodeNumber1
	 *            The index of the first endpoint of the edge.
	 * @param nodeNumber2
	 *            The index of the second endpoint of the edge.
	 * @return A number between 0 and 5, giving the index of the edge of
	 *         interest.
	 */
	protected static int getEdgeNumber(int nodeNumber1, int nodeNumber2) {
		return nodeNumber1 + nodeNumber2
				- ((nodeNumber1 == 0) ? 1 : ((nodeNumber2 == 0) ? 1 : 0));
	}

	/**
	 * Determines the edge that connects two endpoints of this tetrahedron.
	 * 
	 * @param nodeNumber1
	 *            The index of the first endpoint of the edge.
	 * @param nodeNumber2
	 *            The index of the second endpoint of the edge.
	 * @return The edge connecting the two endpoints with the given indices.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected Edge<T> getEdge(int nodeNumber1, int nodeNumber2) throws NodeLockedException, ManagedObjectDoesNotExistException {
		return getAdjacentEdge(getEdgeNumber(nodeNumber1, nodeNumber2));
	}

	/**
	 * Determines the edge that connects two endpoints of this tetrahedron.
	 * 
	 * @param a
	 *            The first endpoint of the edge.
	 * @param b
	 *            The second endpoint of the edge.
	 * @return A number between 0 and 5, giving the index of the edge of
	 *         interest.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected int getEdgeNumber(SpaceNode<T> a, SpaceNode<T> b) throws NodeLockedException, ManagedObjectDoesNotExistException {
		return getEdgeNumber(getNodeNumber(a), getNodeNumber(b));
	}

	/**
	 * Determines the edge that connects two endpoints of this tetrahedron.
	 * 
	 * @param a
	 *            The first endpoint of the edge.
	 * @param b
	 *            The second endpoint of the edge.
	 * @return The edge connecting the two given endpoints.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected Edge<T> getEdge(SpaceNode<T> a, SpaceNode<T> b) throws NodeLockedException, ManagedObjectDoesNotExistException {
		return getAdjacentEdge(getEdgeNumber(a, b));
	}

	/**
	 * Returns the incident triangle opposite to a given endpoint of this
	 * tetrahedron.
	 * 
	 * @param node
	 *            An endpoint of this tetrahedron.
	 * @return A reference to the triangle that lies opposite to
	 *         <code>node</code>.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected Triangle3D<T> getOppositeTriangle(SpaceNode<T> node) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (node == null) {
			for (int i = 0; i < 4; i++) {
				if (adjacentNodes[i] == null)
					return getAdjacentTriangle(i);
			}
		}
		else {
			ManagedObjectReference<T> ref = node.getReference();
			for (int i = 0; i < 4; i++) {
				if (ref.equals(adjacentNodes[i]))
					return getAdjacentTriangle(i);
			}
		}
		throw new UnknownManagedObjectException("The SpaceNode " + node
				+ " is not adjacent to the Tetrahedron " + this);
	}

	/**
	 * Returns the incident node opposite to a given triangle which is incident
	 * to this tetrahedron.
	 * 
	 * @param triangle
	 *            An incident triangle of this tetrahedron.
	 * @return The endpoint of this triangle that lies opposite to
	 *         <code>triangle</code>.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected SpaceNode<T> getOppositeNode(Triangle3D<T> triangle) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (triangle ==null) {
			for (int i = 0; i < 4; i++) {
				if (adjacentTriangles[i] == null)
					return getAdjacentNode(i);
			}
		}
		else {
			ManagedObjectReference<T> ref = triangle.getReference();
			for (int i = 0; i < 4; i++) {
				if (ref.equals(adjacentTriangles[i]))
					return getAdjacentNode(i);
			}
		}
		throw new UnknownManagedObjectException("The Triangle " + triangle
				+ " is not adjacent to the Tetrahedron " + this);
	}

	/**
	 * Returns a reference to the triangle connecting this tetrahedron with
	 * another one.
	 * 
	 * @param tetrahedron
	 *            An adjacent tetrahedron.
	 * @return The triangle which is incident to this tetrahedron and
	 *         <code>tetrahedron</code>.
	 */
	protected Triangle3D<T> getConnectingTriangle(Tetrahedron<T> tetrahedron) throws NodeLockedException, ManagedObjectDoesNotExistException {
		for (int i = 0; i < 4; i++) {
			if (getAdjacentTriangle(i).isAdjacentTo(tetrahedron))
				return getAdjacentTriangle(i);
		}
		throw new UnknownManagedObjectException("The Tetrahedron " + tetrahedron
				+ " is not adjacent to " + this + "!");
	}

	/**
	 * Returns this index of the triangle connecting this tetrahedron with
	 * another one.
	 * 
	 * @param tetrahedron
	 *            An adjacent tetrahedron.
	 * @return An index between 0 and 3 which is the position of the triangle
	 *         incident to this tetrahedron and <code>tetrahedron</code> in
	 *         this tetrahedron's list of incident triangles.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected int getConnectingTriangleNumber(Tetrahedron<T> tetrahedron) throws NodeLockedException, ManagedObjectDoesNotExistException {
		for (int i = 0; i < 4; i++) {
			if (getAdjacentTriangle(i).isAdjacentTo(tetrahedron))
				return i;
		}
		throw new UnknownManagedObjectException("The Tetrahedron " + tetrahedron
				+ " is not adjacent to " + this + "!");
	}

	/**
	 * Returns the three incident triangles that are adjacent to a given
	 * triangle.
	 * 
	 * @param base
	 *            A triangle which is incident to this tetrahedron.
	 * @return An array of three triangles.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected Triangle3D<T>[] getTouchingTriangles(Triangle3D<T> base) throws NodeLockedException, ManagedObjectDoesNotExistException {
		Triangle3D<T>[] ret = new Triangle3D[3];
		SpaceNode<T>[] triangleNodes = base.getNodes();
		for (int i = 0; i < 3; i++)
			ret[i] = getOppositeTriangle(triangleNodes[i]);
		return ret;
	}

	/**
	 * Given two nodes incident to this tetrahedron, this function returns
	 * another endpoint. The returned endpoint is different from the result of
	 * {@link #getSecondOtherNode(SpaceNode, SpaceNode)}.
	 * 
	 * @param nodeA
	 *            A first incident node.
	 * @param nodeB
	 *            A second incident node.
	 * @return A third incident node.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	private SpaceNode<T> getFirstOtherNode(SpaceNode<T> nodeA, SpaceNode<T> nodeB) throws NodeLockedException, ManagedObjectDoesNotExistException {
		ManagedObjectReference<T> refA = (nodeA==null)?null:nodeA.getReference();
		ManagedObjectReference<T> refB = (nodeB==null)?null:nodeB.getReference();
		for (int i = 0; i < 4; i++) {
			if (adjacentNodes[i] == null) {
				if ((refA != null) && (refB != null))
					return null;
			}
			else {
				if ((!adjacentNodes[i].equals(refA)) && (!adjacentNodes[i].equals(refB)))
					return getAdjacentNode(i);
				
			}
		}
		return null;
	}

	/**
	 * Given two nodes incident to this tetrahedron, this function returns
	 * another endpoint. The returned endpoint is different from the result of
	 * {@link #getFirstOtherNode(SpaceNode, SpaceNode)}.
	 * 
	 * @param nodeA
	 *            A first incident node.
	 * @param nodeB
	 *            A second incident node.
	 * @return A third incident node.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	private SpaceNode<T> getSecondOtherNode(SpaceNode<T> nodeA, SpaceNode<T> nodeB) throws NodeLockedException, ManagedObjectDoesNotExistException {
		ManagedObjectReference<T> refA = (nodeA==null)?null:nodeA.getReference();
		ManagedObjectReference<T> refB = (nodeB==null)?null:nodeB.getReference();
		for (int i = 3; i >= 0; i--) {
			if (adjacentNodes[i] == null) {
				if ((refA != null) && (refB != null))
					return null;
			}
			else {
				if ((!adjacentNodes[i].equals(refA)) && (!adjacentNodes[i].equals(refB)))
					return getAdjacentNode(i);
				
			}
		}
		return null;
	}

	/**
	 * Determines whether a given coordinate lies in convex position, meaning
	 * that the incident triangle with list index
	 * <code>connectingTriangleNumver</code> is truly cut by a line connecting
	 * the given coordinate and the endpoint of this tetrahedron that lies
	 * opposite to the same triangle.
	 * 
	 * @param point
	 *            The coordinate that should be tested.
	 * @param connectingTriangleNumber
	 *            The index of the triangle facing the coordinate.
	 * @return <code>true</code>, if the given coordinate truly lies in
	 *         convex position and <code>false</code> otherwise.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected boolean isPointInConvexPosition(double[] point,
			int connectingTriangleNumber) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (!isInfinite()) {
			for (int i = 0; i < 4; i++) {
				if (i != connectingTriangleNumber) {
					Triangle3D<T> adjacentTriangle = getAdjacentTriangle(i);
					adjacentTriangle.updatePlaneEquationIfNecessary();
					if (!adjacentTriangle.trulyOnSameSide(getAdjacentNode(i)
							.getPosition(), point))
						return false;
				}

			}
			return true;
		} else {
			// Triangle3D convexHullTriangle = getAdjacentTriangles()[0];
			// SpaceNode oppositeNode =
			// convexHullTriangle.getOppositeTetrahedron(this).getOppositeNode(convexHullTriangle);
			// boolean ret = (oppositeNode ==
			// null)||!convexHullTriangle.onSameSide(point,
			// oppositeNode.getPosition());
			// if (ret) {
			// for (int i = 1; i < 4; i++) {
			// if (i != connectingTriangleNumber) {
			// double[] projectedPosition
			// }
			// }
			// if (NewDelaunayTest.createOutput())
			// NewDelaunayTest.out("isPointInConvexPosition");
			// }
			// return ret;
			return false;
		}
	}

	/**
	 * Determines whether a given coordinate lies in convex position, meaning
	 * that the incident triangle with list index
	 * <code>connectingTriangleNumver</code> is cut by a line connecting the
	 * given coordinate and the endpoint of this tetrahedron that lies opposite
	 * to the same triangle.
	 * 
	 * @param point
	 *            The coordinate that should be tested.
	 * @param connectingTriangleNumber
	 *            The index of the triangle facing the coordinate.
	 * @return 1, if the given coordinate lies truly in convex position to this
	 *         tetrahedron (meaning that a line connecting the node opposite to
	 *         the specified triangle and the given coordinate would cut the
	 *         inside of the specified triangle), 0 if the point lies on the
	 *         border between convex positions and non-convex position, and -1
	 *         if the point lies in a non-convex position.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected int isInConvexPosition(double[] point,
			int connectingTriangleNumber) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (!isInfinite()) {
			int result = 1;
			for (int i = 0; i < 4; i++) {
				if (i != connectingTriangleNumber) {
					Triangle3D<T> adjacentTriangle = getAdjacentTriangle(i);
					adjacentTriangle.updatePlaneEquationIfNecessary();
					int currentResult = adjacentTriangle.orientation(
							getAdjacentNode(i).getPosition(), point);
					if (currentResult < 0)
						return -1;
					else
						result *= currentResult;
					// if ()
					// if
					// (!adjacentTriangles[i].trulyOnSameSide(adjacentNodes[i].getPosition(),
					// point))
					// return false;
				}

			}
			return result;
		} else {
			// int orientation = this.orientation(point);
			// if (orientation >= 0) {
			// Triangle3D convexHullTriangle = getAdjacentTriangles()[0];
			// double[] connection = subtract(point,
			// getAdjacentNodes()[connectingTriangleNumber].getPosition());
			// double[] normalVector = adjacentTriangles[0].getNormalVector();
			// int[] refs = new int[] {(connectingTriangleNumber == 1)?2:1,
			// (connectingTriangleNumber == 3)?2:3 };
			// double[][] connectionsToCenter = new double[2][];
			// for (int i = 0; i < connectionsToCenter.length; i++)
			// connectionsToCenter[i] =
			// subtract(adjacentNodes[refs[i]].getPosition(),
			// adjacentNodes[connectingTriangleNumber].getPosition());
			// for (int i = 0; i < 2; i++) {
			// double[] orthogonal = crossProduct(connectionsToCenter[i],
			// normalVector);
			// int angleReferenceSign = (int)Math.signum(dot(orthogonal,
			// connectionsToCenter[i^1]));
			// double angleTest = dot(orthogonal, connection);
			// if (Math.abs(angleTest) <
			// norm(orthogonal)*norm(connection)*0.000000001) {
			// // exact mathematics...
			// Vector center = new
			// Vector(adjacentNodes[connectingTriangleNumber].getPosition());
			// Vector normalX = adjacentTriangles[0].getExactNormalVector();
			// Vector[] connectionsToCenterX = new Vector[] {
			// (new
			// Vector(adjacentNodes[refs[0]].getPosition())).subtract(center),
			// (new
			// Vector(adjacentNodes[refs[1]].getPosition())).subtract(center),
			// };
			// Vector orthogonalX = connectionsToCenterX[i].cross(normalX);
			// FieldElement angleTestX = orthogonalX.multiply(connectionX);
			// Rational reference = new Rational(angleReferenceSign);
			// }
			// else return ((int)(Math.signum(angleTest)))*angleReferenceSign;
			// }
			// if (NewDelaunayTest.createOutput())
			// NewDelaunayTest.out("isPointInConvexPosition");
			//				
			// }
			return -1;
		}
	}

	/**
	 * Removes two flat tetrahedra that have two common triangles.
	 * 
	 * @param <T>
	 *            The type of the user objects stored in the given tetrahedra.
	 * @param tetrahedronA
	 *            The first flat tetrahedron.
	 * @param tetrahedronB
	 *            The second flat tetrahedron.
	 * @return A list of tetrahedra that were originally adjacent to either one
	 *         of the two flat tetrahedra that were removed.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected static <T> Tetrahedron<T>[] remove2FlatTetrahedra(
			Tetrahedron<T> tetrahedronA, Tetrahedron<T> tetrahedronB) throws NodeLockedException, ManagedObjectDoesNotExistException {
		// int connectingTriangleNumber =
		// tetrahedronA.getConnectingTriangleNumber(tetrahedronB);
		// if
		// (!tetrahedronB.isAdjacentTo(tetrahedronA.getAdjacentNodes()[connectingTriangleNumber]))
		// return flip2to3(tetrahedronA, tetrahedronB);
		System.out.println("**************** Removing 2 flat tetrahedra! *************************");
		Triangle3D<T>[] triangleListA = tetrahedronA.getAdjacentTriangles(), 
			triangleListB = tetrahedronB.getAdjacentTriangles();
		LinkedList<Tetrahedron<T>> adjacentTetrahedra = new LinkedList<Tetrahedron<T>>();
		int[] outerTrianglesA = new int[] { -1, -1, -1 }, outerTrianglesB = new int[] {
				-1, -1, -1 };
		// int outerTriangle1A = -1, outerTriangle1B = 0, outerTriangle2A = -1,
		// outerTriangle2B = 0;

		// find matching triangles:
		int outerTriangleCount = 0;
		for (int i = 0; (i < 4); i++) {
			boolean possible = true;
			for (int j = 0; (j < 4) && (possible); j++)
				possible = triangleListA[i] != triangleListB[j];
			if (possible) {
				outerTrianglesA[outerTriangleCount] = i;
				for (int j = 0; j < 4; j++) {
					if (triangleListA[i].isSimilarTo(triangleListB[j]))
						outerTrianglesB[outerTriangleCount] = j;
				}
				outerTriangleCount++;
			}
		}
		// remove the tetrahedra:
		tetrahedronA.remove();
		tetrahedronB.remove();
		
		// make sure that the tetrahedra which became neighbors point to the same triangles:
		for (int i = 0; i < outerTriangleCount; i++) {

			Tetrahedron<T> a = triangleListA[outerTrianglesA[i]]
					.getOppositeTetrahedron(null);
			if (!adjacentTetrahedra.contains(a))
				adjacentTetrahedra.add(a);
			Tetrahedron<T> b = triangleListB[outerTrianglesB[i]]
					.getOppositeTetrahedron(null);
			if (!adjacentTetrahedra.contains(b))
				adjacentTetrahedra.add(b);
			a.replaceTriangle(triangleListA[outerTrianglesA[i]],
					triangleListB[outerTrianglesB[i]]);
		}
		// adjacentTetrahedra.add(triangleListA[outerTriangle1A].getOppositeTetrahedron(null));
		// adjacentTetrahedra.add(triangleListB[outerTriangle1B].getOppositeTetrahedron(null));

		// Tetrahedron a2 =
		// triangleListA[outerTriangle2A].getOppositeTetrahedron(null);
		// Tetrahedron b2 =
		// triangleListB[outerTriangle2B].getOppositeTetrahedron(null);
		// if (!adjacentTetrahedra.contains(a2))
		// adjacentTetrahedra.add(a2);
		// if (!adjacentTetrahedra.contains(b2))
		// adjacentTetrahedra.add(b2);
		// for (int i = 0; i < outerTriangleCount; i++)
		// tetrahedronA.replaceTriangle(outerTrianglesA[i],
		// triangleListB[outerTrianglesB[i]]);
		return adjacentTetrahedra.toArray(new Tetrahedron[adjacentTetrahedra
				.size()]);
	}

	/**
	 * Performs a 2->3 Flip of two adjacent tetrahedra.
	 * @param <T> The type of the user objects stored in the endpoints of the two tetrahedra.
	 * @param tetrahedronA The first tetrahedron to flip.
	 * @param tetrahedronB The second tetrahedron to flip.
	 * @return An array of tetrahedra which were created during the process of flipping.  
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	/**
	 * @param <T>
	 * @param tetrahedronA
	 * @param tetrahedronB
	 * @return
	 * @throws NodeLockedException
	 * @throws ManagedObjectDoesNotExistException
	 */
	protected static <T> Tetrahedron<T>[] flip2to3(Tetrahedron<T> tetrahedronA,
			Tetrahedron<T> tetrahedronB) throws NodeLockedException, ManagedObjectDoesNotExistException {
		// if (tetrahedronA.isFlat() && tetrahedronB.isFlat())
		// return remove2FlatTetrahedra(tetrahedronA, tetrahedronB);
		SpatialOrganizationManager<T> chosenSOM = tetrahedronA.getSOM();

		int connectingTriangleNumber = tetrahedronA
				.getConnectingTriangleNumber(tetrahedronB);
		Triangle3D<T> connectingTriangle = tetrahedronA.getAdjacentTriangle(connectingTriangleNumber);
		SpaceNode<T> lowerNode = tetrahedronB.getOppositeNode(connectingTriangle);
		if (lowerNode == null) {
			if (NewDelaunayTest.createOutput())
				NewDelaunayTest.out("flip2To3");
		}
		int convexPosition = (lowerNode == null) ? 1 : tetrahedronA
				.isInConvexPosition(lowerNode.getPosition(),
						connectingTriangleNumber);
		if (convexPosition >= 0) {
			boolean checkForFlatTetrahedra = convexPosition == 0;
			Triangle3D<T>[] upperTriangles = tetrahedronA
					.getTouchingTriangles(connectingTriangle);
			Triangle3D<T>[] lowerTriangles = tetrahedronB
					.getTouchingTriangles(connectingTriangle);
			SpaceNode<T> upperNode = tetrahedronA.getAdjacentNode(connectingTriangleNumber);
			Triangle3D<T>[] newTriangles = new Triangle3D[3];
			SpaceNode<T>[] connectingTriangleNodes = connectingTriangle.getNodes();
			CacheManager<T> cm = tetrahedronA.getLockingCacheManger();
			for (int i = 0; i < 3; i++) 
				newTriangles[i] = new Triangle3D<T>(upperNode, lowerNode,
						connectingTriangleNodes[i], null, null, chosenSOM, cm);
			tetrahedronA.remove();
			tetrahedronB.remove();
			Tetrahedron<T>[] ret = new Tetrahedron[3];
			for (int i = 0; i < 3; i++) {
				// make sure a node at position 0 is always inserted at position
				// 0, if it is part of the connecting triangle:
				int a = (i + 1) % 3;
				int b = (i + 2) % 3;
				if (b == 0) {
					b = 2;
					a = 0;
				}
				if ((checkForFlatTetrahedra)
						&& (upperTriangles[i].orientation(lowerNode
								.getPosition(), lowerNode.getPosition()) == 0)) {
					ret[i] = new FlatTetrahedron<T>(newTriangles[b],
							upperTriangles[i], lowerTriangles[i],
							newTriangles[a], connectingTriangleNodes[a],
							lowerNode, upperNode, connectingTriangleNodes[b], cm);
				} else
					ret[i] = new Tetrahedron<T>(newTriangles[b],
							upperTriangles[i], lowerTriangles[i],
							newTriangles[a], connectingTriangleNodes[a],
							lowerNode, upperNode, connectingTriangleNodes[b], cm);
			}
			return ret;
		} else if (tetrahedronA.isInfinite()) {
			if (NewDelaunayTest.createOutput())
				NewDelaunayTest.out("flip2to3");
			return null;
		} else
			return null;
	}

	/**
	 * Performs a 3->2 Flip of two adjacent tetrahedra.
	 * @param <T> The type of the user objects stored in the endpoints of the two tetrahedra.
	 * @param tetrahedronA The first tetrahedron to flip.
	 * @param tetrahedronB The second tetrahedron to flip.
	 * @param tetrahedronC The third tetrahedron to flip.
	 * @return An array of tetrahedra which were created during the process of flipping.  
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected static <T> Tetrahedron<T>[] flip3to2(Tetrahedron<T> tetrahedronA,
			Tetrahedron<T> tetrahedronB, Tetrahedron<T> tetrahedronC) throws NodeLockedException, ManagedObjectDoesNotExistException {

		SpatialOrganizationManager<T> chosenSOM = tetrahedronA.getSOM();
		if (tetrahedronB.getSOM().equals(tetrahedronC.getSOM()))
			chosenSOM = tetrahedronB.getSOM();
		
		SpaceNode<T>[] newTriangleNodes = new SpaceNode[3];
		newTriangleNodes[0] = tetrahedronA.getAdjacentNode(tetrahedronA
				.getConnectingTriangleNumber(tetrahedronB));
		newTriangleNodes[1] = tetrahedronB.getAdjacentNode(tetrahedronB
				.getConnectingTriangleNumber(tetrahedronC));
		newTriangleNodes[2] = tetrahedronC.getAdjacentNode(tetrahedronC
				.getConnectingTriangleNumber(tetrahedronA));

		SpaceNode<T> upperNode = tetrahedronA.getFirstOtherNode(
				newTriangleNodes[0], newTriangleNodes[1]);
		SpaceNode<T> lowerNode = tetrahedronA.getSecondOtherNode(
				newTriangleNodes[0], newTriangleNodes[1]);
		CacheManager<T> cm = tetrahedronA.getLockingCacheManger();

		Triangle3D<T> newTriangle = new Triangle3D<T>(newTriangleNodes[0],
				newTriangleNodes[1], newTriangleNodes[2], null, null,chosenSOM, cm);

		Tetrahedron<T>[] ret = new Tetrahedron[2];

		Triangle3D<T> tetraAOppTriangleLow = tetrahedronA
				.getOppositeTriangle(lowerNode);
		Triangle3D<T> tetraBOppTriangleLow = tetrahedronB
				.getOppositeTriangle(lowerNode);
		Triangle3D<T> tetraCOppTriangleLow = tetrahedronC
				.getOppositeTriangle(lowerNode);
		Triangle3D<T> tetraAOppTriangleUp = tetrahedronA
				.getOppositeTriangle(upperNode);
		Triangle3D<T> tetraBOppTriangleUp = tetrahedronB
				.getOppositeTriangle(upperNode);
		Triangle3D<T> tetraCOppTriangleUp = tetrahedronC
				.getOppositeTriangle(upperNode);

		boolean flat = tetrahedronA.isFlat() && tetrahedronB.isFlat()
				&& tetrahedronC.isFlat();
		tetrahedronA.remove();
		tetrahedronB.remove();
		tetrahedronC.remove();
		if (!flat) {
			ret[0] = new Tetrahedron<T>(newTriangle, tetraAOppTriangleLow,
					tetraBOppTriangleLow, tetraCOppTriangleLow, upperNode,
					newTriangleNodes[2], newTriangleNodes[0],
					newTriangleNodes[1], cm);
			ret[1] = new Tetrahedron<T>(newTriangle, tetraAOppTriangleUp,
					tetraBOppTriangleUp, tetraCOppTriangleUp, lowerNode,
					newTriangleNodes[2], newTriangleNodes[0],
					newTriangleNodes[1], cm);
		} else {
			ret[0] = new FlatTetrahedron<T>(newTriangle, tetraAOppTriangleLow,
					tetraBOppTriangleLow, tetraCOppTriangleLow, upperNode,
					newTriangleNodes[2], newTriangleNodes[0],
					newTriangleNodes[1], cm);
			ret[1] = new FlatTetrahedron<T>(newTriangle, tetraAOppTriangleUp,
					tetraBOppTriangleUp, tetraCOppTriangleUp, lowerNode,
					newTriangleNodes[2], newTriangleNodes[0],
					newTriangleNodes[1], cm);

		}
		return ret;
	}

	// public Edge[] getTriangleEdges(Triangle3D triangle) {
	// //Edge[] ret = new Edge[3];
	// int triangleNumber = getTriangleNumber(triangle);
	// switch (triangleNumber) {
	// case 0: return new Edge[] {edges[3]};
	// }
	// return null;
	// }

	protected ManagedObjectReference<T>[] getAdjacentNodeReferences() {
		ManagedObjectReference<T>[] ret = new ManagedObjectReference[4];
		for (int i = 0; i < ret.length; i++) {
			if (adjacentNodes[i] == null)
				ret[i] = null;
			else
				ret[i] = adjacentNodes[i].getReference();
		}
		return ret;
	}

	/**
	 * @return An array containing the nodes incident to this tetrahedron.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected SpaceNode<T>[] getAdjacentNodes() throws NodeLockedException, ManagedObjectDoesNotExistException {
		SpaceNode<T>[] ret = new SpaceNode[4];
		for (int i = 0; i < 4; i++) {
			ret[i] = (adjacentNodes[i]==null)?null:tracker.organizeNode(adjacentNodes[i]);
		}
		return ret;
	}

	/**
	 * Returns the second tetrahedron that is incident to the incident triangle with index <code>number</code>. 
	 * @param number An index specifying a position in the list of triangles of this tetrahedron. The 
	 * corresponding triangle will be chosen to determine the adjacent tetrahedron. 
	 * @return An adjacent tetrahedron.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected Tetrahedron<T> getAdjacentTetrahedron(int number) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (adjacentTriangles[number] != null) {
			return tracker.organizeTriangle(adjacentTriangles[number]).getOppositeTetrahedron(this);
		} else
			return null;
	}
	
	/**
	 * Returns one of the edges incident to this tetrahedron.
	 * @param number The index of the requested edge.
	 * @return The incident edge with the given index.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected Edge<T> getAdjacentEdge(int number) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (adjacentEdges[number]== null)
			return null;
		else
			return tracker.organizeEdge(adjacentEdges[number]);
	}

	/**
	 * Returns one of the nodes incident to this tetrahedron.
	 * @param number The index of the requested node.
	 * @return The incident node with the given index.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected SpaceNode<T> getAdjacentNode(int number) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (adjacentNodes[number] == null)
			return null;
		else
			return tracker.organizeNode(adjacentNodes[number]);
	}
	
	/**
	 * Returns one of the triangles incident to this tetrahedron.
	 * @param number The index of the requested triangle.
	 * @return The incident triangle with the given index.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected Triangle3D<T> getAdjacentTriangle(int number) throws NodeLockedException, ManagedObjectDoesNotExistException {
		if (adjacentTriangles[number] == null)
			return null;
		else
			return tracker.organizeTriangle(adjacentTriangles[number]);
	}
	

	/**
	 * @return An array of triangles containing the 4 triangles incident to this tetrahedron.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected Triangle3D<T>[] getAdjacentTriangles() throws NodeLockedException, ManagedObjectDoesNotExistException {
		Triangle3D<T>[] ret = new Triangle3D[4];
		for (int i = 0; i < 4; i++) {
			ret[i] = (adjacentTriangles[i]==null)?null:tracker.organizeTriangle(adjacentTriangles[i]);
		}
		return ret;
	}

	/**
	 * Determines whether a given node is an endpoint of this tetrahedron.
	 * @param node The node of interest.
	 * @return <code>true</code>, if the node is an endpoint.
	 */
	public boolean isAdjacentTo(SpaceNode<T> node) {
		if (node == null)
			return (adjacentNodes[0] == null) || (adjacentNodes[1] == null)
				|| (adjacentNodes[2] == null) || (adjacentNodes[3] == null);
		else {
			ManagedObjectReference<T> ref = node.getReference();
			return (ref.equals(adjacentNodes[0])) || (ref.equals(adjacentNodes[1]))
				|| (ref.equals(adjacentNodes[2])) || (ref.equals(adjacentNodes[3]));
		}
	}

	/**
	 * Walks toward a specified point. If the point lies inside this
	 * tetrahedron, this tetrahedron is returned. Otherwise, an adjacent
	 * tetrahedron is returned that lies closer to the point.
	 * 
	 * @param coordinate
	 *            The coordinate that should be approximated.
	 * @return An adjacent tetrahedron that lies closer to the specified point
	 *         than this tetrahedron, or this tetrahedron, if the point lies inside it.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public Tetrahedron<T> walkToPoint(double[] coordinate)
			throws PositionNotAllowedException, NodeLockedException, ManagedObjectDoesNotExistException {
		if (!isInfinite()) {
			// need to create a copy of triangleOrder to make sure that no one else modifies my order...
			int[] order = new int[4];
			synchronized (triangleOrder) {
				Collections.shuffle(triangleOrder, NewDelaunayTest.rand);
				for (int i = 0; i < 4; i++) {
					order[i] = triangleOrder.get(i);
				}
			}
			for (int i = 0; i < 4; i++) {
				int pos = order[i];
				Triangle3D<T> currentTriangle = getAdjacentTriangle(pos);
				currentTriangle.updatePlaneEquationIfNecessary();
				int orientation = currentTriangle.orientation(
						getAdjacentNode(pos).getPosition(), coordinate);
				if (orientation < 0) {
					Tetrahedron<T> oppositeTetrahedron = currentTriangle.getOppositeTetrahedron(this);
					if (oppositeTetrahedron == null) {
						System.out.println("Oops... walkToPoint will return null!");
						oppositeTetrahedron = currentTriangle.getOppositeTetrahedron(this);
					}
					return oppositeTetrahedron;
				} else if (orientation == 0) {
					Tetrahedron<T> oppositeTetrahedron = currentTriangle
							.getOppositeTetrahedron(this);
					if ((oppositeTetrahedron.isInfinite())
							&& (isTrulyInsideSphere(coordinate))) {
						testPosition(coordinate);
						return oppositeTetrahedron;
					}
				}
			}
		} else {
			if (!isInsideSphere(coordinate))
				return getAdjacentTriangle(0).getOppositeTetrahedron(this);
		}
		testPosition(coordinate);
		return this;
	}

	/**
	 * Checks if a node may be moved to a given coordinate.
	 * @param position The coordinate of interest.
	 * @throws PositionNotAllowedException If the position is equal to any endpoint of this tetrahedron.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected void testPosition(double[] position)
			throws PositionNotAllowedException, NodeLockedException, ManagedObjectDoesNotExistException {
		for (SpaceNode<T> node : getAdjacentNodes()) {
			if (node != null) {
				double[] diff = subtract(position, node.getPosition());
				if ((Math.abs(diff[0]) == 0) && (Math.abs(diff[1]) == 0)
						&& (Math.abs(diff[2]) == 0)) {
					throw new PositionNotAllowedException(node.proposeNewPosition());
				}
			}
		}
	}

	/**
	 * Returns whether a given tetrahedron is adjacent to this tetrahedron.
	 * @param otherTetrahedron The potential neighbor of this tetrahedron.
	 * @return <code>true</code>, iff this tetrahedron is adjacent to <code>otherTetrahedron</code>.  
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public boolean isNeighbor(Tetrahedron<T> otherTetrahedron) throws NodeLockedException, ManagedObjectDoesNotExistException {
		return (getAdjacentTriangle(0).isAdjacentTo(otherTetrahedron))
				|| (getAdjacentTriangle(1).isAdjacentTo(otherTetrahedron))
				|| (getAdjacentTriangle(2).isAdjacentTo(otherTetrahedron))
				|| (getAdjacentTriangle(3).isAdjacentTo(otherTetrahedron));
	}

//	// public int getId() {
//	// return id;
//	// }
//	
//	/**
//	 * @return the unique address of this Managed Object
//	 * @see ini.cx3d.parallelSpatialOrganization.ManagedObject#getAddress()
//	 */
//	public long getAddress() {
//		// TODO Auto-generated method stub
//		return 0;
//	}


	/**
	 * Organizes a local copy of this Tetrahedron. The request is directed to this object's ManagedObjectTracker
	 * which will then either request a copy from the assigned SpatialOrganizationManager or from the assigned 
	 * CacheManager, if this object is a local copy. (In the latter case, however, this function will return <code>null</code>, 
	 * because there may not be two local copies of one object.)
	 * @return a local deep copy of the Managed Object if this object is not a local copy and <code>null</code> else.
	 * @see ManagedObjectTracker#organizeTetrahedron(ManagedObjectReference)
	 * @deprecated It doesn't really make any sense to use this function at all. Either,  a SpatialOrganizationManager will
	 * create a local copy of an object which should be modified itself, or local copies of incident objects are requested.
	 */
	public Tetrahedron<T> getLocalCopy() throws NodeLockedException, ManagedObjectDoesNotExistException {
		return tracker.organizeTetrahedron(tracker.getReference());
    }
	
	
//	/**
//	 * @return the SpatialOrganizationManager that hosts this Managed Object
//	 * @see ini.cx3d.parallelSpatialOrganization.ManagedObject#getSOM()
//	 */
//	public SpatialOrganizationManager getSOM() {
//		return som;
//	}
//	
//	/**
//	 * @return a reference to the Managed Object
//	 */
//	public ManagedObjectReference<T> getReference(){
//		TetrahedronReference<T> ret = new TetrahedronReference(som, address);
//		return ret; 
//	}
//	
//	/**
//	 * @param the current transaction ID
//	 */
//	public void setTransactionID(Integer transactionID){
//		this.transactionID = transactionID;
//		
//	}
}
