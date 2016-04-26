package ini.cx3d.synapses.factory;

import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.swig.simulation.Excrescence;
import ini.cx3d.swig.simulation.simulation;
import ini.cx3d.synapses.interfaces.PhysicalSpine;

public class PhysicalSpineFactory {
    static {
        ini.cx3d.swig.simulation.Excrescence.setECM(ECMFacade.getInstance());
    }

    private static final boolean NATIVE = simulation.useNativePhysicalSpine;
    public static final boolean DEBUG = false;

    public static PhysicalSpine create() {
        if(NATIVE) {
            PhysicalSpine ps = ini.cx3d.swig.simulation.PhysicalSpine.create();
            ini.cx3d.swig.simulation.Excrescence.registerJavaObject((Excrescence) ps);
            ini.cx3d.swig.simulation.PhysicalSpine.registerJavaObject((ini.cx3d.swig.simulation.PhysicalSpine) ps);
            return ps;
        } else {
            return new ini.cx3d.synapses.PhysicalSpine();
        }
    }

    public static PhysicalSpine create(ini.cx3d.physics.interfaces.PhysicalObject po, double[] origin, double length) {
        if(NATIVE) {
            PhysicalSpine ps = ini.cx3d.swig.simulation.PhysicalSpine.create(po, origin, length);
            ini.cx3d.swig.simulation.Excrescence.registerJavaObject((Excrescence) ps);
            ini.cx3d.swig.simulation.PhysicalSpine.registerJavaObject((ini.cx3d.swig.simulation.PhysicalSpine) ps);
            return ps;
        } else {
            return new ini.cx3d.synapses.PhysicalSpine(po, origin, length);
        }
    }
}
