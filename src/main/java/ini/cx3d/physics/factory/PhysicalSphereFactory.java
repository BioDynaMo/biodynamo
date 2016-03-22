package ini.cx3d.physics.factory;

import ini.cx3d.physics.InterObjectForce;
import ini.cx3d.physics.interfaces.PhysicalSphere;
import ini.cx3d.swig.physics.physics;

/**
 * Factory that generates PhysicalSphere objects
 */
public class PhysicalSphereFactory {

    private static final boolean NATIVE = physics.useNativePhysicalSphere;
    public static final boolean DEBUG =  physics.debugPhysicalSphere;

    static{
        PhysicalObjectFactory.initializeInterObjectForce();
    }

    public static PhysicalSphere create() {
        if (NATIVE) {
            PhysicalSphere ps = ini.cx3d.swig.physics.PhysicalSphere.create();
            ini.cx3d.swig.physics.PhysicalObject.registerJavaObject(ps);
            ini.cx3d.swig.physics.PhysicalNode.registerJavaObject(ps);
            return ps;
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalSphere();
        } else {
            return new ini.cx3d.physics.debug.PhysicalSphereDebug();
        }
    }
}
