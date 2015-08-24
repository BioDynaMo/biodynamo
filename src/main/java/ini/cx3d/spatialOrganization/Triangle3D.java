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

/**
 * Represents a triangle in three-dimensional space. A triangle is defined by its three endpoints, and two incident tetrahedra.
 * Each triangle stores information about the corresponding plane equation as well as its circumcircle. 
 * 
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of user objects associated with the nodes in this triangulation.
 */
/**
 * @author dennis
 * 
 * @param <T>
 */
public class Triangle3D<T> extends Plane3D<T> {

	/**
	 * The two tetrahedra that are incident to this triangle.
	 */
	private Tetrahedron<T>[] adjacentTetrahedra = new Tetrahedron[2];

	/**
	 * The three nodes that are incident to this triangle.
	 */
	private SpaceNode<T>[] nodes = new SpaceNode[3];

	/**
	 * The coordinate of this triangle's circumcenter.
	 */
	private double[] circumCenter = null;

	/**
	 * Stores whether the plane equation has been updated since the last change
	 * of any endpoint.
	 */
	private boolean planeUpdated = false;

	/**
	 * Stores whether the circumcenter is up to date.
	 */
	private boolean circumCenterUpdated = false;

	/**
	 * Defines the upper side of this triangle. A point is said to be on the
	 * upper side iff its dot product with the normal vector of the plane
	 * equation minus the offset is a positive number nad
	 * <code>upperSidePositive</code> is <code>true</code> or vice versa.
	 */
	private boolean upperSidePositive = true;

	/**
	 * Used to remember during the flip algorithm whether this triangle has
	 * already been tested to be locally Delaunay.
	 */
	private int connectionChecked = -1;

	/**
	 * Creates a new triangle from three nodes and two tetrahedra.
	 * 
	 * @param sn1
	 *            The first incident node.
	 * @param sn2
	 *            The second incident node.
	 * @param sn3
	 *            The third incident node.
	 * @param tetrahedron1
	 *            The first incident tetrahedron.
	 * @param tetrahedron2
	 *            The second incident tetrahedron.
	 */
	public Triangle3D(SpaceNode<T> sn1, SpaceNode<T> sn2, SpaceNode<T> sn3,
			Tetrahedron<T> tetrahedron1, Tetrahedron<T> tetrahedron2) {
		nodes[0] = sn1;
		nodes[1] = sn2;
		if (sn2 == null) {
			nodes[1] = sn1;
			nodes[0] = null;
		}
		nodes[2] = sn3;
		if (sn3 == null) {
			nodes[2] = sn1;
			nodes[0] = null;
		}
		adjacentTetrahedra[0] = tetrahedron1;
		adjacentTetrahedra[1] = tetrahedron2;
	}

	/**
	 * Compares this triangle to another triangle.
	 * 
	 * @param otherTriangle
	 *            The other triangle.
	 * @return <code>true</code>, if both triangles are incident to the same
	 *         nodes.
	 */
	public boolean isSimilarTo(Triangle3D<T> otherTriangle) {
		SpaceNode<T>[] otherNodes = otherTriangle.getNodes();
		return isAdjacentTo(otherNodes[0]) && isAdjacentTo(otherNodes[1])
				&& isAdjacentTo(otherNodes[2]);
	}

	/**
	 * Tests whether this triangle has infinite size, meaning that it is
	 * incident to <code>null</code>.
	 * 
	 * @return <code>true</code>, if this tetrahedron is incident to a '<code>null</code>'-node.
	 */
	protected boolean isInfinite() {
		return nodes[0] == null;
	}

	// public Triangle3D(Tetrahedron tetrahedron, SpaceNode nonUsedNode) {
	// super();
	//		
	// int pos = 0;
	// SpaceNode[] nodes = tetrahedron.getAdjacentNodes();
	// for (int i = 0; i < nodes.length; i++) {
	// if (nodes[i] != nonUsedNode) {
	// this.nodes[pos++] = nodes[i];
	// // points[pos++] = nodes[i].getPosition();
	// }
	// }
	//		
	//	
	// // calculate the crossing of 3 Planes:
	// // just a dummy because it is used so often:
	// }

