package ini.cx3d.physics.factory;

import ini.cx3d.physics.interfaces.PhysicalCylinder;
import ini.cx3d.physics.interfaces.PhysicalSphere;
import ini.cx3d.swig.simulation.simulation;

/**
 * Factory that generates PhysicalCylinder objects
 */
public class PhysicalCylinderFactory {

    private static final boolean NATIVE = simulation.useNativePhysicalCylinder;
    public static final boolean DEBUG = simulation.debugPhysicalCylinder;

    static{
        PhysicalObjectFactory.initializeInterObjectForce();
    }

    public static PhysicalCylinder create() {
        if (NATIVE) {
            PhysicalCylinder ps = ini.cx3d.swig.simulation.PhysicalCylinder.create();
            ini.cx3d.swig.simulation.PhysicalObject.registerJavaObject(ps);
            ini.cx3d.swig.simulation.PhysicalNode.registerJavaObject(ps);
            return ps;
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalCylinder();
        } else {
            return new ini.cx3d.physics.debug.PhysicalCylinderDebug();
        }
    }
}
