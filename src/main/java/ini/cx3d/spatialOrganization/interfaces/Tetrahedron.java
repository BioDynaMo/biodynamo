package ini.cx3d.spatialOrganization.interfaces;

import ini.cx3d.spatialOrganization.PositionNotAllowedException;

/**
 * Common interface for Tetrahedron implementations
 */
public interface Tetrahedron<T> {
	/**
	 * Extracts the user objects associated with the four endpoints of this
	 * tetrahedron.
	 *
	 * @return An array of objects of type <code>T</code>.
	 */
	T[] getVerticeContents();

	/**
	 * Returns whether this tetrahedron is infinite.
	 *
	 * @return <code>true</code>, if the tetrahedron is infinite (first
	 *         endpoint is null).
	 */
	boolean isInfinite();

	/**
	 * Returns whether this tetrahedron is a flat tetrahedron. Used to simplify
	 * distinction between the two types <code>Tetrahedron</code> and
	 * <code>FlatTetrahedron</code>.
	 *
	 * @return <code>false</code> for all instances of
	 *         <code>Tetrahedron</code>.
	 */
	boolean isFlat();

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
	 */
	void changeCrossSection(int number, double newValue);

	/**
	 * Calculates all cross section areas of the six edges incident to this
	 * tetrahedron.
	 */
	void updateCrossSectionAreas();

	/**
	 * Calculates the volume of this tetrahedron and changes the stored value.
	 * (The volume equals 1/6th of the determinant of 3 incident edges with a
	 * common endpoint.)
	 */
	void calculateVolume();

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
	 */
	int orientationExact(double[] position);

	/**
	 * Calculates the properties of this tetrahedron's circumsphere.
	 */
	void calculateCircumSphere();

	/**
	 * Determines wether a given point lies inside or outside the circumsphere
	 * of this tetrahedron or lies on the surface of this sphere.
	 *
	 * @param point
	 *            The point for which the orientation should be determined.
	 * @return -1, if the point lies outside this tetrahedron's circumsphere, 1
	 *         if it is inside the sphere and 0, if it lies on the surface of
	 *         the sphere.
	 */
	int orientation(double[] point);

	/**
	 * Determines wether a given point lies truly inside the circumsphere of
	 * this tetrahedron
	 *
	 * @param point
	 *            The point for which the orientation should be determined.
	 * @return <code>true</code> if the distance of the point to the center of
	 *         the circumsphere is smaller than the radius of the circumsphere
	 *         and <code>false</code> otherwise.
	 */
	boolean isTrulyInsideSphere(double[] point);

	/**
	 * Determines wether a given point lies truly inside the circumsphere of
	 * this tetrahedron
	 *
	 * @param point
	 *            The point for which the orientation should be determined.
	 * @return <code>true</code> if the distance of the point to the center of
	 *         the circumsphere is smaller or equal to the radius of the
	 *         circumsphere and <code>false</code> otherwise.
	 */
	boolean isInsideSphere(double[] point);

	/**
	 * Removes this tetrahedron from the triangulation. All the incident nodes,
	 * edges and triangles are informed that this tetrahedron is being removed.
	 *
	 * !IMPORTANT!: No triangle organizer is informed about the removement of
	 * this tetrahedron. A caller of this function must keep track of the new
	 * open triangles itself!
	 */
	void remove();

	/**
	 * Replaces one of the incident triangles of this tetrahedron. Automatically
	 * exchanges the affected edges, too.
	 *
	 * @param oldTriangle
	 *            The triangle that should be replaced.
	 * @param newTriangle
	 *            The new trianlge.
	 */
	void replaceTriangle(Triangle3D<T> oldTriangle,
						 Triangle3D<T> newTriangle);

	/**
	 * Determines which index a given node has in this tetrahedron's list of
	 * endpoints.
	 *
	 * @param node
	 *            The node of interest.
	 * @return An index between 0 and 3.
	 */
	int getNodeNumber(ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> node);

