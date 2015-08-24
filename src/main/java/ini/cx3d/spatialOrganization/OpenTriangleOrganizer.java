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

import static ini.cx3d.utilities.Matrix.add;
import static ini.cx3d.utilities.Matrix.crossProduct;
import static ini.cx3d.utilities.Matrix.dot;
import static ini.cx3d.utilities.Matrix.normalize;
import static ini.cx3d.utilities.Matrix.scalarMult;
import static ini.cx3d.utilities.Matrix.subtract;

import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Stack;

/**
 * Whenever an incomplete delaunay triangulation is created for some reason (removement of 
 * a node or during an initial triangulation), 'holes' in the triangulation need to be filled.
 * 
 * This class provides the functionality to create complete triangulations from incomplete ones.
 * It keeps track of all triangles in the triangulation that are currently incident to only
 * one tetrahedron (=> 'open triangles') and can fill 'holes' in the triangulation. In order 
 * to do so, it creates new tetrahedra by successively choosing an open triangle and a corresponding
 * node.
 * 
 * @see #triangulate()
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of user objects with the nodes in a given triangulation.
 */
public class OpenTriangleOrganizer<T> {
	
	/**
	 * Counts the number of times exact arithmetics have been used in any instance of this class.
	 * Used for debugging purposes only! DO NOT USE. Needs to be removed. 
	 */
	public static int exactCalculations = 0;

	/**
	 * Counts the number of times that calculations were performed without using exact arithmetics 
	 * in any instance of this class. 
	 * Used for debugging purposes only! DO NOT USE. Needs to be removed. 
	 */
	public static int normalCalculations = 0;
	
	/**
	 * Saves tolerance values that have been used during calculations of any instance of this class. 
	 * Used for debugging purposes only! DO NOT USE. Needs to be removed. 
	 */
	public static double[] toleranceValues = new double[1024];

	/**
	 * A pointer to a position in <code>toleranceValues</code>.
	 * Used for debugging purposes only! DO NOT USE. Needs to be removed. 
	 */
	public static int toleranceValuePointer = 0;

	/**
	 * Adds a new value to the array <code>toleranceValues</code>.
	 * @param tolerance The new value.
	 * Used for debugging purposes only! DO NOT USE. Needs to be removed. 
	 */
	public static void addToleranceValue(double tolerance) {
		toleranceValues[toleranceValuePointer] = tolerance;
		toleranceValuePointer = (toleranceValuePointer+1) & 1023;
	}

	/**
	 * Saves the maximum tolerance that was ever exploited.
	 * Used for debugging purposes only! DO NOT USE. Needs to be removed. 
	 */
	public static double maxToleranceNeeded = 0.0;
	
	/**
	 * A hashmap to store open triangles that are created during any operation on the triangulation.
	 * Used to keep track of which triangles are not open any more.
	 */
	private HashMap<TriangleHashKey<T>, Triangle3D<T>> map;
	
	/**
	 * Stores all tetrahedra that are created by this instance. No tetrahedra will
	 * be recorded if this reference is <code>null</code>, which is the default.
	 * Use {@link #recoredNewTetrahedra()} to start recording. 
	 */
	private LinkedList<Tetrahedron<T>> newTetrahedra = null;
	
	/**
	 * A node organizer used to keep track of all nodes that are adjacent to 
	 * any 'hole' in the triangulation. 
	 * These nodes are potential candidates for combination with open triangles. 
	 */
	private AbstractTriangulationNodeOrganizer<T> tno;
	 
	/**
	 * In addition to <code>map</code>, all open triangles are also stored in this stack.
	 * Thereby, a fast extraction of any open triangle is guaranteed.
	 */
	Stack<Triangle3D<T>> openTriangles = new Stack<Triangle3D<T>>();

	/**
	 * Stores the shortest signed delaunay distance that was found for a given open
	 * triangle.  
	 */
	private double shortestDistance = Double.MAX_VALUE;
	
	/**
	 * A link to a tetrahedron that was created during the completion of an
	 * incomplete triangulation. 
	 */
	Tetrahedron<T> aNewTetrahedron = null;

	/**
	 * Initializes an open triangle organizer with a specified node organizer and
	 * a preferred capacity of open triangles.
	 * @param preferredCapacity The number of open triangles that this organizer will
	 * have to keep track of. The internal hashmap is initialized with double this size.
	 * @param tno The node organizer used to keep track of "open" nodes.
	 */
	public OpenTriangleOrganizer(int preferredCapacity,
			AbstractTriangulationNodeOrganizer<T> tno) {
		map = new HashMap<TriangleHashKey<T>, Triangle3D<T>>(
				preferredCapacity * 2);
		this.tno = tno;
	}
	
	/**
	 * Starts the recording of newly created tetrahedra.
	 */
	public void recoredNewTetrahedra() {
		if (newTetrahedra == null)
			newTetrahedra = new LinkedList<Tetrahedron<T>>();
	}
	
	/**
	 * Returns the list of newly created tetrahedra.
	 * @return The list of newly created tetrahedra, if recording was turned on before
	 * (Use {@link #recoredNewTetrahedra()}) or <code>null</code> else.
	 */
	public LinkedList<Tetrahedron<T>> getNewTetrahedra() {
		return newTetrahedra;
	}
	
//	/**
//	 * @return
//	 */
//	public Collection<Triangle3D<T>> getOpenTriangles() {
//		return map.values();
//	}

