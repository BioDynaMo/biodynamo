package ini.cx3d.spatialOrganization.factory;

import ini.cx3d.spatialOrganization.SpaceNode;
import ini.cx3d.spatialOrganization.SpatialOrganizationEdge;
import ini.cx3d.spatialOrganization.interfaces.Edge;
import ini.cx3d.utilities.DebugUtil;

/**
 * Factory that generates Edge objects and enables quick switching between Java and native CPP implementation
 */
public class EdgeFactory<T> {

    private static final boolean NATIVE = true;
    public static final boolean DEBUG = false;

    public Edge create(SpaceNode<T> a, SpaceNode<T> b) {
        Edge edge = null;
        if (NATIVE) {
            edge =  ini.cx3d.swig.spatialOrganization.EdgeT_PhysicalNode.create(a, b);
        } else {
            edge = new ini.cx3d.spatialOrganization.Edge(a, b);
        }
        if(DEBUG) {
            System.out.println("DBG edge created " + edge);
            edge =  DebugUtil.createDebugLoggingProxy(edge, new Class[]{Edge.class, SpatialOrganizationEdge.class});
        }
        return  edge;
    }
}
