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

/**
 * During the flip algorithm, it can happen that tetrahedra with no volume are created. 
 * Since these have no volume and no circumsphere, they cannot simply be stored as normal
 * tetrahedra. This class extends the class {@link Tetrahedron} in such a way, that these 
 * problems are handled.
 * 
 * @author Dennis Goehlsdorf
 *
 * @param <T> The type of user objects associated with nodes in this triangulation.
 */
public class FlatTetrahedron<T> extends Tetrahedron<T> {
	/**
	 * Constructs a new flat tetrahedron from a given triangle and a fourth point.
	 * Missing triangles are created.
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
	public FlatTetrahedron(Triangle3D<T> oneTriangle,
			SpaceNode<T> fourthPoint, OpenTriangleOrganizer<T> org, CacheManager<T> cm) throws NodeLockedException, ManagedObjectDoesNotExistException {
		super(oneTriangle, fourthPoint, org, cm);
	}

	/**
	 * Creates a new flat tetrahedron from four triangles and four points.
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
	public FlatTetrahedron(Triangle3D<T> triangleA,
			Triangle3D<T> triangleB, Triangle3D<T> triangleC,
			Triangle3D<T> triangleD, SpaceNode<T> nodeA,
			SpaceNode<T> nodeB, SpaceNode<T> nodeC,
			SpaceNode<T> nodeD, CacheManager<T> cm) throws NodeLockedException, ManagedObjectDoesNotExistException {
		super(triangleA, triangleB, triangleC, triangleD,
				nodeA, nodeB, nodeC, nodeD, cm);
	}

	/**
	 * Updates the circumsphere of this tetrahedron. Since a flat tetrahedron does not
	 * have a real circumsphere, no work is performed in this function but the adjacent nodes are 
	 * informed about the movement of <code>movedNode</code>. 
	 * @param movedNode The node that was moved.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public void updateCirumSphereAfterNodeMovement(SpaceNode<T> movedNode) throws NodeLockedException, ManagedObjectDoesNotExistException {
		for (int i = 0; i < 4; i++) {
			if (getAdjacentNode(i) != movedNode) 
				getAdjacentTriangle(i).informAboutNodeMovement();
		}
	}

	/**
	 * Calculates the volume of this flat tetrahedron. Since the volume of a flat tetrahedron is
	 * 0, the volume is simpley set to that value.
	 */
	protected void calculateVolume() {
		this.volume = 0.0;
	}

	/**
	 * Sets all the cross section areas associated with the incident edges to 0.
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected void updateCrossSectionAreas() throws NodeLockedException, ManagedObjectDoesNotExistException {
		for (int i = 0; i < 6; i++) 
			changeCrossSection(i, 0.0);
	}
	
	
	/**
	 * Computes the properties of the circumsphere around this tetrahedron.
	 * Since a flat tetrahedron has no circumsphere, no work is performed in this function.
	 */
	public void calculateCircumSphere() {
	}
	
	/**
	 * Returns a string representation of this tetrahedron. The result of {@link Tetrahedron#toString()}
	 * is extended by adding a "_" to the left and the right side of a normal representation of
	 * a tetrahedron to indicate that this tetrahedron is flat.  
	 * @return A string representing this flat tetrahedron.
	 * @see Tetrahedron#toString()
	 */
	public String toString() {
		return "_"+ super.toString()+"_";
	}
	
	/**
	 * Returns whether this tetrahedron is flat or not.
	 * @return <code>true</code> in any case, because all instances of this class are flat tetrahedra.
	 */
	protected boolean isFlat() {
		return true;
	}
	
	/**
	 * Determines wether a given point lies inside or outside the circumsphere
	 * of this tetrahedron or lies on the surface of this sphere. For a flat tetrahedron,
	 * all points that do not lie in the same plane as the tetrahedron itself are defined
	 * to lie outside the circumsphere. For points which lie in the same plane
	 * as the tetrahedron,  the orientation is 1 if the point lies inside the circumcircle around
	 * any of the incident triangles, 0 if it lies on any circumcircle and -1 else.
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
//		Tetrahedron<T> innerTetrahedron = getAdjacentTetrahedron(0);
		getAdjacentTriangle(0).updatePlaneEquationIfNecessary();
		int orientation = getAdjacentTriangle(0).orientation(point, point);
		if (orientation == 0) {
			int memory = -1;
			for (int i = 0; i < 4; i++) {
				Triangle3D<T> adjacentTriangle = getAdjacentTriangle(i);
				if (adjacentTriangle != null) {
					int dummy = adjacentTriangle.circleOrientation(point);
					if (dummy == 1)
						return 1;
					else if (dummy == 0)
						memory = 0;
				}
			}
			return memory;
		}
		else return orientation;
	}

	/**
	 * {@inheritDoc}
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public boolean isTrulyInsideSphere(double[] point) throws NodeLockedException, ManagedObjectDoesNotExistException {
		return orientation(point) > 0;
	}
	
	/**
	 * {@inheritDoc}
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public boolean isInsideSphere(double[] point) throws NodeLockedException, ManagedObjectDoesNotExistException {
		return orientation(point) >= 0;
	}
	
	/**
	 * {@inheritDoc}
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	protected boolean isPointInConvexPosition(double[] point, int connectingTriangleNumber) throws NodeLockedException, ManagedObjectDoesNotExistException {
		Triangle3D<T> adjacentTriangle0 = getAdjacentTriangle(0);
		adjacentTriangle0.updatePlaneEquationIfNecessary();
		return adjacentTriangle0.orientation(point, point) == 0;
	}
	
	/**
	 * {@inheritDoc}
	 * @throws ManagedObjectDoesNotExistException 
	 * @throws NodeLockedException 
	 */
	public int isInConvexPosition(double[] point, int connectingTriangleNumber) throws NodeLockedException, ManagedObjectDoesNotExistException {
		Triangle3D<T> adjacentTriangle0 = getAdjacentTriangle(0);
		adjacentTriangle0.updatePlaneEquationIfNecessary();
		if (adjacentTriangle0.orientation(point, point) == 0)
			return 0;
		else return -1;
	}

	
	
}
