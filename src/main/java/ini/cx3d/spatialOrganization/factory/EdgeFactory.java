package ini.cx3d.spatialOrganization.factory;

import ini.cx3d.spatialOrganization.interfaces.Edge;
import ini.cx3d.swig.simulation.simulation;

/**
 * Factory that generates Edge objects and enables quick switching between Java and native CPP implementation
 */
public class EdgeFactory<T> {

    private static final boolean NATIVE = simulation.useNativeEdge;
    public static final boolean DEBUG = simulation.debugEdge;

    public Edge create(ini.cx3d.spatialOrganization.interfaces.SpaceNode a, ini.cx3d.spatialOrganization.interfaces.SpaceNode b) {
        if (NATIVE) {
            return  ini.cx3d.swig.simulation.EdgeT_PhysicalNode.create(a, b);
        } else if(!DEBUG) {
            return new ini.cx3d.spatialOrganization.Edge(a, b);
        } else {
            throw new UnsupportedOperationException("EdgeDebug has not been implemented yet");
        }
    }
}
