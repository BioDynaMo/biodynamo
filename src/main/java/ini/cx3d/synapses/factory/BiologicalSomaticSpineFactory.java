package ini.cx3d.synapses.factory;

import ini.cx3d.synapses.interfaces.BiologicalSomaticSpine;

public class BiologicalSomaticSpineFactory {
    public static BiologicalSomaticSpine create() {
        return ini.cx3d.swig.biology.BiologicalSomaticSpine.create();
    }
}