	/**
	 * Determines which index a given triangle has in this tetrahedron's list of
	 * incident triangles.
	 *
	 * @param triangle
	 *            The triangle of interest.
	 * @return An index between 0 and 3.
	 */
	int getTriangleNumber(Triangle3D triangle);

	/**
	 * Determines the edge that connects two endpoints of this tetrahedron.
	 *
	 * @param nodeNumber1
	 *            The index of the first endpoint of the edge.
	 * @param nodeNumber2
	 *            The index of the second endpoint of the edge.
	 * @return The edge connecting the two endpoints with the given indices.
	 */
	Edge getEdge(int nodeNumber1, int nodeNumber2);

	/**
	 * Determines the edge that connects two endpoints of this tetrahedron.
	 *
	 * @param a
	 *            The first endpoint of the edge.
	 * @param b
	 *            The second endpoint of the edge.
	 * @return A number between 0 and 5, giving the index of the edge of
	 *         interest.
	 */
	int getEdgeNumber(ini.cx3d.spatialOrganization.interfaces.SpaceNode a, ini.cx3d.spatialOrganization.interfaces.SpaceNode b);

	/**
	 * Determines the edge that connects two endpoints of this tetrahedron.
	 *
	 * @param a
	 *            The first endpoint of the edge.
	 * @param b
	 *            The second endpoint of the edge.
	 * @return The edge connecting the two given endpoints.
	 */
	Edge getEdge(ini.cx3d.spatialOrganization.interfaces.SpaceNode a, ini.cx3d.spatialOrganization.interfaces.SpaceNode b);

	/**
	 * Returns the incident triangle opposite to a given endpoint of this
	 * tetrahedron.
	 *
	 * @param node
	 *            An endpoint of this tetrahedron.
	 * @return A reference to the triangle that lies opposite to
	 *         <code>node</code>.
	 */
	Triangle3D<T> getOppositeTriangle(ini.cx3d.spatialOrganization.interfaces.SpaceNode node);

	/**
	 * Returns the incident node opposite to a given triangle which is incident
	 * to this tetrahedron.
	 *
	 * @param triangle
	 *            An incident triangle of this tetrahedron.
	 * @return The endpoint of this triangle that lies opposite to
	 *         <code>triangle</code>.
	 */
	SpaceNode<T> getOppositeNode(Triangle3D triangle);

	/**
	 * Returns a reference to the triangle connecting this tetrahedron with
	 * another one.
	 *
	 * @param tetrahedron
	 *            An adjacent tetrahedron.
	 * @return The triangle which is incident to this tetrahedron and
	 *         <code>tetrahedron</code>.
	 */
	Triangle3D<T> getConnectingTriangle(Tetrahedron tetrahedron);

	/**
	 * Returns this index of the triangle connecting this tetrahedron with
	 * another one.
	 *
	 * @param tetrahedron
	 *            An adjacent tetrahedron.
	 * @return An index between 0 and 3 which is the position of the triangle
	 *         incident to this tetrahedron and <code>tetrahedron</code> in
	 *         this tetrahedron's list of incident triangles.
	 */
	int getConnectingTriangleNumber(Tetrahedron tetrahedron);

	/**
	 * Returns the three incident triangles that are adjacent to a given
	 * triangle.
	 *
	 * @param base
	 *            A triangle which is incident to this tetrahedron.
	 * @return An array of three triangles.
	 */
	Triangle3D<T>[] getTouchingTriangles(Triangle3D base);

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
	 */
	boolean isPointInConvexPosition(double[] point,
									int connectingTriangleNumber);

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
	 */
	int isInConvexPosition(double[] point,
						   int connectingTriangleNumber);

	/**
	 * @return An array containing the nodes incident to this tetrahedron.
	 */
	SpaceNode<T>[] getAdjacentNodes();

