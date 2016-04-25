package ini.cx3d.physics.factory;

import ini.cx3d.physics.debug.PhysicalNodeDebug;
import ini.cx3d.physics.interfaces.PhysicalNode;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.swig.physics.physics;

/**
 * Factory that generates PhysicalNode objects
 */
public class PhysicalNodeFactory {

    private static final boolean NATIVE = physics.useNativePhysicalNode;
    public static final boolean DEBUG = physics.debugPhysicalNode;

    static {
        ini.cx3d.swig.physics.PhysicalNode.setECM(ECMFacade.getInstance());
    }

    public static PhysicalNode create() {
        if (NATIVE) {
            PhysicalNode node =  ini.cx3d.swig.physics.PhysicalNode.create();
            ini.cx3d.swig.physics.PhysicalNode.registerJavaObject(node);
            return node;
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalNode();
        } else {
            PhysicalNode node =  new PhysicalNodeDebug();
            ini.cx3d.swig.physics.PhysicalNode.registerJavaObject(node);
            return node;
        }
    }

    public static double[] getBarycentricCoordinates(double[] Q, double[] P1, double[] P2, double[] P3, double[] P4){
        if (NATIVE) {
            return ini.cx3d.swig.physics.PhysicalNode.getBarycentricCoordinates(Q, P1, P2, P3, P4);
        } else if(!DEBUG) {
            return ini.cx3d.physics.PhysicalNode.getBarycentricCoordinates_java(Q, P1, P2, P3, P4);
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static double[] getBarycentricCoordinates(double[] Q, Object[] vertices){
        if (NATIVE) {
            return ini.cx3d.swig.physics.PhysicalNode.getBarycentricCoordinates(Q, vertices);
        } else if(!DEBUG) {
            return ini.cx3d.physics.PhysicalNode.getBarycentricCoordinates_java(Q, vertices);
        } else {
            throw new UnsupportedOperationException();
        }
    }
}