	/**
	 * Creates a string representation of this triangle.
	 * 
	 * @return A string in the format "(ID1, ID2, ID3)", where IDx denote the
	 *         ID's of the incident nodes.
	 */
	public String toString() {
		return "(" + nodes[0] + "," + nodes[1] + "," + nodes[2] + ")";
	}

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
	public double getSDDistance(double[] fourthPoint) {
		if (!isInfinite() && onUpperSide(fourthPoint)) {
			double sdDistance = calculateSDDistance(fourthPoint);
			if (sdDistance != Double.MAX_VALUE)
				return (upperSidePositive) ? sdDistance : -sdDistance;
			else
				return Double.MAX_VALUE;
		} else
			return Double.MAX_VALUE;
	}

	/**
	 * Calculates the distance of the center of a circumsphere touching all
	 * points of this triangle and the given 4th point and the center of the
	 * circumcenter touching all three points of the triangle.
	 * <p>
	 * This distance is not necessarily normalized!
	 * 
	 * @param fourthPoint
	 *            The 4th point defining the circumsphere.
	 * @return The signed delaunay distance or -1 if it cannot be calculated.
	 */
	private double calculateSDDistance(double[] fourthPoint) {
		if (!isInfinite()) {
			// calc that distance within 6 subtractions, 3 additions, 1 division
			// and 9 multiplications. Beat that!
			double[] ad = subtract(nodes[0].getPosition(), fourthPoint);
			double denominator = dot(ad, normalVector);
			// if ((denominator != 0.0) && (Math.abs(denominator) < tolerance))
			// {
			if ((denominator != 0.0) && (Math.abs(denominator) < tolerance)) {
				ExactVector n0Vector = new ExactVector(nodes[0].getPosition());
				ExactVector v1 = n0Vector.subtract(new ExactVector(nodes[1]
						.getPosition()));
				ExactVector v2 = n0Vector.subtract(new ExactVector(nodes[2]
						.getPosition()));
				ExactVector normalVector = v1.crossProduct(v2);
				Rational dot = normalVector.dotProduct(n0Vector
						.subtract(new ExactVector(fourthPoint)));
				if (dot.isZero())
					denominator = 0.0;
				else {
					denominator = ((Rational) dot).doubleValue();
					dot = normalVector.dotProduct(new ExactVector(
							this.normalVector));
					if (dot.compareTo(new Rational(0, 1)) < 0)
						denominator = 0 - denominator;
				}
			}
			if (denominator != 0) {
				double sdDistance = dot(ad, subtract(scalarMult(0.5, add(
						nodes[0].getPosition(), fourthPoint)), circumCenter))
						/ denominator;
				// int test = 0;
				// if (test == 1) {
				// double[] sphereCenter = add(
				// circumCenter, scalarMult(
				// sdDistance,
				// normalVector));
				// if (NewDelaunayTest.createOutput())
				// NewDelaunayTest
				// .out("Sphere Center lies on "
				// + ((this
				// .onUpperSide(sphereCenter)) ? "upper"
				// : "lower")
				// + "side of the plane!");
				// if (NewDelaunayTest.createOutput())
				// NewDelaunayTest
				// .out("Checking sphere center:");
				// if (NewDelaunayTest.createOutput())
				// NewDelaunayTest.out(norm(subtract(
				// sphereCenter, nodes[0]
				// .getPosition())));
				// if (NewDelaunayTest.createOutput())
				// NewDelaunayTest.out(norm(subtract(
				// sphereCenter, nodes[1]
				// .getPosition())));
				// if (NewDelaunayTest.createOutput())
				// NewDelaunayTest.out(norm(subtract(
				// sphereCenter, nodes[2]
				// .getPosition())));
				// if (NewDelaunayTest.createOutput())
				// NewDelaunayTest.out(norm(subtract(
				// sphereCenter, fourthPoint))
				// + "\n");
				// }
				return sdDistance;
			}
		}
		return Double.MAX_VALUE;

	}

	/**
	 * Computes the normal vector for the plane equation of this triangle. The
	 * normal vector is calculated using precise arithmetics and therefore, the
	 * result is given as an instance of <code>ExactVector</code>.
	 * 
	 * @return The normal vector for this triangle.
	 */
	protected ExactVector getExactNormalVector() {
		return calculateExactNormalVector(this.getExactPositionVectors());
	}

