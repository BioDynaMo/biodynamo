package ini.cx3d.synapses.factory;

import ini.cx3d.synapses.interfaces.BiologicalBouton;

public class BiologicalBoutonFactory {
    public static BiologicalBouton create() {
        return ini.cx3d.swig.simulation.BiologicalBouton.create();
    }
}
