package ini.cx3d.spatialOrganization.factory;

import ini.cx3d.spatialOrganization.interfaces.ExactVector;
import ini.cx3d.spatialOrganization.interfaces.Rational;
import ini.cx3d.swig.spatialOrganization.spatialOrganization;

/**
 * Factory that generates ExactVector objects and enables quick switching between Java and native CPP implementation
 */
public class ExactVectorFactory {

    private static boolean NATIVE = spatialOrganization.useNativeExactVector;

    public ExactVector create(Rational[] values) {
        if (NATIVE) {
            return ini.cx3d.swig.spatialOrganization.ExactVector.create(values);
        } else {
            return new ini.cx3d.spatialOrganization.ExactVector(values);
        }
    }

    public ExactVector create(double[] values) {
        if (NATIVE) {
            return ini.cx3d.swig.spatialOrganization.ExactVector.create(values);
        } else {
            return new ini.cx3d.spatialOrganization.ExactVector(values);
        }
    }

    public static Rational det(ExactVector[] vector) {
        if (NATIVE) {
            return ini.cx3d.swig.spatialOrganization.ExactVector.det(vector);
        } else {
            return ini.cx3d.spatialOrganization.ExactVector.det(vector);
        }
    }
}
