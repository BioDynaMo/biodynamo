package ini.cx3d.synapses.factory;

import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.swig.biology.Excrescence;
import ini.cx3d.swig.biology.biology;
import ini.cx3d.synapses.interfaces.PhysicalSpine;

public class PhysicalSpineFactory {
    static {
        ini.cx3d.swig.biology.Excrescence.setECM(ECMFacade.getInstance());
    }

    private static final boolean NATIVE = biology.useNativePhysicalSpine;
    public static final boolean DEBUG = false;

    public static PhysicalSpine create() {
        if(NATIVE) {
            PhysicalSpine ps = ini.cx3d.swig.biology.PhysicalSpine.create();
            ini.cx3d.swig.biology.Excrescence.registerJavaObject((Excrescence) ps);
            ini.cx3d.swig.biology.PhysicalSpine.registerJavaObject((ini.cx3d.swig.biology.PhysicalSpine) ps);
            return ps;
        } else {
            return new ini.cx3d.synapses.PhysicalSpine();
        }
    }

    public static PhysicalSpine create(ini.cx3d.physics.interfaces.PhysicalObject po, double[] origin, double length) {
        if(NATIVE) {
            PhysicalSpine ps = ini.cx3d.swig.biology.PhysicalSpine.create(po, origin, length);
            ini.cx3d.swig.biology.Excrescence.registerJavaObject((Excrescence) ps);
            ini.cx3d.swig.biology.PhysicalSpine.registerJavaObject((ini.cx3d.swig.biology.PhysicalSpine) ps);
            return ps;
        } else {
            return new ini.cx3d.synapses.PhysicalSpine(po, origin, length);
        }
    }
}
