package ini.cx3d.spatialOrganization.factory;

import ini.cx3d.spatialOrganization.interfaces.TriangleHashKey;
import ini.cx3d.swig.spatialOrganization.spatialOrganization;

/**
 * Factory that generates TriangleHashKey objects and enables quick switching between Java and native CPP implementation
 */
public class TriangleHashKeyFactory<T> {

    private static final boolean NATIVE = spatialOrganization.useNativeTriangleHashKey;
    public static final boolean DEBUG = false;

    public TriangleHashKey create(ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> a, ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> b, ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> c) {
        if (NATIVE) {
            return new ini.cx3d.swig.spatialOrganization.TriangleHashKeyT_PhysicalNode(a, b, c);
        } else if (!DEBUG){
            return new ini.cx3d.spatialOrganization.TriangleHashKey(a, b, c);
        } else {
            throw new UnsupportedOperationException("TriangleHashKeyDebug has not been implemented yet");
        }
    }
}
