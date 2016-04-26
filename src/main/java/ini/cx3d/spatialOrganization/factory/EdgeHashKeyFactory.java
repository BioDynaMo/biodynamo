package ini.cx3d.spatialOrganization.factory;

import ini.cx3d.spatialOrganization.interfaces.EdgeHashKey;
import ini.cx3d.swig.simulation.simulation;

/**
 * Factory that generates EdgeHashKey objects and enables quick switching between Java and native CPP implementation
 */
public class EdgeHashKeyFactory<T> {

    private static final boolean NATIVE = simulation.useNativeEdgeHashKey;
    public static final boolean DEBUG = false;

    public EdgeHashKey create(ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> a, ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> b, ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> opposite_node) {
        if (NATIVE) {
            return new ini.cx3d.swig.simulation.EdgeHashKeyT_PhysicalNode(a, b, opposite_node);
        } else if(!DEBUG){
            return new ini.cx3d.spatialOrganization.EdgeHashKey(a, b, opposite_node);
        } else {
            throw new UnsupportedOperationException("EdgeHashKeyDebug has not been implemented yet");
        }
    }
}
