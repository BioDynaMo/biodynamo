package ini.cx3d.spatialOrganization.factory;

import ini.cx3d.spatialOrganization.debug.OpenTriangleOrganizerDebug;
import ini.cx3d.spatialOrganization.interfaces.OpenTriangleOrganizer;
import ini.cx3d.spatialOrganization.interfaces.TriangulationNodeOrganizer;
import ini.cx3d.swig.simulation.OpenTriangleOrganizerT_PhysicalNode;
import ini.cx3d.swig.simulation.simulation;

/**
 * Factory that generates OpenTriangleOrganizer objects
 */
public class OpenTriangleOrganizerFactory {

    private static final boolean NATIVE = simulation.useNativeOpenTriangleOrganizer;
    public static final boolean DEBUG = simulation.debugOpenTriangleOrganizer;

    public static <T> OpenTriangleOrganizer create(int preferredCapacity,
                                        TriangulationNodeOrganizer<T> tno) {
        if (NATIVE) {
            return OpenTriangleOrganizerT_PhysicalNode.create(preferredCapacity, tno);
        } else if(!DEBUG) {
            return new ini.cx3d.spatialOrganization.OpenTriangleOrganizer(preferredCapacity, tno);
        } else {
            return new OpenTriangleOrganizerDebug(preferredCapacity, tno);
        }
    }

    public static <T> ini.cx3d.spatialOrganization.interfaces.OpenTriangleOrganizer createSimpleOpenTriangleOrganizer() {
        if(NATIVE) {
            return OpenTriangleOrganizerT_PhysicalNode.createSimpleOpenTriangleOrganizer();
        } else {
            return ini.cx3d.spatialOrganization.OpenTriangleOrganizer.createSimpleOpenTriangleOrganizer_java();
        }
    }
}