	/**
	 * Returns a simple instance of <code>OpenTriangleOrganizer</code>. As a node
	 * organizer, an instance of {@link SimpleTriangulationNodeOrganizer} is used 
	 * and the number of open triangles that will approximately be used is set
	 * to 30.
	 * @param <T> The type of user objects associated with nodes in the triangulation.
	 * @return A new instance of <code>OpenTriangleOrganizer</code>.
	 */
	public static <T> OpenTriangleOrganizer<T> createSimpleOpenTriangleOrganizer() {
		return new OpenTriangleOrganizer<T>(30,
				new SimpleTriangulationNodeOrganizer<T>());
	}

	/**
	 * Informs this open triangle organizer that a new 
	 * open triangle is available. In order to do so, the new open 
	 * triangle is added to the hashmap. 
	 * @param triangle The new open triangle.
	 */
	public void putTriangle(Triangle3D<T> triangle) {
		TriangleHashKey<T> key = new TriangleHashKey<T>(triangle.getNodes()[0],
				triangle.getNodes()[1],
				triangle.getNodes()[2]);
		map.put(key, triangle);
		tno.addTriangleNodes(triangle);
		openTriangles.push(triangle);
	}

	/**
	 * Informs this open triangle organizer that an open triangle 
	 * is no longer available. In order to do so, the new open 
	 * triangle is removed from the hashmap. 
	 * @param triangle The triangle that should be removed.
	 */
	public void removeTriangle(Triangle3D<T> triangle) {
		TriangleHashKey<T> key = new TriangleHashKey<T>(triangle.getNodes()[0],
				triangle.getNodes()[1],
				triangle.getNodes()[2]);
		map.remove(key);
	}

	/**
	 * Searches for a triangle which is incident to three specified nodes.
	 * If such an open triangle is not yet known to this
	 * open triangle organizer, it is created and added to the 
	 * hashmap. Otherwise, it is removed from the hashmap (since it won't be
	 * open any more soon).
	 * @param a The first node incident to the requested triangle.
	 * @param b The second node incident to the requested triangle.
	 * @param c The third node incident to the requested triangle.
	 * @return A triangle with the requested three endpoints.
	 */
	public Triangle3D<T> getTriangle(SpaceNode<T> a, SpaceNode<T> b,
			SpaceNode<T> c) {
		TriangleHashKey<T> key = new TriangleHashKey<T>(a, b, c);
		Triangle3D<T> ret = map.get(key);
		if (ret == null) {
			ret = new Triangle3D<T>(a, b, c, null, null);
			map.put(key, ret);
			openTriangles.push(ret);
		} else if (ret.isCompletelyOpen()) {
			openTriangles.push(ret);
		} else {
			map.remove(key);
			// ret.addTetrahedron(newTe);
		}
		return ret;
	}

	/**
	 * Searches for a triangle which is incident to three specified nodes.
	 * If this triangle does not exist yet, it is created and added to the 
	 * hashmap. In contrary to {@link #getTriangle(SpaceNode, SpaceNode, SpaceNode)},
	 * the triangle is not removed from the hashmap if it was already part of the 
	 * hashmap.
	 * @param a The first node incident to the requested triangle.
	 * @param b The second node incident to the requested triangle.
	 * @param c The third node incident to the requested triangle.
	 * @return A triangle with the requested three endpoints.
	 */
	public Triangle3D<T> getTriangleWithoutRemoving(
			SpaceNode<T> a, SpaceNode<T> b, SpaceNode<T> c) {
		TriangleHashKey<T> key = new TriangleHashKey<T>(a, b, c);
		Triangle3D<T> ret = map.get(key);
		if (ret == null) {
			ret = new Triangle3D<T>(a, b, c, null, null);
			openTriangles.push(ret);
			map.put(key, ret);
		}
		return ret;
	}

	/**
	 * Determines if a triangle with the three specified endpoints is stored in 
	 * the hashmap for open triangles already.
	 * @param a The first node incident to the searched triangle.
	 * @param b The second node incident to the searched triangle.
	 * @param c The third node incident to the searched triangle.
	 * @return <code>true</code>, if an open triangle with the desired endpoints
	 * is already stored in this triangle organizer.
	 */
	protected boolean contains(SpaceNode<T> a, SpaceNode<T> b,
			SpaceNode<T> c) {
		TriangleHashKey<T> key = new TriangleHashKey<T>(a, b, c);
		return map.containsKey(key);
	}

	/**
	 * Returns whether this triangle organizer is storing any more open triangles.
	 * @return <code>true</code>, if the hashmap storing empty triangles is empty.
	 */
	protected boolean isEmpty() {
		return map.isEmpty();
		// return openTriangles == 0;
	}

