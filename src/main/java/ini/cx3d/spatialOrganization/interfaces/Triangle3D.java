package ini.cx3d.spatialOrganization.interfaces;

import ini.cx3d.spatialOrganization.SpaceNode;
import ini.cx3d.spatialOrganization.Tetrahedron;
/**
 * Common interface for Triangle3D implementations
 */
public interface Triangle3D<T> extends Plane3D<T> {
	/**
	 * Compares this triangle to another triangle.
	 *
	 * @param otherTriangle
	 *            The other triangle.
	 * @return <code>true</code>, if both triangles are incident to the same
	 *         nodes.
	 */
	boolean isSimilarTo(Triangle3D<T> otherTriangle);

	/**
	 * Returns the distance of the center of a circumsphere touching all points
	 * of this triangle and the given 4th point and the center of the
	 * circumcenter touching all three points of the triangle. The sign of the
	 * returned value indicates whether the center of the circumsphere lies on
	 * the upper side of the plane defined by this triangle.
	 * <p>
	 * This distance is not necessarily normalized!
	 *
	 * @param fourthPoint
	 *            The 4th point defining the circumsphere.
	 * @return The signed Delaunay distance or {@link Double#MAX_VALUE} if it
	 *         cannot be calculated.
	 *
	 * @see #calculateSDDistance(double[])
	 */
	double getSDDistance(double[] fourthPoint);

	/**
	 * Calculates the distance of the center of a circumsphere touching all
	 * points of this triangle and the given 4th point and the center of the
	 * circumcenter touching all three points of the triangle. This distance is
	 * NOT normalized since the vector orthogonal is not normalized!
	 *
	 * @param fourthPoint
	 *            The 4th point defining the circumsphere.
	 * @return The signed delaunay distance or -1 if it cannot be calculated.
	 *
	 */
	Rational getSDDistanceExact(double[] fourthPoint);

	/**
	 * Calculates the center of the circumsphere around the three endpoints of this triangle and a
	 * given fourth point.
	 * <p>
	 * <b>Currently, there is no function implemented that estimates the error of this function's
	 * result. Therefore, the function {@link ini.cx3d.spatialOrganization.Tetrahedron#calculateCircumSphere()} should be preferred.</b>
	 * @param fourthPoint The fourth point defining the sphere.
	 * @return The coordinate of the center of the circumsphere if it can be computed and <code>null</code> else.
	 */
	double[] calculateCircumSphereCenter(double[] fourthPoint);

	/**
	 * Calculates the center of the circumsphere around the three endpoints of this triangle and a
	 * given fourth point if the circumcenter of this triangle is updated.
	 * <p>
	 * <b>Currently, there is no function implemented that estimates the error of this function's
	 * result. Therefore, the function {@link Tetrahedron#calculateCircumSphere()} should be preferred.</b>
	 * @param fourthPoint The fourth point defining the sphere.
	 * @return The coordinate of the center of the circumsphere if the circumcenter of
	 * 	this triangle was updated and <code>null</code> else.
	 */
	double[] calculateCircumSphereCenterIfEasy(double[] fourthPoint);

	/**
	 * This function informs the triangle that one of its incident nodes moved.
	 * Therefore, the plane equation and the circumcircle will have to be recalculated.
	 */
	void informAboutNodeMovement();

	/**
	 * Updates the plane equation for this triangle if and incident node has moved since the last
	 * update.
	 */
	void updatePlaneEquationIfNecessary();

	/**
	 * Updates all parameters of this triangle.
	 */
	void update();

	/**
	 * {@inheritDoc}
	 */
	int orientationExact(double[] point1, double[] point2);

	/**
	 * Computes the orientation of a point to the circumcircle of this triangle.
	 * This function does NOT test whether the given coordinate lies in the plane of this triangle.
	 * It only compares the distance of the point to the circumcenter with the radius of the
	 * circumcircle.
	 * @param point The coordinate of interest.
	 * @return 1, if the distance of <code>point</code> is smaller than the radius of the circumcircle, 0, if it is equal and 1, if
	 * it is bigger.
	 */
	int circleOrientation(double[] point);

	/**
	 * Given a tetrahedron which is incident to this triangle, this function returns the second tetrahedron incident to
	 * this triangle.
	 * @param incidentTetrahedron A tetrahedron incident to this triangle.
	 * @return The tetrahedron opposite to <code>incidentTetrahedron</code> at this triangle.
	 * @throws RuntimeException if <code>incidentTetrahedron</code> is not incident to this triangle.
	 */
	Tetrahedron<T> getOppositeTetrahedron(Tetrahedron<T> incidentTetrahedron);

	/**
	 * Removes a given tetrahedron from the list of incident tetrahedra.
	 * @param tetrahedron A tetrahedron incident to this triangle.
	 */
	void removeTetrahedron(Tetrahedron<T> tetrahedron);

