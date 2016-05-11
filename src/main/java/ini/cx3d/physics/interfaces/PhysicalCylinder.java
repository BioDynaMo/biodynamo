package ini.cx3d.physics.interfaces;

import ini.cx3d.localBiology.interfaces.CellElement;
import ini.cx3d.localBiology.interfaces.NeuriteElement;

/**
 * Created by lukas on 22.03.16.
 */
public interface PhysicalCylinder extends PhysicalObject {
	ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

	/** Returns a <code>PhysicalCylinder</code> with all fields similar than in this
	 * <code>PhysicalCylinder</code>. Note that the relatives in the tree structure, the
	 * tension, the volume, the
	 * <code>CellElement</code>, as well as<code>Excrescences</code> and the
	 * <code>IntracellularSubstances</code> are not copied. */
	PhysicalCylinder getCopy();

	/**
	 * Returns true if the <code>PhysicalObject</code> given as argument is a mother, daughter
	 * or sister branch.*/
	boolean isRelative(PhysicalObject po);

	@Override
	double[] getForceOn(PhysicalCylinder c);

	/**
	 * Returns the location in absolute coordinates of where the <code>PhysicalObject</code>
	 * given as argument is attached on this where the <code>PhysicalCylinder</code>
	 * If the argument is one of our daughter <code>PhysicalCylinder</code>, the point mass location
	 * is returned. Otherwise, the return is <code>null</code>.
	 *
	 * @param daughterWhoAsks the PhysicalObject requesting it's origin.
	 *
	 */
	double[] originOf(PhysicalObject daughterWhoAsks);

	void removeDaugther(PhysicalObject daughterToRemove);

	void updateRelative(PhysicalObject oldRelative, PhysicalObject newRelative);

	/**
	 * returns the total force that this <code>PhysicalCylinder</code> exerts on it's mother.
	 * It is the sum of the spring force an the part of the inter-object force computed earlier in
	 * <code>runPhysics()</code>.
	 */
	double[] forceTransmittedFromDaugtherToMother(PhysicalObject motherWhoAsks);

	/**
	 * Checks if this <code>PhysicalCylinder</code> is either too long (and in this case it will insert
	 * another <code>PhysicalCylinder</code>), or too short (and in this second case fuse it with the
	 * proximal element or even delete it).
	 * */
	boolean runDiscretization();

	/** Method used for active extension of a terminal branch, representing the steering of a
	 * growth cone. The movement should always be forward, otherwise no movement is performed.
	 *
	 * @param speed of the growth rate (microns/hours).
	 * @direction the 3D direction of movement.
	 */
	void extendCylinder(double speed, double[] direction);

	/** Method used for active extension of a terminal branch, representing the steering of a
	 * growth cone. There is no check for real extension (unlike in extendCylinder() ).
	 *
	 * @param speed of the growth rate (microns/hours).
	 * @direction the 3D direction of movement.
	 */
	void movePointMass(double speed, double[] direction);

	/**
	 * Branch retraction by moving the distal end (i.e. the massLocation) toward the
	 * proximal end (the mother), maintaining the same tension in the PhysicalCylinder. The method
	 * shortens the actual and the resting length so that the result is a shorter
	 * cylinder with the same tension.
	 * - If this PhysicalCylinder is longer than the required shortening, it simply retracts.
	 * - If it is shorter and its mother has no other daughter, it merges with it's mother and
	 * the method is recursively called (this time the cylinder length is bigger because we have
	 * a new PhysicalCylinder that resulted from the fusion of two).
	 * - If it is shorter and either the previous PhysicalCylinder has another daughter or the
	 * mother is not a PhysicalCylinder, it disappears.
	 *
	 * @param speed of the retraction (microns/hours).
	 * @return false if the neurite doesn't exist anymore (complete retraction)
	 */
	boolean retractCylinder(double speed);

	/**
	 * Bifurcation of the growth cone creating : adds the 2 <code>PhysicalCylinder</code> that become
	 * daughter left and daughter right
	 * @param length the length of the new branches
	 * @param direction_1 of the first branch (if
	 * @param direction_2
	 */

	PhysicalCylinder[] bifurcateCylinder(double length, double[] direction_1, double[] direction_2);

	/**
	 * Makes a side branching by adding a second daughter to a non terminal <code>PhysicalCylinder</code>.
	 * The new <code>PhysicalCylinder</code> is perpendicular to the mother branch.
	 * @param direction the direction of the new neuriteLement (But will be automatically corrected if
	 * not al least 45 degrees from the cylinder's axis).
	 * @return the newly added <code>NeuriteSegment</code>
	 */
	PhysicalCylinder branchCylinder(double length, double[] direction);

	void setRestingLengthForDesiredTension(double tensionWeWant);

	/**
	 * Progressive modification of the volume. Updates the diameter, the intracellular concentration
	 * @param speed cubic micron/ h
	 */
	void changeVolume(double speed);

	/**
	 * Progressive modification of the diameter. Updates the volume, the intracellular concentration
	 * @param speed micron/ h
	 */
	void changeDiameter(double speed);

	/**
	 *
	 */
	void runPhysics();

	double[] getForceOn(PhysicalSphere s);

	boolean isInContactWithSphere(PhysicalSphere s);

	boolean isInContactWithCylinder(PhysicalCylinder c);

	/** Returns the point on this cylinder's spring axis that is the closest to the point p.*/
	double[] closestPointTo(double[] p);

	void runIntracellularDiffusion();

	double[] getUnitNormalVector(double[] positionInPolarCoordinates);

