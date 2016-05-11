package ini.cx3d.synapses.factory;

import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.swig.simulation.Excrescence;
import ini.cx3d.swig.simulation.simulation;
import ini.cx3d.synapses.interfaces.PhysicalSomaticSpine;

public class PhysicalSomaticSpineFactory {
    static {
        Excrescence.setECM(ECMFacade.getInstance());
    }

    private static final boolean NATIVE = simulation.useNativePhysicalSomaticSpine;
    public static final boolean DEBUG = false;

    public static PhysicalSomaticSpine create() {
        if(NATIVE) {
            PhysicalSomaticSpine ps = ini.cx3d.swig.simulation.PhysicalSomaticSpine.create();
            Excrescence.registerJavaObject((Excrescence) ps);
            ini.cx3d.swig.simulation.PhysicalSomaticSpine.registerJavaObject((ini.cx3d.swig.simulation.PhysicalSomaticSpine) ps);
            return ps;
        } else {
            return new ini.cx3d.synapses.PhysicalSomaticSpine();
        }
    }

    public static PhysicalSomaticSpine create(ini.cx3d.physics.interfaces.PhysicalObject po, double[] origin, double length) {
        if(NATIVE) {
            PhysicalSomaticSpine ps = ini.cx3d.swig.simulation.PhysicalSomaticSpine.create(po, origin, length);
            Excrescence.registerJavaObject((Excrescence) ps);
            ini.cx3d.swig.simulation.PhysicalSomaticSpine.registerJavaObject((ini.cx3d.swig.simulation.PhysicalSomaticSpine) ps);
            return ps;
        } else {
            return new ini.cx3d.synapses.PhysicalSomaticSpine(po, origin, length);
        }
    }
}
