package ini.cx3d.physics.factory;

import ini.cx3d.physics.InterObjectForce;
import ini.cx3d.physics.interfaces.PhysicalSphere;
import ini.cx3d.swig.simulation.simulation;

/**
 * Factory that generates PhysicalSphere objects
 */
public class PhysicalSphereFactory {

    private static final boolean NATIVE = simulation.useNativePhysicalSphere;
    public static final boolean DEBUG =  simulation.debugPhysicalSphere;

    static{
        PhysicalObjectFactory.initializeInterObjectForce();
    }

    public static PhysicalSphere create() {
        if (NATIVE) {
            PhysicalSphere ps = ini.cx3d.swig.simulation.PhysicalSphere.create();
            ini.cx3d.swig.simulation.PhysicalObject.registerJavaObject(ps);
            ini.cx3d.swig.simulation.PhysicalNode.registerJavaObject(ps);
            return ps;
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalSphere();
        } else {
            return new ini.cx3d.physics.debug.PhysicalSphereDebug();
        }
    }
}
