package ini.cx3d.physics.interfaces;

import ini.cx3d.SimStateSerializable;

/**
 * Created by lukas on 08.03.16.
 */
public interface PhysicalBond extends SimStateSerializable {
	ini.cx3d.physics.interfaces.PhysicalObject getFirstPhysicalObject();

	ini.cx3d.physics.interfaces.PhysicalObject getSecondPhysicalObject();

	void setFirstPhysicalObject(ini.cx3d.physics.interfaces.PhysicalObject a);

	void setSecondPhysicalObject(ini.cx3d.physics.interfaces.PhysicalObject b);

	/** If false, the first PhysicalObject doesn't feel the influence of this PhysicalBond.*/
	boolean isHasEffectOnA();

	/** If false, the second PhysicalObject doesn't feel the influence of this PhysicalBond.*/
	boolean isHasEffectOnB();

	/** If false, the second PhysicalObject doesn't feel the influence of this PhysicalBond.*/
	void setHasEffectOnB(boolean hasEffectOnB);

	/** If true, allows the physical bond to "slide" from b to b's mother or daughter left,
	 * if b is a chain of PhysicalCylinders. It can be seen as the migration of a along b.*/
	boolean isSlidingAllowed();

	/**
	 * If true, allows the physical bond to "slide" from b to b's mother or daughter left,
	 * if b is a chain of PhysicalCylinders. It can be seen as the migration of a along b.
	 * @param slidingAllowed
	 */
	void setSlidingAllowed(boolean slidingAllowed);

	void exchangePhysicalObject(ini.cx3d.physics.interfaces.PhysicalObject oldPo, ini.cx3d.physics.interfaces.PhysicalObject newPo);

	void vanish();

	ini.cx3d.physics.interfaces.PhysicalObject getOppositePhysicalObject(ini.cx3d.physics.interfaces.PhysicalObject po);

	void setPositionOnObjectInLocalCoord(ini.cx3d.physics.interfaces.PhysicalObject po, double[] positionInLocalCoordinates);

	double[] getPositionOnObjectInLocalCoord(ini.cx3d.physics.interfaces.PhysicalObject po);

	/**
	 * Returns the force that this PhysicalBond is applying to a PhsicalObject.
	 * The function also returns the proportion of the mass that is applied to the
	 * proximal end (mother's point mass) in case of PhysicalCylinder.
	 * (For PhysicalSpheres, the value p is meaningless).
	 *
	 * @param po the PhysicalObject to which the force is being applied.
	 * @return [Fx,Fy,Fz,p] an array with the 3 force components and the proportion
	 * applied to the proximal end - in case of a PhysicalCylinder.
	 */
	double[] getForceOn(ini.cx3d.physics.interfaces.PhysicalObject po);

	/**
	 * Gets the location in absolute cartesian coord of the first insertion point (on a).
	 * (Only for graphical display).Raises a NullPointerException if a == null.
	 * @return x,y,z coord of the insertion point of one end
	 */
	double[] getFirstEndLocation();

	/**
	 * Gets the location in absolute cartesian coord of the first insertion point (on a).
	 * (Only for graphical display). Raises a NullPointerException if b == null.
	 * @return x,y,z coord of the insertion point of one end
	 */
	double[] getSecondEndLocation();

	/**
	 * @return the restingLength
	 */
	double getRestingLength();

	/**
	 * @param restingLength the restingLength to set
	 */
	void setRestingLength(double restingLength);

	/**
	 * @return the springConstant
	 */
	double getSpringConstant();

	/**
	 * @param springConstant the springConstant to set
	 */
	void setSpringConstant(double springConstant);

	/**
	 * @return the maxTension
	 */
	double getMaxTension();

	/**
	 * @param maxTension the maxTension to set
	 */
	void setMaxTension(double maxTension);

	/**
	 * @return the dumpingConstant
	 */
	double getDumpingConstant();

	/**
	 * @param dumpingConstant the dumpingConstant to set
	 */
	void setDumpingConstant(double dumpingConstant);
}