	/**
	 * Computes the normal vector for a plane equation given by three position
	 * vectors. The normal vector is calculated using precise arithmetics and
	 * therefore, the result is given as an instance of <code>ExactVector</code>.
	 * 
	 * @param points
	 *            The endpoints of the triangle for which the normal vector
	 *            should be calculated, given in exact representation.
	 * @return The normal vector for the triangle defined by the three
	 *         coordinates.
	 */
	private ExactVector calculateExactNormalVector(ExactVector[] points) {
		return points[1].subtract(points[0]).crossProduct(
				points[2].subtract(points[0]));

	}

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
	public Rational getSDDistanceExact(double[] fourthPoint) {// , double[]
																// occupiedSide)
																// {
		if (!isInfinite() && onUpperSide(fourthPoint)) {
			ExactVector[] points = new ExactVector[4];
			for (int i = 0; i < 3; i++)
				points[i] = new ExactVector(nodes[i].getPosition());
			points[3] = new ExactVector(fourthPoint);
			ExactVector normalVector = calculateExactNormalVector(points);
			if (normalVector.dotProduct(new ExactVector(this.normalVector))
					.compareTo(new Rational(0, 1)) < 0)
				// Rational zero = new Rational(0, 1);
				// int result = (int) Math
				// .signum(normalVector
				// .multiply(
				// points[3]
				// .subtract(points[0]))
				// .multiply(
				// normalVector
				// .multiply((new Vector(
				// occupiedSide))
				// .subtract(points[0])))
				// .compareTo(zero));
				// if (result > 0)
				normalVector.negate();
			return (upperSidePositive) ? calculateSDDistanceExact(points,
					normalVector) : calculateSDDistanceExact(points,
					normalVector).negate();
			// return calculateSDDistanceExact(points,normalVector);
		} else
			return new Rational(Long.MAX_VALUE, 1);
	}

	/**
	 * Returns the distance of the center of a circumsphere touching all points
	 * of this triangle and the given 4th point and the center of the
	 * circumcenter touching all three points of the triangle. The sign of the
	 * returned value indicates whether the center of the circumsphere lies on
	 * the upper side of the plane defined by this triangle.
	 * <p>
	 * This function uses precise arithmetics to assure reliability of the
	 * result.
	 * <p>
	 * The calculated distance is not normalized! It can therefore only be used
	 * to compare this signed delaunay distance with the delaunay distance of
	 * the same triangle to another coordinate.
	 * 
	 * @param points
	 *            An array of three vectors. The first three points are expected
	 *            to be the three endpoints of the triangle and the fourth is
	 *            expected to be the point to which the Delaunay distance should
	 *            be calculated.
	 * @param normalVector
	 *            A normal vector for the plane defined by the three endpoints
	 *            of the triangle.
	 * @return The signed delaunay distance or {@link Long#MAX_VALUE} if it
	 *         cannot be calculated.
	 */
	private Rational calculateSDDistanceExact(ExactVector[] points,
			ExactVector normalVector) {
		if (!isInfinite()) {
			// calc that distance within 6 subtractions, 3 additions, 1 division
			// and 9 multiplications. Beat that!
			ExactVector ad = points[0].subtract(points[3]);
			Rational denominator = ad.dotProduct(normalVector);
			if (!denominator.isZero()) {
				ExactVector circumCenter = calculateCircumCenterExact(points,
						normalVector);
				return points[0].add(points[3]).divideBy(new Rational(2, 1))
						.decreaseBy(circumCenter).dotProduct(ad).divideBy(
								denominator);
			}
		}
		return new Rational(Long.MAX_VALUE);
	}

