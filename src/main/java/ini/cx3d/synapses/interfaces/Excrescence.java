package ini.cx3d.synapses.interfaces;

import ini.cx3d.SimStateSerializable;

public interface Excrescence extends SimStateSerializable {
	public static final int SPINE = 0;
	public static final int BOUTON = 1;
	public static final int SOMATICSPINE = 2;
	public static final int SHAFT = 3;

	/**
	 * Method to create a spine-bouton synapse.
	 * @param otherExcrescence the other spine/bouton
	 * @param createPhysicalBond is true, a PhysicalBond is made between the two respective
	 * PhysicalObjects possessing the Excrescences.
	 *
	 * @return true if the synapse was performed correctly
	 */
	boolean synapseWith(Excrescence otherExcrescence, boolean createPhysicalBond);

	// Inserted by roman. This modifications allow additionally making synapses with somatic spines and with dendritic shafts.
	boolean synapseWithSoma(Excrescence otherExcrescence, boolean createPhysicalBond);

	boolean synapseWithShaft(ini.cx3d.localBiology.interfaces.NeuriteElement otherNe, double maxDis, int nrSegments, boolean createPhysicalBond);

	// dumb getters and setters .......................
	Excrescence getEx();

	void setEx(Excrescence ex);

	double getLength();

	void setLength(double length);

	ini.cx3d.physics.interfaces.PhysicalObject getPo();

	void setPo(ini.cx3d.physics.interfaces.PhysicalObject po);

	double[] getPositionOnPO();

	void setPositionOnPO(double[] positionOnPO);

	int getType();

	void setType(int type);

	/** returns the absolute coord of the point where this element is attached on the PO.*/
	double[] getProximalEnd();

	/** returns the absolute coord of the point where this element ends.*/
	double[] getDistalEnd();
}