	/**
	 * Returns the second tetrahedron that is incident to the incident triangle with index <code>number</code>.
	 * @param number An index specifying a position in the list of triangles of this tetrahedron. The
	 * corresponding triangle will be chosen to determine the adjacent tetrahedron.
	 * @return An adjacent tetrahedron.
	 */
	Tetrahedron<T> getAdjacentTetrahedron(int number);

	/**
	 * @return An array of triangles containing the 4 triangles incident to this tetrahedron.
	 */
	Triangle3D<T>[] getAdjacentTriangles();

	/**
	 * Determines whether a given node is an endpoint of this tetrahedron.
	 * @param node The node of interest.
	 * @return <code>true</code>, if the node is an endpoint.
	 */
	boolean isAdjacentTo(ini.cx3d.spatialOrganization.interfaces.SpaceNode node);

	/**
	 * Walks toward a specified point. If the point lies inside this
	 * tetrahedron, this tetrahedron is returned. Otherwise, an adjacent
	 * tetrahedron is returned that lies closer to the point.
	 *
	 * @param coordinate
	 *            The coordinate that should be approximated.
	 * @param triangleOrder
	 * 			  A small list containing a permutation of the number 0-3. This list is
	 *            used to randomize the visibility walk implemented in
	 * @return An adjacent tetrahedron that lies closer to the specified point
	 *         than this tetrahedron, or this tetrahedron, if the point lies inside it.
	 */
	Tetrahedron<T> walkToPoint(double[] coordinate, int[] triangleOrder)
			throws PositionNotAllowedException;

	/**
	 * Checks if a node may be moved to a given coordinate.
	 * @param position The coordinate of interest.
	 * @throws PositionNotAllowedException If the position is equal to any endpoint of this tetrahedron.
	 */
	void testPosition(double[] position)
			throws PositionNotAllowedException;

	/**
	 * When this tetrahedron is removed, there might still be references to
	 * this tetrahedron.  Therefore, a flag is set to save that this tetrahedron was removed and
	 * this can be read using this function.
	 *
	 * @return <code>true</code>, iff this tetrahedron is still part of the triangulation.
	 */
	boolean isValid();

	/**
	 * Returns whether a given tetrahedron is adjacent to this tetrahedron.
	 * @param otherTetrahedron The potential neighbor of this tetrahedron.
	 * @return <code>true</code>, iff this tetrahedron is adjacent to <code>otherTetrahedron</code>.
	 */
	boolean isNeighbor(Tetrahedron otherTetrahedron);

	/**
	 * Given two nodes incident to this tetrahedron, this function returns
	 * another endpoint. The returned endpoint is different from the result of
	 * {@link #getSecondOtherNode(ini.cx3d.spatialOrganization.interfaces.SpaceNode, ini.cx3d.spatialOrganization.interfaces.SpaceNode)}.
	 *
	 * @param nodeA
	 *            A first incident node.
	 * @param nodeB
	 *            A second incident node.
	 * @return A third incident node.
	 */
	SpaceNode<T> getFirstOtherNode(ini.cx3d.spatialOrganization.interfaces.SpaceNode nodeA, ini.cx3d.spatialOrganization.interfaces.SpaceNode nodeB);

	/**
	 * Given two nodes incident to this tetrahedron, this function returns
	 * another endpoint. The returned endpoint is different from the result of
	 * {@link #getFirstOtherNode(ini.cx3d.spatialOrganization.interfaces.SpaceNode, ini.cx3d.spatialOrganization.interfaces.SpaceNode)}.
	 *
	 * @param nodeA
	 *            A first incident node.
	 * @param nodeB
	 *            A second incident node.
	 * @return A third incident node.
	 */
	SpaceNode<T> getSecondOtherNode(ini.cx3d.spatialOrganization.interfaces.SpaceNode nodeA, ini.cx3d.spatialOrganization.interfaces.SpaceNode nodeB);

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
	 */
	void updateCirumSphereAfterNodeMovement(ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> movedNode);
}