	/**
	 * Calculates the center of the circumsphere around the three endpoints of this triangle and a
	 * given fourth point.
	 * <p>
	 * <b>Currently, there is no function implemented that estimates the error of this function's 
	 * result. Therefore, the function {@link Tetrahedron#calculateCircumSphere()} should be preferred.</b>
	 * @param fourthPoint The fourth point defining the sphere.
	 * @return The coordinate of the center of the circumsphere if it can be computed and <code>null</code> else.
	 */
	public double[] calculateCircumSphereCenter(double[] fourthPoint) {
		if (!isInfinite()) {
			double sd = calculateSDDistance(fourthPoint);
			return add(circumCenter, scalarMult(sd, normalVector));
		} else
			return null;
	}

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
	public double[] calculateCircumSphereCenterIfEasy(double[] fourthPoint) {
		if (circumCenterUpdated) {
			// checkIfUpdated();
			// if (circumCenterUpdated)
			return calculateCircumSphereCenter(fourthPoint);
		}
		return null;
	}

	/**
	 * Calculates the crossing point of three planes given in normal form.
	 * 
	 * @param normals
	 *            The normals of the three planes. <code>normals[i]</code>
	 *            denotes the normal vector of the <code>i</code>th plane.
	 * @param offsets
	 *            The offsets of the three planes. Say, E0: n0.x == n[0].s,
	 *            where s is a point on the plane. Then offset[0] = n[0].s.
	 * @param normalDet
	 *            The determinant of the Matrix normals
	 * @return The cutting point of the three planes if there is any and the
	 *         maximum vector possible if not.
	 */
	public static double[] calculate3PlaneXPoint(double[][] normals,
			double[] offsets, double normalDet) {
		if (normalDet != 0)
			return scalarMult(1 / normalDet,
					add(scalarMult(offsets[0], crossProduct(normals[1],
							normals[2])), scalarMult(offsets[1], crossProduct(
							normals[2], normals[0])), scalarMult(offsets[2],
							crossProduct(normals[0], normals[1]))));
		else
			return new double[] { Double.MAX_VALUE, Double.MAX_VALUE,
					Double.MAX_VALUE };
	}

	/**
	 * Calculates the crossing point of three planes given in normal form using 
	 * precise arithmetics. 
	 * 
	 * @param normals
	 *            The normals of the three planes. <code>normals[i]</code>
	 *            denotes the normal vector of the <code>i</code>th plane.
	 * @param offsets
	 *            The offsets of the three planes. Say, E0: n0.x == n[0].s,
	 *            where s is a point on the plane. Then offset[0] = n[0].s.
	 * @param normalDet
	 *            The determinant of the Matrix normals
	 * @return The cutting point of the three planes if there is any and the
	 *         maximum vector possible if not.
	 */
	public static ExactVector calculate3PlaneXPoint(ExactVector[] normals,
			Rational[] offsets, Rational normalDet) {
		if (!normalDet.isZero())
			return normals[1].crossProduct(normals[2]).multiplyBy(offsets[0])
					.increaseBy(
							normals[2].crossProduct(normals[0]).multiplyBy(
									offsets[1]).increaseBy(
									normals[0].crossProduct(normals[1])
											.multiplyBy(offsets[2]))).divideBy(
							normalDet);
		else
			return new ExactVector(new Rational[] {
					new Rational(Long.MAX_VALUE, 1),
					new Rational(Long.MAX_VALUE, 1),
					new Rational(Long.MAX_VALUE, 1) });
	}

	/**
	 * Calculates the crossing point of three planes given in normal form.
	 * 
	 * @param normals
	 *            The normals of the three planes. <code>normals[i]</code>
	 *            denotes the normal vector of the <code>i</code>th plane.
	 * @param offsets
	 *            The offsets of the three planes. Say, E0: n0.x == n[0].s,
	 *            where s is a point on the plane. Then offset[0] = n[0].s.
	 * @return The cutting point of the three planes if there is any and the
	 *         maximum vector possible if not.
	 */
	public static double[] calculate3PlaneXPoint(double[][] normals,
			double[] offsets) {
		return calculate3PlaneXPoint(normals, offsets, det(normals));
	}

	/**
	 * This function informs the triangle that one of its incident nodes moved. 
	 * Therefore, the plane equation and the circumcircle will have to be recalculated.
	 */
	public void informAboutNodeMovement() {
		this.circumCenterUpdated = false;
		this.planeUpdated = false;
		this.normalVectorUpdated = false;
	}