	/**
	 * Returns an open triangle if there is any left. This function uses the stack {@link #openTriangles}
	 * to search for open triangles. 
	 * @return An open triangle if there is still an open triangle left 
	 * and <code>null</code> else. 
	 */
	private Triangle3D<T> getAnOpenTriangle() {
		if (openTriangles.isEmpty())
			return null;
		Triangle3D<T> ret = openTriangles.pop();
//		if ((ret.getNodes()[0] != null)
//				&& (ret.getNodes()[0].getId() == 45)
//				&& (ret.getNodes()[1].getId() == 36)
//				&& (ret.getNodes()[2].getId() == 50))
//			if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("Stop");
		while ((ret.isInfinite() || ret.isClosed()
				|| ret.isCompletelyOpen() 
				)) {//|| ret.getOppositeTetrahedron(null).isInfinite())) {
//			if ((ret.getNodes()[0] != null)
//					&& (ret.getNodes()[0].getId() == 45)
//					&& (ret.getNodes()[1].getId() == 36)
//					&& (ret.getNodes()[2].getId() == 50))
//				if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("Stop");
			if (openTriangles.isEmpty())
				return null;
			ret = openTriangles.pop();
		}

		return ret;
		// if (map.isEmpty())
		// return null;
		// if ((anOpenTriangle != null) && (!anOpenTriangle.isClosed()) &&
		// (!anOpenTriangle.isCompletelyOpen()))
		// return this.anOpenTriangle;
		// else return map.values().iterator().next();

	}

	/**
	 * Returns an arbitrary tetrahedron which was created during the process of
	 * triangulation.
	 * @return A tetrahedron which was created during triangulation.
	 */
	protected Tetrahedron<T> getANewTetrahedron() {
		return aNewTetrahedron;
	}

	/**
	 * Stores an edge onto a hashmap for edges. This function implements the
	 * same functinoality as {@link #putTriangle(Triangle3D)} but for edges. 
	 * @param a The first endpoint of the edge that should be placed on the hashmap.
	 * @param a The second endpoint of the edge that should be placed on the hashmap.
	 * @param oppositeNode A node lying on the non-open side of this edge.
	 * @param oldOpenEdge If no pre-existing edge was found, this edge is returned.
	 * @param map The hashmap on which the specified edge should be placed.
	 * @return An instance of <code>EdgeHashKey</code> which points to the specified
	 * two endpoints.
	 */
	private EdgeHashKey<T> putEdgeOnMap(SpaceNode<T> a,
			SpaceNode<T> b, SpaceNode<T> oppositeNode,
			EdgeHashKey<T> oldOpenEdge,
			HashMap<EdgeHashKey<T>, EdgeHashKey<T>> map) {
		EdgeHashKey<T> hk1 = new EdgeHashKey<T>(a, b,
				oppositeNode);
		if (map.containsKey(hk1)) {
			map.remove(hk1);
			return oldOpenEdge;
		} else {
			map.put(hk1, hk1);
			return hk1;
		}
	}

	/**
	 * Finds the node with lowest id in a list of nodes.
	 * @param nodes The list of nodes.
	 * @return The node with lowest id.
	 */
	private SpaceNode<T> findCenterNode(
			LinkedList<SpaceNode<T>> nodes) {
		SpaceNode<T> centerNode = null;
		int minID = Integer.MAX_VALUE;
		for (SpaceNode<T> node : nodes) {
			if (node.getId() < minID) {
				minID = node.getId();
				centerNode = node;
			}
		}
		return centerNode;
	}

	/**
	 * Creates a two-dimensional triangulation for a set of points that all lie on one plane and on the 
	 * border of one circle.
	 * @param sortedNodes A list of nodes, which is expected to be sorted by occurrence on the circle. 
	 * @param centerNode The node with the lowest id.
	 * @param map A hashmap for instances of <code>EdgeHashKey</code>. Used to keep track of 'open edges'. 
	 * @param triangleList A list of triangles to which all the triangles created within this function will be
	 * added.
	 * @return An open edge on the convex hull of the triangulated circle.
	 */
	private EdgeHashKey<T> triangulateSortedCirclePoints(
			LinkedList<SpaceNode<T>> sortedNodes,
			SpaceNode<T> centerNode,
			HashMap<EdgeHashKey<T>, EdgeHashKey<T>> map,
			LinkedList<Triangle3D<T>> triangleList) {
		Iterator<SpaceNode<T>> it = sortedNodes.iterator();
		SpaceNode<T> last = it.next();
		SpaceNode<T> current = it.next();
		// EdgeHashKey currentEdge = putEdgeOnMap(last, b, oppositeNode,
		// oldOpenEdge, map)
		EdgeHashKey<T> ret = null;
		while (it.hasNext()) {
			last = current;
			current = it.next();
			triangleList.add(getTriangleWithoutRemoving(
					last, current, centerNode));
			putEdgeOnMap(centerNode, last, current, null,
					map);
			ret = putEdgeOnMap(last, current, centerNode,
					ret, map);
			putEdgeOnMap(current, centerNode, last, null,
					map);
		}
		return ret;
	}

//	private Triangle3D<T> findForbiddenTriangle(
//			LinkedList<SpaceNode<T>> sortedNodes,
//			SpaceNode<T> centerNode) {
//		if (sortedNodes.size() == 4) {
//			Iterator<SpaceNode<T>> it1 = sortedNodes.iterator();
//			it1.next();
//			SpaceNode<T> a = it1.next(), b = it1.next(), c = it1.next();
//			if (contains(a,b,c))
//				return this.getTriangleWithoutRemoving(a, b, c);
//		}
//		else {
//			Iterator<SpaceNode<T>> it1 = sortedNodes.iterator();
//			it1.next();
//			for (int i = 1; i < sortedNodes.size() - 2; i++) {
//				SpaceNode<T> a = it1.next();
//				Iterator<SpaceNode<T>> it2 = sortedNodes.iterator();
//				for (int dummy = 0; dummy <= i; dummy++) 
//					it2.next();
//				for (int j = i+1; j < sortedNodes.size() - 1; j++) {
//					SpaceNode<T> b = it2.next();
//					Iterator<SpaceNode<T>> it3 = sortedNodes.iterator();
//					for (int dummy = 0; dummy <= j; dummy++) 
//						it3.next();
//					for (int k = j+1; k < sortedNodes.size(); k++) {
//						SpaceNode<T> c = it3.next();
//						if (contains(a,b,c))
//							return this.getTriangleWithoutRemoving(a, b, c);
//					}
//				}
//			}
//		}
//		return null;
//	}

