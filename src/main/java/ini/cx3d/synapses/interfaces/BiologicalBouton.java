package ini.cx3d.synapses.interfaces;

import ini.cx3d.SimStateSerializable;

/**
 * Created by lukas on 12.04.16.
 */
public interface BiologicalBouton extends SimStateSerializable {
    ini.cx3d.synapses.interfaces.PhysicalBouton getPhysicalBouton();

    void setPhysicalBouton(ini.cx3d.synapses.interfaces.PhysicalBouton physicalBouton);
}