	// private void checkIfUpdated() {
	// boolean result = true;
	// for (int i = 0; i < nodes.length; i++) {
	// double[] dummyOld = oldPoints[i],
	// dummyCurrent = nodes[i].getPosition();
	// for (int j = 0; j < dummyCurrent.length; j++) {
	// if (dummyOld[j] != dummyCurrent[j]) {
	// dummyOld[j] = dummyCurrent[j];
	// result = false;
	// }
	// }
	// }
	// if (!result) {
	// planeUpdated = false;
	// circumCenterUpdated = false;
	// normalVectorUpdated = false;
	// }
	// }

	/**
	 * Recomputes the center of the circumsphere of this triangle if any incident node moved since the last change.
	 */
	private void updateCircumCenterIfNecessary() {
		if (!circumCenterUpdated && !isInfinite()) {
			this.circumCenterUpdated = true;
			double[] a = nodes[0].getPosition();
			// Start by calculating the normal vectors:
			double[][] n = new double[3][];
			// n[0] = subtract(nodes[1].getPosition(), a);
			// n[1] = subtract(nodes[2].getPosition(), a);
			double[] line01 = subtract(nodes[1].getPosition(), a);
			double[] line02 = subtract(nodes[2].getPosition(), a);
			n[0] = normalize(line01);
			n[1] = normalize(line02);
			n[2] = crossProduct(n[0], n[1]);
			updateNormalVector(n[2]);
			// this.normalVector = n[2];
			normalVectorUpdated = true;
			// if (!normalVectorUpdated) {
			// n[2] = crossProduct(n[0], n[1]);
			// this.normalVector = n[2];
			// normalVectorUpdated = true;
			// } else
			// n[2] = this.normalVector;
			tolerance = dot(normalVector, normalVector) * 0.000000001;
			normalVectorUpdated = true;
			// cut the three planes:
			circumCenter = calculate3PlaneXPoint(n, new double[] {
					dot(add(a, nodes[1].getPosition()), n[0]) * 0.5,
					dot(add(a, nodes[2].getPosition()), n[1]) * 0.5,
					dot(a, n[2]) });
		}
	}

	/**
	 * Calculates the exact circumcenter for a triangle defined by three 
	 * position vectors and one normal vector.
	 * @param points The coordinates of the points of the triangle.
	 * @param normalVector A normal vector of the triangle.
	 * @return The center of the circumcircle around the given triangle.
	 */
	private static ExactVector calculateCircumCenterExact(ExactVector[] points,
			ExactVector normalVector) {
		ExactVector a = points[0];
		// Start by calculating the normal vectors:
		ExactVector[] n = new ExactVector[] { points[1].subtract(a),
				points[2].subtract(a), normalVector };
		return calculate3PlaneXPoint(n, new Rational[] {
				points[1].add(a).dotProduct(n[0]).divideBy(new Rational(2, 1)),
				points[2].add(a).dotProduct(n[1]).divideBy(new Rational(2, 1)),
				a.dotProduct(n[2]) }, ExactVector.det(n));

	}

	/**
	 * Updates the plane equation for this triangle if and incident node has moved since the last 
	 * update.
	 */
	public void updatePlaneEquationIfNecessary() {
		if (!planeUpdated && !isInfinite()) {
			if ((this.nodes[0].getId() == 6) && (this.nodes[1].getId() == 8)
					&& (this.nodes[2].getId() == 5)) {
				if (NewDelaunayTest.createOutput())
					NewDelaunayTest.out("Now!");
			}
			double[] node0Position = nodes[0].getPosition();
			initPlane(subtract(nodes[1].getPosition(), node0Position),
					subtract(nodes[2].getPosition(), node0Position),
					node0Position, false);
			planeUpdated = true;
		}

	}

	/**
	 * Updates all parameters of this triangle.
	 */
	public void update() {
		updateCircumCenterIfNecessary();
		updatePlaneEquationIfNecessary();
	}

	/**
	 * @return An array of three instances of <code>ExactVector</code> which contain the 
	 * coordinates of this triangle's endpoints as rational numbers.
	 */
	private ExactVector[] getExactPositionVectors() {
		return new ExactVector[] { new ExactVector(nodes[0].getPosition()),
				new ExactVector(nodes[1].getPosition()),
				new ExactVector(nodes[2].getPosition()) };
	}