	/**
	 * Tests whether this triangle has an open side and whether a given coordinate
	 * and the incident tetrahedron lie on opposite sides of the triangle.
	 * @param point The coordinate of interest.
	 * @return <code>true</code>, if this triangle has an open side and
	 * if the given coordinate lies on the open side.
	 */
	boolean isOpenToSide(double[] point);

	/**
	 * This function detects on which side of the plane defined by this
	 * triangle a given point lies. This side is then defined to be the
	 * upper side of this triangle.
	 * @param position A coordinate that defines the 'upper side' of this triangle
	 * 			A runtime exception is thrown if the given point lies in the plane
	 * 			defined by this triangle.
	 */
	void orientToSide(double[] position);

	/**
	 * This function determines whether this triangle has a single open side.
	 * If this is the case, the fourth point incident to the tetrahedron incident to this triangle
	 * is defined to be on the lower side of the triangle, thereby defining the open side as the upper side.
	 * If this triangle has either two open sides or no open sides, this function throws a RuntimeException.
	 */
	void orientToOpenSide();

	/**
	 * Determines whether a given coordinate lies on the upper side of this triangle (which must be defined
	 * beforehand, using either {@link #orientToOpenSide()} or {@link #orientToSide(double[])}.
	 * @param point The coordinate of interest.
	 * @return -1, if the coordinate lies on the lower side of the triangle, +1 if it lies on the upper
	 * 			side of the triangle an 0 if it lies in the plane.
	 */
	int orientationToUpperSide(double[] point);

	/**
	 * Determines whether a given coordinate lies on the upper side of this triangle.
	 * @param point The coordinate of interest.
	 * @return <code>true</code>, if the given point lies on the upper side of this triangle or in the plane
	 * defined by this triangle and <code>false</code> otherwise.
	 * @see #orientToSide(double[])
	 * @see #orientToOpenSide()
	 */
	boolean onUpperSide(double[] point);

	/**
	 * Determines whether a given coordinate lies truly on the upper side of this triangle.
	 * @param point The coordinate of interest.
	 * @return <code>true</code>, if the given point lies on the upper side of this triangle and <code>false</code> otherwise.
	 * @see #orientToSide(double[])
	 * @see #orientToOpenSide()
	 */
	boolean trulyOnUpperSide(double[] point);

	/**
	 * Calculates an sd-distance which could typically be expected from points lying in a distance
	 * similar to the distance between the points of this triangle. Used to compute tolerance values in
	 * {@link OpenTriangleOrganizer#triangulate()} but very imprecise! Needs to removed and replaced!
	 * @return An unreliable double value.
	 *
	 */
	double getTypicalSDDistance();

	/**
	 * Tests if this triangle is not incident to any tetrahedron.
	 * @return <code>true</code>, iff this triangle has no incident tetrahedra.
	 */
	boolean isCompletelyOpen();

	/**
	 * @return A reference to the array storing the three endpoints of this
	 * tetrahedron.
	 */
	SpaceNode<T>[] getNodes();

	/**
	 * Returns whether this triangle has already been tested if it is locally Delaunay.
	 * This function is used in a run of {@link SpaceNode#restoreDelaunay()} to
	 * keep track of which triangles have already been tested.
	 * <p>If this triangle was not tested yet, it is immediately marked as being tested.
	 * @param checkingIndex The unique identifier of the run of <code>restoreDelaunay</code>.
	 * @return <code>true</code>, iff this triangle has already been tested for the
	 * Delaunay property.
	 */
	boolean wasCheckedAlready(int checkingIndex);

	/**
	 * Adds an incident tetrahedron to this triangle.
	 * @param tetrahedron A new tetrahedron which is incident to this triangle.
	 */
	void addTetrahedron(Tetrahedron<T> tetrahedron);

	/**
	 * Returns whether this triangle is incident to a given node.
	 *
	 * @param node A node that might be incident to this triangle.
	 * @return <code>true</code>, iff the node is incident to this triangle.
	 */
	boolean isAdjacentTo(SpaceNode<T> node);

	/**
	 * Returns whether this triangle is incident to a given tetrahedron.
	 * @param tetrahedron a tetrahedron that might be incident to this triangle.
	 * @return <code>true</code>, iff the tetrahedron is incident to this triangle.
	 */
	boolean isAdjacentTo(Tetrahedron<T> tetrahedron);

	/**
	 * Tests if this triangle is incident to two tetrahedra.
	 * @return <code>true</code>, iff this triangle has two incident tetrahedra.
	 */
	boolean isClosed();

	/**
	 * Tests whether this triangle has infinite size, meaning that it is
	 * incident to <code>null</code>.
	 *
	 * @return <code>true</code>, if this tetrahedron is incident to a '<code>null</code>'-node.
	 */
	boolean isInfinite();
}
