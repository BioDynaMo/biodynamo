package ini.cx3d.synapses.factory;

import ini.cx3d.synapses.interfaces.BiologicalSpine;

public class BiologicalSpineFactory {
    public static BiologicalSpine create() {
        return ini.cx3d.swig.biology.BiologicalSpine.create();
    }
}