	/**
	 * {@inheritDoc}
	 */
	public int orientationExact(double[] point1, double[] point2) {
		ExactVector[] points = getExactPositionVectors();
		ExactVector normalVector = points[1].subtract(points[0]).crossProduct(
				points[2].subtract(points[0]));
		Rational offset = normalVector.dotProduct(points[0]);
		return normalVector.dotProduct(new ExactVector(point1)).compareTo(
				offset)
				* normalVector.dotProduct(new ExactVector(point2)).compareTo(
						offset);
	}

	/**
	 * Computes the orientation of a point to the circumcircle of this triangle.
	 * This function does NOT test whether the given coordinate lies in the plane of this triangle.
	 * It only compares the distance of the point to the circumcenter with the radius of the 
	 * circumcircle.
	 * @param point The coordinate of interest.
	 * @return 1, if the distance of <code>point</code> is smaller than the radius of the circumcircle, 0, if it is equal and 1, if
	 * it is bigger. 
	 */
	public int circleOrientation(double[] point) {
		updateCircumCenterIfNecessary();
		double[] dummy = subtract(point, this.circumCenter);
		double squaredDistance = dot(dummy, dummy);
		double[] radial = subtract(this.nodes[0].getPosition(),
				this.circumCenter);
		double squaredRadius = dot(radial, radial);
		double tolerance = squaredRadius * 0.000000001;
		if (squaredDistance < squaredRadius + tolerance) {
			if (squaredDistance > squaredRadius - tolerance) {
				ExactVector[] points = getExactPositionVectors();
				ExactVector circumCenter = calculateCircumCenterExact(points,
						calculateExactNormalVector(points));
				Rational pointDistance = circumCenter.subtract(
						new ExactVector(point)).squaredLength();
				Rational squaredRadiusX = circumCenter.subtract(points[0])
						.squaredLength();
				return squaredRadiusX.compareTo(pointDistance);
			} else
				return 1;
		} else
			return -1;
	}

	/**
	 * Given a tetrahedron which is incident to this triangle, this function returns the second tetrahedron incident to 
	 * this triangle.
	 * @param incidentTetrahedron A tetrahedron incident to this triangle. 
	 * @return The tetrahedron opposite to <code>incidentTetrahedron</code> at this triangle.
	 * @throws RuntimeException if <code>incidentTetrahedron</code> is not incident to this triangle.
	 */
	public Tetrahedron<T> getOppositeTetrahedron(Tetrahedron<T> incidentTetrahedron) {
		if (adjacentTetrahedra[0] == incidentTetrahedron)
			return adjacentTetrahedra[1];
		else if (adjacentTetrahedra[1] == incidentTetrahedron)
			return adjacentTetrahedra[0];
		else
			throw new RuntimeException("Tetrahedron not known!");
	}

	/**
	 * Removes a given tetrahedron from the list of incident tetrahedra.
	 * @param tetrahedron A tetrahedron incident to this triangle.
	 */
	public void removeTetrahedron(Tetrahedron<T> tetrahedron) {
		if (adjacentTetrahedra[0] == tetrahedron)
			adjacentTetrahedra[0] = null;
		else
			adjacentTetrahedra[1] = null;
	}

	/**
	 * @return A reference to the array storing the three endpoints of this
	 * tetrahedron.
	 */
	protected SpaceNode<T>[] getNodes() {
		return nodes;
	}

	/**
	 * Adds an incident tetrahedron to this triangle. 
	 * @param tetrahedron A new tetrahedron which is incident to this triangle. 
	 */
	protected void addTetrahedron(Tetrahedron<T> tetrahedron/*
															 * , int
															 * positionInTetrahedron
															 */) {
		if (adjacentTetrahedra[0] == null) {
			adjacentTetrahedra[0] = tetrahedron;
			// this.positionsInTriangleLists[0] = positionInTetrahedron;
		} else {
			adjacentTetrahedra[1] = tetrahedron;
			// this.positionsInTriangleLists[1] = positionInTetrahedron;
		}
		this.connectionChecked = -1;
	}