	/**
	 * Given an order of nodes that all lie on the surface of one circle, this function searches for triangles in the triangulation that 
	 * do not match the standardized triangulation of this circle. In a standardized triangulation, every triangle is incident to two points 
	 * which are successors on the circle and to the one point with lowest ID.
	 * 
	 * Triangles that violate the standardized triangulation are removed together with incident tetrahedra.
	 * @param sortedNodes A list of nodes that lie on one circle. This list is expected to be sorted in terms of angular 
	 * neighborhoodship of nodes.
	 */
	private void removeForbiddenTriangles(LinkedList<SpaceNode<T>> sortedNodes) {
		// Special treatment for situation with 4 nodes only:
		if (sortedNodes.size() == 4) {
			SpaceNode<T> center = sortedNodes.get(0), a = sortedNodes.get(1), b = sortedNodes.get(2), c = sortedNodes.get(3);
			// in case there is only one valid triangle, remove the others:
			if (contains(center, a, b)) {
				if (!contains(center, b, c))
					removeAllTetrahedraInSphere(getTriangleWithoutRemoving(center, a, b).getOppositeTetrahedron(null));
			}
			else if (contains(center, b, c))
				removeAllTetrahedraInSphere(getTriangleWithoutRemoving(center, b, c).getOppositeTetrahedron(null));
			// otherwise, check if there are triangles that are not allowed (and remove them if necessary
			else {
				if (contains(a,b,c))
					removeAllTetrahedraInSphere(getTriangleWithoutRemoving(a, b, c).getOppositeTetrahedron(null));
				if (contains(center, a, c))
					removeAllTetrahedraInSphere(getTriangleWithoutRemoving(center, a, c).getOppositeTetrahedron(null));
			}
		}
		// general case:
		else {
			boolean removeAllCircleTriangles = false;
			// first, copy nodes to array for faster access:
			SpaceNode<T>[] nodes = new SpaceNode[sortedNodes.size()];
			sortedNodes.toArray(nodes);
			// check if any valid triangle is missing, if yes, set removeAllCircleTriangles = true
			for (int i = 1; (i < nodes.length-1) && (!removeAllCircleTriangles); i++) 
				if (!contains(nodes[0], nodes[i], nodes[i+1])) 
					removeAllCircleTriangles = true;
			// if any valid triangle was missing...
			if (removeAllCircleTriangles)
				// ...test if any triangle imaginable exists...
				for (int i = 0; i < nodes.length-2; i++) 
					for (int j = i+1; j < nodes.length-1; j++) 
						for (int k = j+1; k < nodes.length; k++) 
							if (contains(nodes[i],nodes[j], nodes[k]))
								// and remove it together with incident tetrahedra:
								removeAllTetrahedraInSphere(getTriangleWithoutRemoving(nodes[i], nodes[j], nodes[k]).getOppositeTetrahedron(null));
		}
	}

	/**
	 * Sorts a list of nodes that all lie on one circle. 
	 * @param nodes The list of nodes that should be sorted.
	 * @param startingEdge Any pre-existing edge.
	 * @param centerNode The node with lowest ID.
	 * @return A sorted list of nodes. The first node in this list will be the node with lowest ID, successive nodes are sorted
	 * according to their occurrence on the circle.
	 */
	private LinkedList<SpaceNode<T>> sortCircleNodes(
			LinkedList<SpaceNode<T>> nodes,
			EdgeHashKey<T> startingEdge, SpaceNode<T> centerNode) {
		LinkedList<SpaceNode<T>> sortedNodes = new LinkedList<SpaceNode<T>>();
		SpaceNode<T> searchNode = null;
		SpaceNode<T> lastSearchNode;
		SpaceNode<T> removedNode1 = null, removedNode2 = null;
		if (startingEdge == null) {
			lastSearchNode = nodes.removeFirst();
			double minDistance = Double.MAX_VALUE;
			for (SpaceNode<T> node : nodes) {
				double[] dummy = subtract(lastSearchNode
						.getPosition(), node.getPosition());
				double dot = dot(dummy, dummy);
				if (dot < minDistance) {
					searchNode = node;
					minDistance = dot;
				}
			}
			nodes.remove(searchNode);
			removedNode1 = lastSearchNode;
			removedNode2 = searchNode;
		} else {
			searchNode = startingEdge.b;
			lastSearchNode = startingEdge.a;
		}
		while (!nodes.isEmpty()) {
			if (searchNode == null
					|| lastSearchNode == null) {
				if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("sortCircleNodes");
			}
			double[] lastVector = normalize(subtract(
					searchNode.getPosition(),
					lastSearchNode.getPosition()));
			double biggestCosinus = -2.0;
			SpaceNode<T> pickedNode = null;
			for (SpaceNode<T> node : nodes) {
				double[] dummy = normalize(subtract(node
						.getPosition(), searchNode
						.getPosition()));
				double currentCosinus = dot(dummy,
						lastVector);
				if (currentCosinus > biggestCosinus) {
					biggestCosinus = currentCosinus;
					pickedNode = node;
				}
			}
			sortedNodes.addLast(pickedNode);
			lastSearchNode = searchNode;
			searchNode = pickedNode;
			nodes.remove(pickedNode);
		}
		if (startingEdge != null) {
			sortedNodes.addFirst(startingEdge.b);
			sortedNodes.addFirst(startingEdge.a);
		} else {
			sortedNodes.addFirst(removedNode2);
			sortedNodes.addFirst(removedNode1);
		}
		while (!sortedNodes.isEmpty()
				&& (sortedNodes.getFirst() != centerNode))
			nodes.addLast(sortedNodes.removeFirst());
		sortedNodes.addAll(nodes);
		return sortedNodes;
	}

