package ini.cx3d.synapses.interfaces;

import ini.cx3d.SimStateSerializable;

/**
 * Created by lukas on 15.04.16.
 */
public interface PhysicalBouton extends SimStateSerializable, Excrescence {
	@Override
	ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

	BiologicalBouton getBiologicalBouton();

	void setBiologicalBouton(BiologicalBouton biologicalBouton);

	@Override
	boolean synapseWith(Excrescence otherExcressence,
						boolean createPhysicalBond);

	// Roman: Method for making synapses directly on the soma
	boolean synapseWithSoma(Excrescence otherExcrescence,
							boolean createPhysicalBond);

	boolean synapseWithShaft(ini.cx3d.localBiology.interfaces.NeuriteElement otherNe, double maxDis,
							 int nrSegments, boolean createPhysicalBond);
}
