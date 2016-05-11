package ini.cx3d.synapses.interfaces;

import ini.cx3d.SimStateSerializable;

/**
 * Created by lukas on 12.04.16.
 */
public interface BiologicalSpine extends SimStateSerializable {
    ini.cx3d.synapses.interfaces.PhysicalSpine getPhysicalSpine();

    void setPhysicalSpine(ini.cx3d.synapses.interfaces.PhysicalSpine physicalSpine);
}
