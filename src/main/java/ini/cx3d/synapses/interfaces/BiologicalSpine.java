package ini.cx3d.synapses.interfaces;

import ini.cx3d.SimStateSerializable;
import ini.cx3d.synapses.PhysicalSpine;

/**
 * Created by lukas on 12.04.16.
 */
public interface BiologicalSpine extends SimStateSerializable {
    PhysicalSpine getPhysicalSpine();

    void setPhysicalSpine(PhysicalSpine physicalSpine);
}
