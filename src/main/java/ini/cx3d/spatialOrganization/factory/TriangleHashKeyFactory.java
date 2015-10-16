package ini.cx3d.spatialOrganization.factory;

import ini.cx3d.spatialOrganization.SpaceNode;
import ini.cx3d.spatialOrganization.interfaces.EdgeHashKey;
import ini.cx3d.spatialOrganization.interfaces.TriangleHashKey;
import ini.cx3d.utilities.DebugUtil;

/**
 * Factory that generates TriangleHashKey objects and enables quick switching between Java and native CPP implementation
 */
public class TriangleHashKeyFactory<T> {

    private static final boolean NATIVE = true;
    public static final boolean DEBUG = false;

    public TriangleHashKey create(SpaceNode<T> a, SpaceNode<T> b, SpaceNode<T> c) {
        TriangleHashKey triangleHashKey = null;
        if (NATIVE) {
            triangleHashKey = new ini.cx3d.swig.spatialOrganization.TriangleHashKeyT_PhysicalNode(a, b, c);
        } else {
            triangleHashKey = new ini.cx3d.spatialOrganization.TriangleHashKey(a, b, c);
        }
        if(DEBUG) {
            System.out.println("DBG TriangleHashKey created " + triangleHashKey);
            triangleHashKey =  DebugUtil.createDebugLoggingProxy(triangleHashKey, new Class[]{EdgeHashKey.class});
        }
        return  triangleHashKey;
    }
}