	/**
	 * Returns whether this triangle has already been tested if it is locally Delaunay.
	 * This function is used in a run of {@link SpaceNode#restoreDelaunay()} to
	 * keep track of which triangles have already been tested.
	 * <p>If this triangle was not tested yet, it is immediately marked as being tested.  
	 * @param checkingIndex The unique identifier of the run of <code>restoreDelaunay</code>.
	 * @return <code>true</code>, iff this triangle has already been tested for the 
	 * Delaunay property. 
	 */
	protected boolean wasCheckedAlready(int checkingIndex) {
		if (checkingIndex == connectionChecked)
			return true;
		else {
			this.connectionChecked = checkingIndex;
			return false;
		}
	}

	/**
	 * Returns whether this triangle is incident to a given tetrahedron.
	 * @param tetrahedron a tetrahedron that might be incident to this triangle.
	 * @return <code>true</code>, iff the tetrahedron is incident to this triangle.
	 */
	protected boolean isAdjacentTo(Tetrahedron<T> tetrahedron) {
		return (this.adjacentTetrahedra[0] == tetrahedron)
				|| (this.adjacentTetrahedra[1] == tetrahedron);
	}

	/**
	 * Returns whether this triangle is incident to a given node.
	 * 
	 * @param node A node that might be incident to this triangle.
	 * @return <code>true</code>, iff the node is incident to this triangle.
	 */
	protected boolean isAdjacentTo(SpaceNode<T> node) {
		return (this.nodes[0] == node) || (this.nodes[1] == node)
				|| (this.nodes[2] == node);
	}

	/**
	 * Tests if this triangle is not incident to any tetrahedron.
	 * @return <code>true</code>, iff this triangle has no incident tetrahedra.
	 */
	protected boolean isCompletelyOpen() {
		return (adjacentTetrahedra[0] == null)
				&& (adjacentTetrahedra[1] == null);
	}

	/**
	 * Tests if this triangle is incident to two tetrahedra.
	 * @return <code>true</code>, iff this triangle has two incident tetrahedra.
	 */
	protected boolean isClosed() {
		return (adjacentTetrahedra[0] != null)
				&& (adjacentTetrahedra[1] != null);
	}

	/**
	 * Tests whether this triangle has an open side and whether a given coordinate
	 * and the incident tetrahedron lie on opposite sides of the triangle.
	 * @param point The coordinate of interest.
	 * @return <code>true</code>, if this triangle has an open side and
	 * if the given coordinate lies on the open side.
	 */
	public boolean isOpenToSide(double[] point) {
		if (adjacentTetrahedra[0] == null) {
			if (adjacentTetrahedra[1] == null)
				return true;
			else {
				if (adjacentTetrahedra[1].isInfinite())
					return true;
				return !(onSameSide(adjacentTetrahedra[1].getOppositeNode(this)
						.getPosition(), point));
			}
		} else if (adjacentTetrahedra[1] == null) {
			if (adjacentTetrahedra[0].isInfinite())
				return true;
			return !(onSameSide(adjacentTetrahedra[0].getOppositeNode(this)
					.getPosition(), point));
		} else
			return false;
	}

	/**
	 * Internal function that is called whenever the normal vector of this 
	 * triangle is changed.
	 * @param newNormalVector The new normal vector.
	 */
	protected void updateNormalVector(double[] newNormalVector) {
		this.normalVector = newNormalVector;
		this.offset = dot(normalVector, nodes[0].getPosition());
		this.normalVectorUpdated = true;
	}

	/**
	 * This function detects on which side of the plane defined by this 
	 * triangle a given point lies. This side is then defined to be the 
	 * upper side of this triangle.
	 * @param position A coordinate that defines the 'upper side' of this triangle
	 * 			A runtime exception is thrown if the given point lies in the plane
	 * 			defined by this triangle.
	 */
	public void orientToSide(double[] position) {
		if (!isInfinite()) {
			this.updatePlaneEquationIfNecessary();
			double dot = dot(position, this.normalVector);
			if (dot > offset + tolerance)
				upperSidePositive = true;
			else if (dot < offset - tolerance)
				upperSidePositive = false;
			else {
				ExactVector[] points = getExactPositionVectors();
				ExactVector normalVector = calculateExactNormalVector(points);
				Rational dot1 = normalVector.dotProduct(points[0]);
				Rational dot2 = normalVector.dotProduct(new ExactVector(
						position));
				int comparison = dot1.compareTo(dot2);
				if (comparison == 0)
					throw new RuntimeException("The triangle " + this
							+ " cannot be oriented to " + position
							+ " because that point lies in the plane!");
				upperSidePositive = comparison < 0;
			}
		}
	}

