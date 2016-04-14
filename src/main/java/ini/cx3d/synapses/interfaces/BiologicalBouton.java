package ini.cx3d.synapses.interfaces;

import ini.cx3d.SimStateSerializable;
import ini.cx3d.synapses.PhysicalBouton;

/**
 * Created by lukas on 12.04.16.
 */
public interface BiologicalBouton extends SimStateSerializable {
    PhysicalBouton getPhysicalBouton();

    void setPhysicalBouton(PhysicalBouton physicalBouton);
}
