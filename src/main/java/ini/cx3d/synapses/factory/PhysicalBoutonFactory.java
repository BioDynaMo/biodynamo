package ini.cx3d.synapses.factory;

import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.swig.biology.Excrescence;
import ini.cx3d.swig.biology.biology;
import ini.cx3d.synapses.interfaces.PhysicalBouton;

public class PhysicalBoutonFactory {
    static {
        ini.cx3d.swig.biology.Excrescence.setECM(ECMFacade.getInstance());
    }

    private static final boolean NATIVE = biology.useNativePhysicalBouton;
    public static final boolean DEBUG = false;

    public static PhysicalBouton create() {
        if(NATIVE) {
            PhysicalBouton pb =  ini.cx3d.swig.biology.PhysicalBouton.create();
            ini.cx3d.swig.biology.Excrescence.registerJavaObject((Excrescence) pb);
            ini.cx3d.swig.biology.PhysicalBouton.registerJavaObject((ini.cx3d.swig.biology.PhysicalBouton) pb);
            return pb;
        } else {
            return new ini.cx3d.synapses.PhysicalBouton();
        }
    }

    public static PhysicalBouton create(ini.cx3d.physics.interfaces.PhysicalObject po, double[] origin, double length) {
        if(NATIVE) {
            PhysicalBouton pb =  ini.cx3d.swig.biology.PhysicalBouton.create(po, origin, length);
            ini.cx3d.swig.biology.Excrescence.registerJavaObject((Excrescence) pb);
            ini.cx3d.swig.biology.PhysicalBouton.registerJavaObject((ini.cx3d.swig.biology.PhysicalBouton) pb);
            return pb;
        } else {
            return new ini.cx3d.synapses.PhysicalBouton(po, origin, length);
        }
    }
}