	/**
	 * Removes one tetrahedron from the triangulation and possibly all adjacent tetrahedra that have the same circumsphere as the
	 * first tetrahedron.
	 * @param startingTetrahedron The first tetrahedron to remove.
	 */
	protected void removeAllTetrahedraInSphere(
			Tetrahedron<T> startingTetrahedron) {
		if (startingTetrahedron.isValid()) {
			Tetrahedron<T>[] tetrahedronsToRemove = new Tetrahedron[4];
			for (int i = 0; i < 4; i++) {
				Triangle3D<T> triangle = startingTetrahedron
						.getAdjacentTriangles()[i];
				Tetrahedron<T> oppositeTetrahedron = triangle
						.getOppositeTetrahedron(startingTetrahedron);
				if ((oppositeTetrahedron != null)
						&& (!startingTetrahedron.isInfinite()^ oppositeTetrahedron.isInfinite())
						&& (startingTetrahedron
								.isInsideSphere(oppositeTetrahedron
										.getOppositeNode(
												triangle)
										.getPosition())))
					tetrahedronsToRemove[i] = oppositeTetrahedron;
				if (triangle.isClosed())
					this.putTriangle(triangle);
				else
					this.removeTriangle(triangle);
			}
			startingTetrahedron.remove();
			for (int i = 0; i < 4; i++) {
				if (tetrahedronsToRemove[i] != null)
					removeAllTetrahedraInSphere(tetrahedronsToRemove[i]);
			}
		}
	}

	/**
	 * Creates a two-dimensional triangulation for a set of 4 or more points that lie in the same plane and on the border of the same circle.
	 * @param similarDistanceNodes A list of nodes that have a similar 2D-signed delaunay distance to the  <code>startingEdge</code>. 
	 * @param startingEdge An initial edge which is part of the circle.
	 * @param map A hashmap for instances of type <code>EdgeHashKey</code> which is used to keep track of open edges in 
	 * {@link #triangulatePointsOnSphere(LinkedList, LinkedList, Triangle3D)}.
	 * @param triangleList A list of triangles, which is used to store all triangles created in this function and
	 * pass them to the calling function.
	 * @return An open edge on the convex hull of the triangulated circle.
	 */
	private EdgeHashKey<T> triangulatePointsOnCircle(
			LinkedList<SpaceNode<T>> similarDistanceNodes,
			EdgeHashKey<T> startingEdge,
			HashMap<EdgeHashKey<T>, EdgeHashKey<T>> map,
			LinkedList<Triangle3D<T>> triangleList) {
		if (startingEdge != null) {
			similarDistanceNodes.addFirst(startingEdge.a);
			similarDistanceNodes.addFirst(startingEdge.b);
		}
		if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("triangulating points on Circle: "
						+ similarDistanceNodes);
		SpaceNode<T> centerNode = findCenterNode(similarDistanceNodes);
		if (startingEdge != null) {
			similarDistanceNodes.removeFirst();
			similarDistanceNodes.removeFirst();
		}
		LinkedList<SpaceNode<T>> sortedNodes = sortCircleNodes(
				similarDistanceNodes, startingEdge,
				centerNode);
		removeForbiddenTriangles(sortedNodes);
		return triangulateSortedCirclePoints(sortedNodes,
				centerNode, map, triangleList);
	}

