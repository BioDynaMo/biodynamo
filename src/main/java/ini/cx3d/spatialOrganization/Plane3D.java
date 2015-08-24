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
 * Used to represent a plane in three dimensional space. Here, a plane is fully defined by 
 * a normal vector and an offset value.
 * 
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of user objects associated with nodes in the current triangulation.
 */
public class Plane3D<T> {
	
	/**
	 * Used to define whether or not normal vectors should be normalized to unit length. 
	 */
	protected static final boolean NORMALIZE = true;
	
	/**
	 * The normal vector of this plane.
	 */
	double[] normalVector;
	
	/**
	 * The offset of this plane which is equal to the dot product of <code>normalVector</code>
	 * with any coordinate on the plane. 
	 */
	double offset = 0.0;
	
	/**
	 * Defines a tolerance intervall in which precise arithmetics are used.
	 */
	double tolerance = 0.0;

	/**
	 * A boolean to store whether or not the normal vector has changed. 
	 */
	boolean normalVectorUpdated = false;
	
	/**
	 * The default constructor. Only used to avoid errors in child classes. 
	 */
	Plane3D() {
	}
	
	/**
	 * Creates a plane from a given normal vector and a defined offset value.
	 * @param normalVector The normal vector of the plane.
	 * @param offset The offset of the plane.
	 */
	public Plane3D(double[] normalVector, double offset) {
		this.normalVector = normalVector;
		tolerance = dot(normalVector,normalVector)*0.000000001;
		this.offset = offset;
	}
	
	/**
	 * Creates a plane from two direction vectors and one vector 
	 * pointing to a position on the plane.
	 * @param directionVector1 The first direction vector. Must not be colinear to <code>directionVector2</code>.
	 * @param directionVector2 The second direction vector. Must not be colinear to <code>directionVector1</code>.
	 * @param positionVector The coordinate of a point on the plane.
	 * @param normalize Defines whether or not the normal vector of this plane should be normalized or not.
	 */
	public Plane3D(double[] directionVector1, double[] directionVector2, double[] positionVector, boolean normalize) {
		initPlane(directionVector1,directionVector2,positionVector,normalize);
	}
	
	/** 
	 * Creates a plane from two direction vectors and one vector 
	 * pointing to a position on the plane. The normal vector of the resulting plane will be 
	 * normalized if {@link #NORMALIZE} is set to <code>true</code>.
	 * @param directionVector1 The first direction vector. Must not be colinear to <code>directionVector2</code>.
	 * @param directionVector2 The second direction vector. Must not be colinear to <code>directionVector1</code>.
	 * @param positionVector The coordinate of a point on the plane.
	 */
	public Plane3D(double[] directionVector1, double[] directionVector2, double[] positionVector) {
		initPlane(directionVector1,directionVector2,positionVector,NORMALIZE);
	}
	
	/**
	 * Creates a plane from a set of nodes. 
	 * @param nodes An array of nodes which is expected to be of length 4. Three nodes from these 4 are used to 
	 * create the plane. <code>nonUsedNode</code> defines, which node will not be used.
	 * @param nonUsedNode The node in <code>nodes</code> which will not become part of the newly created plane.
	 * @param normalize Defines whether or not the normal vector of this plane should be normalized or not.
	 */
	public Plane3D(SpaceNode<T>[] nodes, SpaceNode<T> nonUsedNode, boolean normalize) {
		int first = (nodes[0] == nonUsedNode)?1:0;
		initPlane(
				subtract(
						nodes[first].getPosition(),
						(first == 0) ? ((nodes[1] != nonUsedNode) ? (nodes[1]
								.getPosition())
								: nodes[2].getPosition())
								: nodes[2].getPosition()),
				subtract(
						nodes[first].getPosition(),
						(nodes[3] == nonUsedNode) ? nodes[2]
								.getPosition()
								: nodes[3].getPosition()),
				nodes[first].getPosition(), normalize);
		defineUpperSide(nonUsedNode.getPosition());
	}

	/**
	 * Creates a plane from a set of nodes. The normal vector of the resulting plane will be 
	 * normalized if {@link #NORMALIZE} is set to <code>true</code>.
	 * @param nodes An array of nodes which is expected to be of length 4. Three nodes from these 4 are used to 
	 * create the plane. <code>nonUsedNode</code> defines, which node will not be used.
	 * @param nonUsedNode The node in <code>nodes</code> which will not become part of the newly created plane.
	 */
	public Plane3D(SpaceNode<T>[] nodes, SpaceNode<T> nonUsedNode) {
		this(nodes,nonUsedNode,NORMALIZE);
	}

	/**
	 * Creates a plane from the endpoint of a tetrahedron. The normal vector of the resulting plane will be 
	 * normalized if {@link #NORMALIZE} is set to <code>true</code>.
	 * @param tetra The tetrahedron which should be used to create an new plane. Three of the endpoints of
	 * <code>tetra</code> will be used for the new plane/ 
	 * @param nonUsedNode The node in <code>nodes</code> which will not become part of the newly created plane.
	 */
	public Plane3D(Tetrahedron<T> tetra, SpaceNode<T> nonUsedNode) {
		this(tetra.getAdjacentNodes(),nonUsedNode);
	}
	
