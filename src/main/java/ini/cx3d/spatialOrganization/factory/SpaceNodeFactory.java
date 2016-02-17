package ini.cx3d.spatialOrganization.factory;

import ini.cx3d.JavaUtil;
import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.spatialOrganization.PositionNotAllowedException;
import ini.cx3d.spatialOrganization.debug.SpaceNodeDebug;
import ini.cx3d.spatialOrganization.interfaces.Edge;
import ini.cx3d.spatialOrganization.interfaces.SpaceNode;
import ini.cx3d.spatialOrganization.interfaces.Tetrahedron;
import ini.cx3d.swig.spatialOrganization.SpaceNodeT_PhysicalNode;
import ini.cx3d.swig.spatialOrganization.spatialOrganization;

/**
 * Factory that generates SpaceNode objects
 */
public class SpaceNodeFactory<T> {

    private static final boolean NATIVE = spatialOrganization.useNativeSpaceNode;
    public static final boolean DEBUG = spatialOrganization.debugSpaceNode;

    // JavaUtil needs to be static - otherwise it will be garbage collected and
    // also destroyed on the cpp side
    private static JavaUtil java_ = new JavaUtil();
    static {
        SpaceNodeT_PhysicalNode.setJavaUtil(java_);
    }

    public SpaceNode create(double[] position, T content) {
        if (NATIVE) {
           return  SpaceNodeT_PhysicalNode.create(position, (PhysicalNode) content);
        } else if(!DEBUG) {
            return new ini.cx3d.spatialOrganization.SpaceNode(position, content);
        } else {
            return new SpaceNodeDebug(position, content);
        }
    }

    public SpaceNode create(double x, double y, double z, T content) {
        if (NATIVE) {
            return SpaceNodeT_PhysicalNode.create(x, y, z, (PhysicalNode) content);
        } else if(!DEBUG) {
            return new ini.cx3d.spatialOrganization.SpaceNode(x, y, z, content);
        } else {
            return new SpaceNodeDebug(x, y, z, content);
        }
    }

    public static <T> Tetrahedron<T> searchInitialInsertionTetrahedron(
            Tetrahedron start, double[] coordinate)
            throws PositionNotAllowedException {
        if(NATIVE){
            return null;
        } else {
            return ini.cx3d.spatialOrganization.SpaceNode.searchInitialInsertionTetrahedron_java(start, coordinate);
        }
    }
}
