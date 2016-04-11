package ini.cx3d.physics.interfaces;

import ini.cx3d.localBiology.interfaces.CellElement;

import java.util.AbstractSequentialList;

/**
 * Created by lukas on 17.03.16.
 */
public interface PhysicalSphere extends PhysicalNode, PhysicalObject {
	@Override
	ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

	double getInterObjectForceCoefficient();

	void setInterObjectForceCoefficient(double interObjectForceCoefficient);

	double getRotationalInertia();

	void setRotationalInertia(double rotationalInertia);

	/** returns true because this object is a PhysicalSphere */
	boolean isAPhysicalSphere();

	void movePointMass(double speed, double[] direction);

	/**
	 *
	 *
	 * @param daughterWhoAsks .
	 *
	 */
	double[] originOf(PhysicalObject daughterWhoAsks);

	/**
	 * A PhysicalSphere has no mother that could call, so this method is not used.
	 */
	double[] forceTransmittedFromDaugtherToMother(PhysicalObject motherWhoAsks);

	void removeDaugther(PhysicalObject daughterToRemove);

	void updateRelative(PhysicalObject oldRelative, PhysicalObject newRelative);

	/**
	 * @return the somaElement
	 */
	ini.cx3d.localBiology.interfaces.SomaElement getSomaElement();

	/**
	 * @param somaElement the somaElement to set
	 */
	void setSomaElement(ini.cx3d.localBiology.interfaces.SomaElement somaElement);

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

	@Override
	void updateDependentPhysicalVariables();

	/**
	 * Updates the concentration of substances, based on the volume of the object.
	 * Is usually called after change of the volume (and therefore we don't modify it here)
	 */
	void updateIntracellularConcentrations();

	/**
	 * Recompute volume after diameter has changed.
	 */
	void updateVolume();

	/* Recompute diameter, after volume has been changed and recompute then concentration of
         * IntracellularSubstances.*/
	void updateDiameter();

	/**
	 * Extension of a PhysicalCylinder as a daughter of this PhysicalSphere. The position on the sphere where
	 * the cylinder will be attached is specified in spherical coordinates with respect to the
	 * cx3d.cells Axis with two angles. The cylinder that is produced is specified by the object (usually
	 * a SomaElement) that calls this method. Indeed, unlike PhysicalCylinder.insertProximalCylinder() for instance,
	 * this method is called for biological reasons, and not for discretization purposes.
	 *
	 * @param phi the angle from the zAxis
	 * @param theta the angle from the xAxis around the zAxis
	 */
	PhysicalCylinder addNewPhysicalCylinder(double newLength, double phi, double theta);

	/**
	 * Division of the sphere into two spheres. The one in which the method is called becomes
	 * one the 1st daughter sphere (it keeps its Soma); a new PhysicalSphere is instantiated
	 * and becomes the 2nd daughter (and the Soma given as argument is attributed to it
	 * as CellElement). One can specify the relative size of the daughters (2nd/1st).
	 * In asymmetrical division the cx3d.cells that divides stays the progenitor, so the ratio is
	 * smaller than 1.
	 * @param vr ratio of the two volumes (vr = v2/v1)
	 * @param phi the angle from the zAxis (for the division axis)
	 * @param theta the angle from the xAxis around the zAxis (for the division axis)
	 * @return the other daughter (new sphere)
	 */
	PhysicalSphere divide(double vr, double phi, double theta);

	/**
	 * Tells if a sphere is in the detection range of an other sphere.
	 */
	boolean isInContactWithSphere(PhysicalSphere s);

	boolean isInContactWithCylinder(PhysicalCylinder c);

	@Override
	double[] getForceOn(PhysicalCylinder c);

	@Override
	double[] getForceOn(PhysicalSphere s);

	void runPhysics();

	double[] getAxis();

	/**
	 * @return the daughters
	 */
	AbstractSequentialList<ini.cx3d.physics.interfaces.PhysicalCylinder> getDaughters();

	@Override
	void runIntracellularDiffusion();

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
	 * Returns the position in spherical coordinates (r,phi,theta)
	 * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
	 * @param positionInLocalCoordinates
	 * @return
	 */
	double[] transformCoordinatesLocalToPolar(double[] positionInLocalCoordinates);

	/**
	 * Returns the position in the local coordinate system (xAxis, yXis, zAxis)
	 * of a point expressed in spherical coordinates (r,phi,theta).
	 * @param positionInPolarCoordinates
	 * @return
	 */
	double[] transformCoordinatesPolarToLocal(double[] positionInPolarCoordinates);

	@Override
	double[] transformCoordinatesPolarToGlobal(double[] positionInPolarCoordinates);

	@Override
	double[] transformCoordinatesGlobalToPolar(double[] positionInGlobalCoordinates);

	@Override
	double[] getUnitNormalVector(double[] positionInPolarCoordinates);

	@Override
	CellElement getCellElement();

	@Override
	boolean isRelative(PhysicalObject po);

	@Override
	double getLength();
}