	/**
	 * Initializes the plane by computing a normal vector and an offset value from
	 * two direction vectors and one position vector. 
	 * @param directionVector1 The first direction vector. Must not be colinear to <code>directionVector2</code>.
	 * @param directionVector2 The second direction vector. Must not be colinear to <code>directionVector1</code>.
	 * @param positionVector The coordinate of a point on the plane.
	 * @param normalize Defines whether or not the normal vector of this plane should be normalized or not.
	 */
	void initPlane(double[] directionVector1, double[] directionVector2, double[] positionVector, boolean normalize) {
		if (!normalVectorUpdated) {
			normalVectorUpdated = true;
			this.normalVector = crossProduct(directionVector1, directionVector2);
			tolerance = dot(normalVector,normalVector)*0.000000001;
			if (tolerance == 0.0)
				throw new RuntimeException("tolerance was set to 0!");
		}
		if (normalize) {
			double norm = norm(this.normalVector);
			this.normalVector[0] /= norm;
			this.normalVector[1] /= norm;
			this.normalVector[2] /= norm;
			tolerance = 0.000000001;
		}
		this.offset = dot(normalVector, positionVector); 
	}
	
	/**
	 * Reverts the orientation of this plane by switching the sign of all entries 
	 * in the normal vector and the offset value.
	 */
	public void changeUpperSide() {
		offset = -offset;
		normalVector[0] = -normalVector[0];
		normalVector[1] = -normalVector[1];
		normalVector[2] = -normalVector[2];
	}
	
	/**
	 * Defines the upper side of this plane. The upper side is defined to be the one side
	 * to which the normal vector points to.
	 * @param point A coordinate on the upper side of the plane.
	 */
	public void defineUpperSide(double[] point) {
		if (dot(point,this.normalVector)+tolerance < offset) 
			changeUpperSide();
	}
	
//	public boolean inThePlane(double[] point) {
//		return (Math.abs(dot(point,this.normalVector)-offset) <= tolerance);
//	}
	
	/**
	 * Computes the orientation of two coordinates relative this plane.
	 * This function uses precise arithmetics to assure the result.
	 * @param point1 The first coordinate.
	 * @param point2 The second coordinate.
	 * @return 1, if both points lie on the same side of the plane, -1, if the points
	 * lie on opposite sides of the plane and 0, if either one of the points lies
	 * on the plane.
	 */
	protected int orientationExact(double[] point1, double[] point2) {
		ExactVector normalVector = new ExactVector(this.normalVector);
		Rational offset = new Rational(this.offset);
		return normalVector.dotProduct(new ExactVector(point1)).compareTo(offset) * 
		       normalVector.dotProduct(new ExactVector(point2)).compareTo(offset);
	}
	
	/**
	 * Computes the orientation of two coordinates relative this plane. 
	 * @param point1 The first coordinate.
	 * @param point2 The second coordinate.
	 * @return 1, if both points lie on the same side of the plane, -1, if the points
	 * lie on opposite sides of the plane and 0, if either one of the points lies
	 * on the plane.
	 */
	public int orientation(double[]point1, double[] point2) {
		double dot1 = dot(point1,this.normalVector);
		double dot2 = dot(point2,this.normalVector);
		if (dot1 > offset+tolerance) {
			if (dot2 < offset-tolerance) return -1;
			else if (dot2 > offset+tolerance) 
				return 1;
			else return orientationExact(point1, point2);
		}
		else if (dot1 < offset-tolerance) {
			if (dot2 > offset+tolerance) return -1;
			else if (dot2 < offset-tolerance) return 1;
			else return orientationExact(point1, point2);
		}
		else 
			return orientationExact(point1, point2); 

	}
	
	/**
	 * Returns whether or not two points lie on the same side of this plane.
	 * 
	 * @param point1 The first point.
	 * @param point2 The second point.
	 * @return <code>true</code>, if both points lie on the same side of the
	 *         plane and <code>false</code>, if they don't or if one of them
	 *         lies in the plane.
	 */
	public boolean trulyOnSameSide(double[] point1, double[] point2) {
		return orientation(point1, point2) > 0;
	}

	/**
	 * Returns whether or not two points lie on different sides of this plane.
	 * 
	 * @param point1 The first point.
	 * @param point2 The second point.
	 * @return <code>true</code>, if both points lie on different sides of the
	 *         plane and <code>false</code>, if they don't or if one of them
	 *         lies in the plane.
	 */
	public boolean trulyOnDifferentSides(double[] point1, double[] point2) {
		return orientation(point1, point2) < 0;
	}

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
	public boolean onSameSide(double[] point1, double[] point2) {
		return orientation(point1, point2) >= 0;
	}
	
	/**
	 * @return The normal vector of this plane.
	 */
	public double[] getNormalVector() {
		return normalVector;
	}
	
}