	/**
	 * Creates a tetrahedralization for a set of 5 or more points that lie on the surface of one sphere. 
	 * @param nodes A list of nodes that have the same signed delaunay distance to <code>startingTriangle</code>.
	 * @param onCircleNodes A list of nodes that lie on one common circle with <code>startingTriangle</code>.
	 * @param startingTriangle An initial triangle which will be part of the triangulation.
	 */
	private void triangulatePointsOnSphere(
			LinkedList<SpaceNode<T>> nodes,
			LinkedList<SpaceNode<T>> onCircleNodes,
			Triangle3D<T> startingTriangle) {
		LinkedList<Triangle3D<T>> surfaceTriangles = new LinkedList<Triangle3D<T>>();
		nodes.add(startingTriangle.getNodes()[0]);
		nodes.add(startingTriangle.getNodes()[1]);
		nodes.add(startingTriangle.getNodes()[2]);
		nodes.addAll(onCircleNodes);
		if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("triangulating points on sphere: "
						+ nodes);
		HashMap<EdgeHashKey<T>, EdgeHashKey<T>> map = new HashMap<EdgeHashKey<T>, EdgeHashKey<T>>(
				30);
		EdgeHashKey anOpenEdge = null;
		if (onCircleNodes.isEmpty()) {
			surfaceTriangles.add(startingTriangle);
			for (int i = 0; i < 3; i++)
				anOpenEdge = putEdgeOnMap(
						startingTriangle.getNodes()[i],
						startingTriangle.getNodes()[(i + 1) % 3],
						startingTriangle.getNodes()[(i + 2) % 3],
						anOpenEdge, map);
		} else {
			if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("Special case: starting triangle "
							+ startingTriangle
							+ " lies on same circle as "
							+ onCircleNodes.toString());
			onCircleNodes
					.add(startingTriangle.getNodes()[0]);
			onCircleNodes
					.add(startingTriangle.getNodes()[1]);
			onCircleNodes
					.add(startingTriangle.getNodes()[2]);
			anOpenEdge = triangulatePointsOnCircle(
					onCircleNodes, null, map,
					surfaceTriangles);
		}
		LinkedList<SpaceNode<T>> similarDistanceNodes = new LinkedList<SpaceNode<T>>();
		double upperBound, lowerBound;
		while (!map.isEmpty()) {
			SpaceNode<T> a = anOpenEdge.a, b = anOpenEdge.b;
			double smallestCosinus = Double.MAX_VALUE;
			upperBound = smallestCosinus;
			lowerBound = smallestCosinus;
			SpaceNode<T> pickedNode = null;
			double tolerance = 0.000000001;
			for (SpaceNode<T> currentNode : nodes) {
				if ((currentNode != anOpenEdge.a)
						&& (currentNode != anOpenEdge.b)) {
					if (currentNode == null)
						if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("");
					double cosinus = anOpenEdge
							.getCosine(currentNode
									.getPosition());
					if (cosinus < upperBound) {
						if (cosinus > lowerBound) {
							similarDistanceNodes
									.add(currentNode);
						} else {
							pickedNode = currentNode;
							smallestCosinus = cosinus;
							upperBound = smallestCosinus
									+ tolerance;
							lowerBound = smallestCosinus
									- tolerance;
							similarDistanceNodes.clear();
						}
					}
				}
			}
			if (similarDistanceNodes.isEmpty()) {
				Triangle3D<T> newTriangle = getTriangleWithoutRemoving(
						a, b, pickedNode);
				surfaceTriangles.add(newTriangle);
				// add the new edges to the hashmap:
				map.remove(anOpenEdge);
				anOpenEdge = putEdgeOnMap(a, pickedNode, b,
						null, map);
				anOpenEdge = putEdgeOnMap(b, pickedNode, a,
						anOpenEdge, map);

			} else {
				similarDistanceNodes.add(pickedNode);
				anOpenEdge = triangulatePointsOnCircle(
						similarDistanceNodes, anOpenEdge,
						map, surfaceTriangles);
				similarDistanceNodes.clear();
			}
			if ((anOpenEdge == null) && (!map.isEmpty()))
				anOpenEdge = map.values().iterator().next();

		}
		SpaceNode<T> centerNode = this.findCenterNode(nodes);
		for (Triangle3D<T> triangle : surfaceTriangles) {
			if (!triangle.isAdjacentTo(centerNode))
				createNewTetrahedron(triangle, centerNode);
		}
	}

	/**
	 * Calculates the 2D-signed delaunay distance of a point to an edge. 
	 * The edge is here defined by the coordinates of its endpoints. This function uses
	 * precise arithmetics to assure correctness of the result.
	 * 
	 * <p><b>HAS NEVER BEEN USED NOR TESTED! USE AT YOUR OWN RISK!</b>`
	 * @param av The coordinate of the first endpoint of the edge. 
	 * @param bv The coordinate of the second endpoint of the edge. 
	 * @param thirdPoint The point for which the signed delaunay distance should be calculated.
	 * @return A rational number that contains the squared signed delaunay distance of the 
	 * specified edge to the given coordinate. 
	 */
	private Rational calc2DSDDistanceExact(double[] av,
			double[] bv, double[] thirdPoint) {
		ExactVector avX = new ExactVector(av), bvX = new ExactVector(bv), thirdPointX = new ExactVector(
				thirdPoint), avToThirdPointX = thirdPointX
				.subtract(avX);
		ExactVector[] normalsX = new ExactVector[] {
				bvX.subtract(avX),
				bvX.subtract(avX).crossProduct(avToThirdPointX),
				avToThirdPointX };
		Rational[] offsetsX = new Rational[] {
				normalsX[0].dotProduct(avX.add(bvX))
						.multiply(new Rational(1, 2)),
				normalsX[1].dotProduct(avX),
				normalsX[2].dotProduct(avX.add(thirdPointX))
						.multiply(new Rational(1, 2)) };
		ExactVector circumCenter = Triangle3D
				.calculate3PlaneXPoint(normalsX, offsetsX,
						ExactVector.det(normalsX));
		return circumCenter.subtract(
				avX.add(bvX).multiply(new Rational(1, 2)))
				.squaredLength();
	}

