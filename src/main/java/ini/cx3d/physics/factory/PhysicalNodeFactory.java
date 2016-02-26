package ini.cx3d.physics.factory;

import ini.cx3d.physics.interfaces.PhysicalNode;
import ini.cx3d.swig.physics.physics;

/**
 * Factory that generates PhysicalNode objects
 */
public class PhysicalNodeFactory {

    private static final boolean NATIVE = false;//physics.useNativePhysicalNode;
    public static final boolean DEBUG = false;//physics.debugPhysicalNode;

    public static PhysicalNode create() {
        if (NATIVE) {
            throw new UnsupportedOperationException();//return ini.cx3d.swig.physics.PhysicalNode.create();
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalNode();
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static double[] getBarycentricCoordinates(double[] Q, double[] P1, double[] P2, double[] P3, double[] P4){
        if (NATIVE) {
            throw new UnsupportedOperationException();//return ini.cx3d.swig.physics.PhysicalNode.getBarycentricCoordinates(Q, P1, P2, P3, P4);
        } else if(!DEBUG) {
            return ini.cx3d.physics.PhysicalNode.getBarycentricCoordinates(Q, P1, P2, P3, P4);
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static double[] getBarycentricCoordinates(double[] Q, Object[] vertices){
        if (NATIVE) {
            throw new UnsupportedOperationException();//return ini.cx3d.swig.physics.PhysicalNode.getBarycentricCoordinates(Q, vertices);
        } else if(!DEBUG) {
            return ini.cx3d.physics.PhysicalNode.getBarycentricCoordinates(Q, vertices);
        } else {
            throw new UnsupportedOperationException();
        }
    }
}