	/**
	 * Defines the three orthonormal local axis so that a cylindrical coordinate system
	 * can be used. The xAxis is aligned with the springAxis. The two other are in the
	 * plane perpendicular to springAxis. This method to update the axis was suggested by
	 * Matt Coock. - Although not perfectly exact, it is accurate enough for us to use.
	 *
	 */
	void updateLocalCoordinateAxis();

	/**
	 * Updates the spring axis, the actual length, the tension and the volume.
	 *
	 * For tension, T = k*(aL-rL)/rL.  k = spring constant,
	 * rL = resting length, aL = actual length. (Note the division by rL.
	 * Otherwise we could have Cylinders with big aL and rL = 0).
	 * <p>
	 * This method also automatically calls the <code>resetComputationCenterPosition()</code>
	 * method at the end.
	 */
	void updateDependentPhysicalVariables();

	/* Recomputes diameter after volume has changed.*/
	void updateDiameter();

	/* Recomputes volume, after diameter has been change. And makes a call for
         * recomputing then concentration of IntracellularSubstances.*/
	void updateVolume();

	/**
	 * Updates the concentration of substances, based on the volume of the object.
	 * Is usually called after change of the volume (and therefore we don't modify it here)
	 */
	void updateIntracellularConcentrations();

	/**
	 * Returns the position in the local coordinate system (xAxis, yXis, zAxis)
	 * of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
	 * @param positionInGlobalCoord
	 * @return
	 */
	double[] transformCoordinatesGlobalToLocal(double[] positionInGlobalCoord);

	/**
	 * Returns the position in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1])
	 * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
	 * @param positionInLocalCoord
	 * @return
	 */
	double[] transformCoordinatesLocalToGlobal(double[] positionInLocalCoord);

	/**
	 * Returns the position in cylindrical coordinates (h,theta,r)
	 * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
	 * @param positionInLocalCoordinates
	 * @return
	 */
	double[] transformCoordinatesLocalToPolar(double[] positionInLocalCoordinates);

	/**
	 * Returns the position in the local coordinate system (xAxis, yXis, zAxis)
	 * of a point expressed in cylindrical coordinates (h,theta,r).
	 * @param positionInPolarCoordinates
	 * @return
	 */
	double[] transformCoordinatesPolarToLocal(double[] positionInPolarCoordinates);

	// P -> G :    P -> L, then L -> G
	double[] transformCoordinatesPolarToGlobal(double[] positionInPolarCoordinates);

	// G -> L :    G -> L, then L -> P
	double[] transformCoordinatesGlobalToPolar(double[] positionInGlobalCoordinates);

	/** Well, there is no field cellElement. We return neuriteElement.*/
	CellElement getCellElement();

	/**
	 * @return the neuriteElement
	 */
	NeuriteElement getNeuriteElement();

	/**
	 * @param neuriteElement the neuriteElement to set
	 */
	void setNeuriteElement(NeuriteElement neuriteElement);

	/**
	 * @return the daughterLeft
	 */
	PhysicalCylinder getDaughterLeft();

	/**
	 * @return the daughterRight
	 */
	PhysicalCylinder getDaughterRight();

	/**
	 * @return the mother
	 */
	PhysicalObject getMother();

	/**
	 * @param mother the mother to set
	 */
	void setMother(PhysicalObject mother);

	/**
	 * @param daughterLeft the daughterLeft to set
	 */
	void setDaughterLeft(PhysicalCylinder daughterLeft);

	/**
	 * @param daughterRight the daughterRight to set
	 */
	void setDaughterRight(PhysicalCylinder daughterRight);

	/**
	 * @param branchOrder the branchOrder to set
	 */
	void setBranchOrder(int branchOrder);

	/**
	 * @return the branchOrder
	 */
	int getBranchOrder();

	double getActualLength();

	/**
	 * Should not be used, since the actual length depends on the geometry.
	 * @param actualLength
	 */
	void setActualLength(double actualLength);

	double getRestingLength();

	void setRestingLength(double restingLength);

	double[] getSpringAxis();

	void setSpringAxis(double[] springAxis);

	double getSpringConstant();

	void setSpringConstant(double springConstant);

	double getTension();

	/**
	 * Gets a vector of length 1, with the same direction as the SpringAxis.
	 * @return a normalized springAxis
	 */
	// NOT A "REAL" GETTER
	double[] getUnitaryAxisDirectionVector();

	/** Should return yes if the PhysicalCylinder is considered a terminal branch.
	 * @return is it a terminal branch
	 */
	boolean isTerminal();

	/**
	 * Returns true if a bifurcation is physicaly possible. That is if the PhysicalCylinder
	 * has no daughter and the actual length is bigger than the minimum required.
	 * @return
	 */
	boolean bifurcationPermitted();

	/**
	 * Returns true if a side branch is physically possible. That is if this is not a terminal
	 * branch and if there is not already a second daughter.
	 * @return
	 */
	boolean branchPermitted();

	/**
	 * retuns the position of the proximal end, ie the massLocation minus the spring axis.
	 * Is mainly used for paint
	 * @return
	 */
	double[] proximalEnd();

	/**
	 * retuns the position of the distal end, ie the massLocation coordinates (but not the
	 * actual massLocation array).
	 * Is mainly used for paint
	 * @return
	 */
	double[] distalEnd();

	/**
	 * Returns the total (actual) length of all the cylinders (including the one in which this method is
	 * called) before the previous branching point. Used to decide if long enough to bifurcate or branch,
	 * independently of the discretization.
	 * @return
	 */
	double lengthToProximalBranchingPoint();

	/** returns true because this object is a PhysicalCylinder */
	boolean isAPhysicalCylinder();

	double getLength();

	double getInterObjectForceCoefficient();

	void setInterObjectForceCoefficient(
			double interObjectForceCoefficient);

	double[] getAxis();
}
