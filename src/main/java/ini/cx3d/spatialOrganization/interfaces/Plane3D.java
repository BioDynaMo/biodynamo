package ini.cx3d.spatialOrganization.interfaces;

/**
 * Common interface for Plane3D implementations
 */
public interface Plane3D<T> {
	/**
	 * Reverts the orientation of this plane by switching the sign of all entries
	 * in the normal vector and the offset value.
	 */
	void changeUpperSide();

	/**
	 * Defines the upper side of this plane. The upper side is defined to be the one side
	 * to which the normal vector points to.
	 * @param point A coordinate on the upper side of the plane.
	 */
	void defineUpperSide(double[] point);

	/**
	 * Computes the orientation of two coordinates relative this plane.
	 * @param point1 The first coordinate.
	 * @param point2 The second coordinate.
	 * @return 1, if both points lie on the same side of the plane, -1, if the points
	 * lie on opposite sides of the plane and 0, if either one of the points lies
	 * on the plane.
	 */
	int orientation(double[] point1, double[] point2);

	/**
	 * Returns whether or not two points lie on the same side of this plane.
	 *
	 * @param point1 The first point.
	 * @param point2 The second point.
	 * @return <code>true</code>, if both points lie on the same side of the
	 *         plane and <code>false</code>, if they don't or if one of them
	 *         lies in the plane.
	 */
	boolean trulyOnSameSide(double[] point1, double[] point2);

	/**
	 * Returns whether or not two points lie on different sides of this plane.
	 *
	 * @param point1 The first point.
	 * @param point2 The second point.
	 * @return <code>true</code>, if both points lie on different sides of the
	 *         plane and <code>false</code>, if they don't or if one of them
	 *         lies in the plane.
	 */
	boolean trulyOnDifferentSides(double[] point1, double[] point2);

	/**
	 * Returns whether or not two points lie on the same side of this plane.
	 *
	 * @param point1 The first point.
	 * @param point2 The second point.
	 * @return <code>true</code>, if both points lie on the same side of the
	 *         plane or if any one of them lies in the plane
	 *         and <code>false</code>, if they don't or if one of them
	 *         lies in the plane.
	 */
	boolean onSameSide(double[] point1, double[] point2);

	/**
	 * @return The normal vector of this plane.
	 */
	double[] getNormalVector();
}
