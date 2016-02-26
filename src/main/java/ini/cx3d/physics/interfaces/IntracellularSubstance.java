package ini.cx3d.physics.interfaces;

/**
 * Created by lukas on 25.02.16.
 */
public interface IntracellularSubstance extends Substance {
	/**
	 * Distribute IntracellularSubstance concentration at division and update quantity.
	 * @param newIS
	 */
	void distributeConcentrationOnDivision(IntracellularSubstance newIS);

	/**
	 * Degradation of the <code>IntracellularSubstance</code>.
	 * @param newIS
	 */
	void degrade();

	/** Returns the degree of asymmetric distribution during cell division.
	 * 0 represents complete symmetrical division, 1 represents complete asymmetric division. */
	double getAsymmetryConstant();

	/** Sets the degree of asymmetric distribution during cell division.
	 * 0 represents complete symmetrical division, 1 represents complete asymmetric division.
	 * The sign + or - is used to distinguish between one daughter (mother cell) and the other
	 * (new cell). */
	void setAsymmetryConstant(double asymmetryConstant);

	/** If true, the Substance can be detected from outside of the PhysicalObject
	 * (equivalent to an membrane bound substance).*/
	boolean isVisibleFromOutside();

	/** If true, the Substance can be detected from outside of the PhysicalObject
	 * (equivalent to an membrane bound substance).*/
	void setVisibleFromOutside(boolean visibleFromOutside);

	/** If true, the volume is taken into account for computing the concentration,
	 * otherwise a virtual volume corresponding to the length of the physical object
	 * (with virtual radius 1) is used.*/
	boolean isVolumeDependant();

	/** If true, the volume is taken into account for computing the concentration,
	 * otherwise a virtual volume corresponding to the length of the physical object
	 * (with virtual radius 1) is used.*/
	void setVolumeDependant(boolean volumeDependant);
}