	/**
	 * Creates a triangle that can serve as an initial triangle for a triangulation. 
	 * Whenever an open triangle organizer is given a set of points but no open triangles, 
	 * the function {@link #triangulate()} calls this function to compute a starting triangle.
	 * <p><b>HAS NEVER BEEN USED NOR TESTED! USE AT YOUR OWN RISK!</b>`
	 * 
	 * @param rep A reporter object that serves to provide distance information to a 
	 * node organizer. 
	 */
	private void createInitialTriangle(
			AbstractTriangulationNodeOrganizer.DistanceReporter rep) {
		// find a starting node:
		SpaceNode<T> a = tno.getFirstNode();

		double tolerance = 0.000000001;
		// find the second node by minimizing the distance to the first node:
		shortestDistance = Double.MAX_VALUE;
		SpaceNode<T> b = null;
		for (SpaceNode<T> dummy : tno.getNodes(a, rep)) {
			double[] vector = subtract(dummy.getPosition(),
					a.getPosition());
			double distance = dot(vector, vector);
			if (distance < shortestDistance + tolerance) {
				if (distance > shortestDistance - tolerance) {
					Rational distNew = (new ExactVector(a
							.getPosition()))
							.subtract(
									new ExactVector(dummy
											.getPosition()))
							.squaredLength();
					Rational distLast = (new ExactVector(a
							.getPosition())).subtract(
							new ExactVector(b.getPosition()))
							.squaredLength();
					if (distLast.compareTo(distNew) > 0) {
						b = dummy;
						shortestDistance = Math.min(
								shortestDistance, distance);
					}
				} else {
					b = dummy;
					shortestDistance = distance;
					tolerance = 0.000000001 * distance;
				}
			}
		}

		// find the third node by minimizing the distance between the center of
		// the circumcircle and
		// the middle point between a and b:

		shortestDistance = Double.MAX_VALUE;
		double[] av = a.getPosition(), bv = b.getPosition();
		double[][] normals = new double[3][];
		normals[0] = subtract(bv, av);
		double[] offsets = new double[3];
		offsets[0] = 0.5 * dot(normals[0], add(av, bv));
		SpaceNode<T> c = null;
		tolerance = dot(normals[0], normals[0]) * 0.000000001;

		for (SpaceNode<T> dummy : tno.getNodes(a, rep)) {
			double[] dummyPos = dummy.getPosition();
			double[] avToDummyPos = subtract(dummyPos, av);
			normals[1] = crossProduct(normals[0],
					avToDummyPos);
			offsets[1] = dot(normals[1], av);
			normals[2] = avToDummyPos;
			offsets[2] = 0.5 * dot(normals[2], add(av,
					dummyPos));
			// find the circumcenter by cutting 3 planes:
			// the plane describing all points with equal distance to a and b,
			// the plane defined by a, b and dummy and
			// the plane describing all points with equal distance to a and
			// dummy
			double[] circumCenter = Triangle3D
					.calculate3PlaneXPoint(normals, offsets);
			if (circumCenter != null) {
				double[] vector = subtract(circumCenter,
						scalarMult(0.5, add(av, bv)));
				double distance = dot(vector, vector);
				if (distance < shortestDistance + tolerance) {
					if (distance > shortestDistance
							- tolerance) {
						int comparison = calc2DSDDistanceExact(
								av, bv, dummyPos)
								.compareTo(
										calc2DSDDistanceExact(
												av,
												bv,
												c
														.getPosition()));
						if ((comparison < 0)
								|| ((comparison == 0) && (dummy
										.getId() < c
										.getId()))) {
							c = dummy;
							shortestDistance = Math.min(
									shortestDistance,
									distance);
						}
					} else {
						c = dummy;
						shortestDistance = distance;
					}
				}
			}
		}
		putTriangle(new Triangle3D<T>(a, b, c, null, null));
	}

	/**
	 * Creates a new tetrahedron from a given triangle and a fourth node. 
	 * The result is stored in {@link #aNewTetrahedron} (and also in {@link #newTetrahedra} if this is
	 * not null. 
	 * @param openTriangle One side of the new tetrahedron.
	 * @param oppositeNode The fourth point of the tetrahedron.
	 */
	private void createNewTetrahedron(
			Triangle3D<T> openTriangle, SpaceNode<T> oppositeNode) {
		aNewTetrahedron = new Tetrahedron<T>(openTriangle,
				oppositeNode, this);
		if (NewDelaunayTest.createOutput()) {
			if (NewDelaunayTest.checkTetrahedronForDelaunayViolation(aNewTetrahedron)) {
				System.out.println("Stop!");
			}
		}
			
		if (newTetrahedra != null)
			newTetrahedra.add(aNewTetrahedron);
	}

