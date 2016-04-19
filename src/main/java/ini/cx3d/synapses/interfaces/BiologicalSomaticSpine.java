package ini.cx3d.synapses.interfaces;

import ini.cx3d.SimStateSerializable;

/**
 * Created by lukas on 18.04.16.
 */
public interface BiologicalSomaticSpine extends SimStateSerializable {
    @Override
    ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

    ini.cx3d.synapses.interfaces.PhysicalSomaticSpine getPhysicalSomaticSpine();

    void setPhysicalSomaticSpine(ini.cx3d.synapses.interfaces.PhysicalSomaticSpine physicalSomaticSpine);
}