	/**
	 * This function determines whether this triangle has a single open side.
	 * If this is the case, the fourth point incident to the tetrahedron incident to this triangle 
	 * is defined to be on the lower side of the triangle, thereby defining the open side as the upper side.
	 * If this triangle has either two open sides or no open sides, this function throws a RuntimeException.
	 */
	public void orientToOpenSide() {
		if (!isInfinite()) {
			if (adjacentTetrahedra[0] == null) {
				if (adjacentTetrahedra[1] == null)
					throw new RuntimeException("The triangle " + this
							+ " has two open sides!");
				if (!adjacentTetrahedra[1].isInfinite()) {
					orientToSide(adjacentTetrahedra[1].getOppositeNode(this)
							.getPosition());
					upperSidePositive ^= true;
				} else
					NewDelaunayTest.out("orientToOpenSide");
			} else if (adjacentTetrahedra[1] == null) {
				if (!adjacentTetrahedra[0].isInfinite()) {
					orientToSide(adjacentTetrahedra[0].getOppositeNode(this)
							.getPosition());
					upperSidePositive ^= true;
				} else
					NewDelaunayTest.out("orientToOpenSide");
			} else
				throw new RuntimeException("The triangle " + this
						+ " has no open side!");
		}
	}

	/**
	 * Determines whether a given coordinate lies on the upper side of this triangle (which must be defined
	 * beforehand, using either {@link #orientToOpenSide()} or {@link #orientToSide(double[])}.
	 * @param point The coordinate of interest. 
	 * @return -1, if the coordinate lies on the lower side of the triangle, +1 if it lies on the upper
	 * 			side of the triangle an 0 if it lies in the plane.
	 */
	public int orientationToUpperSide(double[] point) {
		double dot = dot(point, this.normalVector);
		if (dot > offset + tolerance)
			return upperSidePositive ? 1 : -1;
		else if (dot < offset - tolerance)
			return upperSidePositive ? -1 : 1;
		else {
			ExactVector[] points = getExactPositionVectors();
			ExactVector normalVector = calculateExactNormalVector(points);
			Rational dot1 = normalVector.dotProduct(points[0]);
			Rational dot2 = normalVector.dotProduct(new ExactVector(point));
			if (dot1.equals(dot2))
				return 0;
			else {
				return ((dot1.compareTo(dot2) > 0) ^ upperSidePositive) ? 1
						: -1;
			}

		}
	}

	/**
	 * Determines whether a given coordinate lies on the upper side of this triangle.
	 * @param point The coordinate of interest.
	 * @return <code>true</code>, if the given point lies on the upper side of this triangle or in the plane
	 * defined by this triangle and <code>false</code> otherwise. 
	 * @see #orientToSide(double[])
	 * @see #orientToOpenSide()
	 */
	public boolean onUpperSide(double[] point) {
		return orientationToUpperSide(point) >= 0;
	}

	/**
	 * Determines whether a given coordinate lies truly on the upper side of this triangle.
	 * @param point The coordinate of interest.
	 * @return <code>true</code>, if the given point lies on the upper side of this triangle and <code>false</code> otherwise. 
	 * @see #orientToSide(double[])
	 * @see #orientToOpenSide()
	 */
	public boolean trulyOnUpperSide(double[] point) {
		return orientationToUpperSide(point) > 0;
	}

	/**
	 * Calculates an sd-distance which could typically be expected from points lying in a distance 
	 * similar to the distance between the points of this triangle. Used to compute tolerance values in
	 * {@link OpenTriangleOrganizer#triangulate()} but very imprecise! Needs to removed and replaced!
	 * @return An unreliable double value.
	 * 
	 */
	public double getTypicalSDDistance() {
		if (isInfinite())
			return Double.MAX_VALUE;
		else {
			double[] dummy = subtract(nodes[0].getPosition(), this.circumCenter);
			return norm(dummy) / norm(normalVector);
		}
	}

}
