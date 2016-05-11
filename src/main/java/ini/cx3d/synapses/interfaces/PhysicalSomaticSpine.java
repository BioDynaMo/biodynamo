package ini.cx3d.synapses.interfaces;

import ini.cx3d.SimStateSerializable;

/**
 * Created by lukas on 18.04.16.
 */
public interface PhysicalSomaticSpine extends Excrescence, SimStateSerializable {
	@Override
	ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

	BiologicalSomaticSpine getBiologicalSomaticSpine();

	void setBiologicalSomaticSpine(BiologicalSomaticSpine biologicalSomaticSpine);

	@Override
	boolean synapseWith(Excrescence otherExcressence, boolean createPhysicalBond);

	boolean synapseWithSoma(Excrescence otherExcrescence,
							boolean creatPhysicalBond);

	boolean synapseWithShaft(ini.cx3d.localBiology.interfaces.NeuriteElement otherNe, double maxDis, int nrSegments,
							 boolean createPhysicalBond);
}