	/**
	 * Finishes an incomplete triangulation. Before calling this function the open triangle organizer
	 * must be informed about all open triangles delimiting the non-triangulated volume in the
	 * tetrahedralization.
	 * <p>
	 * This function will successively pick an open triangle and find the corresponding point for this
	 * triangle. The triangle node organizer (given as argument to {@link #OpenTriangleOrganizer(int, AbstractTriangulationNodeOrganizer)})
	 * will provide the order in which the candidate nodes are tested.
	 * <p>
	 * All tetrahedra created by this function will be reported in
	 *    
	 */
	public void triangulate() {
		AbstractTriangulationNodeOrganizer.DistanceReporter rep = new AbstractTriangulationNodeOrganizer.DistanceReporter() {
			public double getCurrentMinimalDistance() {
				return shortestDistance;
			}
		};
		if (openTriangles.isEmpty())
			createInitialTriangle(rep);
		double upperBound = 0, lowerBound = 0;
		LinkedList<SpaceNode<T>> similarDistanceNodes = new LinkedList<SpaceNode<T>>();
		LinkedList<SpaceNode<T>> onCircleNodes = new LinkedList<SpaceNode<T>>();
		Triangle3D<T> openTriangle = getAnOpenTriangle();
		int securityCounter = 0;
		while (openTriangle != null) {
			openTriangle.update();
			SpaceNode<T> pickedNode = null;
//			if (openTriangle.getOppositeTetrahedron(null).isInfinite()) {
//				createNewTetrahedron(openTriangle, null);
//				openTriangle = getAnOpenTriangle();
//				continue;
//			}
			openTriangle.orientToOpenSide();
			Tetrahedron<T> lastTetrahedron = openTriangle
					.getOppositeTetrahedron(null);
			shortestDistance = Double.MAX_VALUE;
			upperBound = shortestDistance;
			lowerBound = shortestDistance;
			double tolerance = openTriangle
					.getTypicalSDDistance() * 0.0000001;
//			double tolerance = 0.00002;
			if (Math.random() < 0.001)
				addToleranceValue(tolerance);
			for (SpaceNode<T> node : tno.getNodes(openTriangle
					.getNodes()[0], rep)) {
				if (!openTriangle.isAdjacentTo(node)) {
					double currentDistance = openTriangle
							.getSDDistance(node
									.getPosition());
					normalCalculations++;
					// boolean dummy =false;
					// if (dummy) {
					// FieldElement lastSDDistance = openTriangle
					// .getSDDistanceExact((pickedNode !=
					// null)?pickedNode.getPosition():node.getPosition(),
					// lastTetrahedron.getOppositeNode(openTriangle).getPosition());
					// FieldElement newSDDistance = openTriangle
					// .getSDDistanceExact(node.getPosition(),
					// lastTetrahedron.getOppositeNode(openTriangle).getPosition());
					// if (NewDelaunayTest.createOutput()) NewDelaunayTest.out(lastSDDistance);
					// if (NewDelaunayTest.createOutput()) NewDelaunayTest.out(newSDDistance);
					//						
					// }
					if ((currentDistance < upperBound)) { // &&
															// (currentDistance
															// >= 0)) {
						boolean smaller = false;
						if (currentDistance > lowerBound) {
//							System.out.print(".");
							exactCalculations++;
							
							Rational lastSDDistance = openTriangle
									.getSDDistanceExact(
											pickedNode
													.getPosition());
//											lastTetrahedron
//													.getOppositeNode(
//															openTriangle)
//													.getPosition());
							Rational newSDDistance = openTriangle
									.getSDDistanceExact(
											node
													.getPosition());
//											lastTetrahedron
//													.getOppositeNode(
//															openTriangle)
//													.getPosition());
							int comparison = lastSDDistance
									.compareTo(newSDDistance);
							if ((comparison != 0) && ((comparison > 0) ^ (currentDistance < shortestDistance))) {
								double difference = Math.abs(currentDistance - shortestDistance);
								if (difference > maxToleranceNeeded)
									maxToleranceNeeded = difference;
							}
							if (comparison == 0)
								similarDistanceNodes
										.add(node);
							else if (comparison > 0) 
								smaller = true;
						} else
							smaller = true;
						if (smaller) {
							similarDistanceNodes.clear();
							shortestDistance = currentDistance;
							// bounds to determine if another node causes the
							// 'same' signed delaunay distance:
							upperBound = shortestDistance
									+ tolerance;
							lowerBound = shortestDistance
									- tolerance;
							pickedNode = node;
						}
					} else if (openTriangle.orientationToUpperSide(node
							.getPosition()) == 0
							&& openTriangle.circleOrientation(node.getPosition())==0)
						onCircleNodes.add(node);
				}

			}
			if (pickedNode == null
					|| (similarDistanceNodes.isEmpty() && onCircleNodes
							.isEmpty())) {
				// if (pickedNode == null)
				// throw new RuntimeException("No opposing node was found for
				// the triangle "+openTriangle+" (adjacent tetrahedron:
				// "+lastTetrahedron+")");
				if (pickedNode != null && pickedNode.getId() == 12)
					if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("triangulate");
				createNewTetrahedron(openTriangle,
						pickedNode);
			} else {
				// Tetrahedron oppositeTenodetrahedron =
				// openTriangle.getOppositeTetrahedron(null);
				similarDistanceNodes.add(pickedNode);
				// // find all nodes that lie on the same circle as the current
				// open triangle:
				// for (SpaceNode node :
				// tno.getNodes(openTriangle.getNodes()[0], rep)) {
				// if (!openTriangle.isAdjacentTo(node) &&
				// !similarDistanceNodes.contains(node) &&
				// openTriangle.inThePlane(node.getPosition()) &&
				// (oppositeTetrahedron.isInsideSphere(node.getPosition()))) {
				// similarDistanceNodes.add(node);
				// }
				// }
				triangulatePointsOnSphere(
						similarDistanceNodes,
						onCircleNodes, openTriangle);
			}
			similarDistanceNodes.clear();
			onCircleNodes.clear();
			openTriangle = getAnOpenTriangle();
			securityCounter++;
			if (securityCounter > 2000)
				throw new RuntimeException("Am I in an infinite loop?");
//				securityCounter = 0;
		}
		if (!map.isEmpty()) {
			if (NewDelaunayTest.createOutput()) NewDelaunayTest.out("Aarrgh");
		}
	}

//	/**
//	 * @return
//	 */
//	protected AbstractTriangulationNodeOrganizer<T> getTriangulationNodeOrganizer() {
//		return tno;
//	}
}
