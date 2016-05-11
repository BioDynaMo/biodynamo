package ini.cx3d.localBiology.interfaces;

import java.util.AbstractSequentialList;

/**
 * Created by lukas on 11.04.16.
 */
public interface SomaElement extends CellElement {
	SomaElement divide(double volumeRatio, double phi, double theta);

	void run();

	ini.cx3d.localBiology.interfaces.NeuriteElement extendNewNeurite();

	/**
	 * Extends a new neurite at a random place on the sphere
	 * @param diameter the diameter of the new neurite
	 * @param phi the angle from the zAxis
	 * @param theta the angle from the xAxis around the zAxis
	 * @return
	 */
	ini.cx3d.localBiology.interfaces.NeuriteElement extendNewNeurite(double diameter);

	ini.cx3d.localBiology.interfaces.NeuriteElement extendNewNeurite(double[] directionInGlobalCoordinates);

	ini.cx3d.localBiology.interfaces.NeuriteElement extendNewNeurite(double diameter, double[] directionInGlobalCoordinates);

	/**
	 * Extends a new neurites
	 * @param diameter the diameter of the new neurite
	 * @param phi the angle from the zAxis
	 * @param theta the angle from the xAxis around the zAxis
	 * @return
	 */
	ini.cx3d.localBiology.interfaces.NeuriteElement extendNewNeurite(double diameter, double phi, double theta);

	ini.cx3d.physics.interfaces.PhysicalObject getPhysical();

	void setPhysical(ini.cx3d.physics.interfaces.PhysicalObject physical);

	ini.cx3d.physics.interfaces.PhysicalSphere getPhysicalSphere();

	void setPhysicalSphere(ini.cx3d.physics.interfaces.PhysicalSphere physicalsphere);

	AbstractSequentialList<ini.cx3d.localBiology.interfaces.NeuriteElement> getNeuriteList();

	boolean isANeuriteElement();

	/** Returns true, because this <code>CellElement</code> is a <code>SomaElement</code>.*/
	boolean isASomaElement();
}
