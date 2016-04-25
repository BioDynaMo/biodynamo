package ini.cx3d.physics.factory;

import ini.cx3d.JavaUtil2;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener;
import ini.cx3d.swig.physics.physics;

/**
 * Factory that generates PhysicalNodeMovementListener objects
 */
public class PhysicalNodeMovementListenerFactory {

    private static final boolean NATIVE = physics.useNativePhysicalNodeMovementListener;
    public static final boolean DEBUG = physics.debugPhysicalNodeMovementListener;

    static {
        if(NATIVE){
            ini.cx3d.swig.physics.PhysicalNodeMovementListener.setMovementOperationId((int)(10000* JavaUtil2.getRandomDouble()));
        } else {
            ini.cx3d.physics.PhysicalNodeMovementListener.setMovementOperationId((int)(10000*JavaUtil2.getRandomDouble()));
        }
    }

    public static SpatialOrganizationNodeMovementListener create() {
        if (NATIVE) {
            return ini.cx3d.swig.physics.PhysicalNodeMovementListener.create();
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalNodeMovementListener();
        } else {
            throw new UnsupportedOperationException();
        }
    }
}
