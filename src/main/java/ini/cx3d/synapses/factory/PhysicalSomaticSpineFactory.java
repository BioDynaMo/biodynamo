package ini.cx3d.synapses.factory;

import ini.cx3d.simulations.ECM;
import ini.cx3d.swig.biology.Excrescence;
import ini.cx3d.swig.biology.biology;
import ini.cx3d.synapses.interfaces.PhysicalSomaticSpine;

public class PhysicalSomaticSpineFactory {
    static {
        Excrescence.setECM(ECM.getInstance());
    }

    private static final boolean NATIVE = biology.useNativePhysicalSomaticSpine;
    public static final boolean DEBUG = false;

    public static PhysicalSomaticSpine create() {
        if(NATIVE) {
            PhysicalSomaticSpine ps = ini.cx3d.swig.biology.PhysicalSomaticSpine.create();
            Excrescence.registerJavaObject((Excrescence) ps);
            ini.cx3d.swig.biology.PhysicalSomaticSpine.registerJavaObject((ini.cx3d.swig.biology.PhysicalSomaticSpine) ps);
            return ps;
        } else {
            return new ini.cx3d.synapses.PhysicalSomaticSpine();
        }
    }

    public static PhysicalSomaticSpine create(ini.cx3d.physics.interfaces.PhysicalObject po, double[] origin, double length) {
        if(NATIVE) {
            PhysicalSomaticSpine ps = ini.cx3d.swig.biology.PhysicalSomaticSpine.create(po, origin, length);
            Excrescence.registerJavaObject((Excrescence) ps);
            ini.cx3d.swig.biology.PhysicalSomaticSpine.registerJavaObject((ini.cx3d.swig.biology.PhysicalSomaticSpine) ps);
            return ps;
        } else {
            return new ini.cx3d.synapses.PhysicalSomaticSpine(po, origin, length);
        }
    }
}
