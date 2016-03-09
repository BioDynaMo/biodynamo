package ini.cx3d.physics.factory;

import ini.cx3d.physics.PhysicalObject;
import ini.cx3d.simulations.ECM;
import ini.cx3d.physics.interfaces.PhysicalBond;
import ini.cx3d.swig.physics.physics;

/**
 * Factory that generates PhysicalBond objects
 */
public class PhysicalBondFactory {

    private static final boolean NATIVE = physics.useNativePhysicalBond;
    public static final boolean DEBUG = physics.debugPhysicalBond;

    public static PhysicalBond create() {
        if (NATIVE) {
            return ini.cx3d.swig.physics.PhysicalBond.create();
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalBond();
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static PhysicalBond create(PhysicalObject a, PhysicalObject b) {
        if (NATIVE) {
            return ini.cx3d.swig.physics.PhysicalBond.create(a, b);
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalBond(a, b);
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static PhysicalBond create(PhysicalObject a, double[] positionOnA, PhysicalObject b , double[] positionOnB, double restingLength, double springConstant) {
        if (NATIVE) {
            return ini.cx3d.swig.physics.PhysicalBond.create(a, positionOnA, b, positionOnB, restingLength, springConstant);
        } else if(!DEBUG) {
            return new ini.cx3d.physics.PhysicalBond(a, positionOnA, b, positionOnB, restingLength, springConstant);
        } else {
            throw new UnsupportedOperationException();
        }
    }
}
