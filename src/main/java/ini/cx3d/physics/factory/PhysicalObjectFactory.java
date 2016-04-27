package ini.cx3d.physics.factory;

import ini.cx3d.physics.InterObjectForce;
import ini.cx3d.physics.debug.PhysicalNodeDebug;
import ini.cx3d.physics.interfaces.PhysicalNode;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.swig.simulation.simulation;

/**
 * Factory that generates PhysicalNode objects
 */
public class PhysicalObjectFactory {

    private static final boolean NATIVE = simulation.useNativePhysicalObject;
    public static final boolean DEBUG = false;//physics.debugPhysicalObject;

    static boolean initializedInterObjectForce = false;
    public static void initializeInterObjectForce() {
        if(!initializedInterObjectForce) {
            setInterObjectForce(DefaultForceFactory.create());
            initializedInterObjectForce = true;
        }
    }
    static {
        initializeInterObjectForce();
    }

    public static InterObjectForce getInterObjectForce() {
        if (NATIVE) {
            return ini.cx3d.swig.simulation.PhysicalObject.getInterObjectForce();
        } else if(!DEBUG) {
            return ini.cx3d.physics.PhysicalObject.getInterObjectForce_java();
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static void setInterObjectForce(InterObjectForce interObjectForce) {
        if (NATIVE) {
            ini.cx3d.swig.simulation.PhysicalObject.setInterObjectForce(interObjectForce);
        } else if(!DEBUG) {
            ini.cx3d.physics.PhysicalObject.setInterObjectForce_java(interObjectForce);
        } else {
            throw new UnsupportedOperationException();
        }
    }
}
