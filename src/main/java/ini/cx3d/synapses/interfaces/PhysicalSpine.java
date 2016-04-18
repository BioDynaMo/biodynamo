package ini.cx3d.synapses.interfaces;

import ini.cx3d.SimStateSerializable;

/**
 * Created by lukas on 15.04.16.
 */
public interface PhysicalSpine extends SimStateSerializable, Excrescence {
	@Override
	ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

	BiologicalSpine getBiologicalSpine();

	void setBiologicalSpine(BiologicalSpine biologicalSpine);

	@Override
	boolean synapseWith(Excrescence otherExcressence, boolean createPhysicalBond);

	boolean synapseWithSoma(Excrescence otherExcrescence,
							boolean creatPhysicalBond);

	boolean synapseWithShaft(ini.cx3d.localBiology.interfaces.NeuriteElement otherNe, double maxDis, int nrSegments,
							 boolean createPhysicalBond);
}
