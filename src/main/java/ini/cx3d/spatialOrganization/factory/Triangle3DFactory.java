package ini.cx3d.spatialOrganization.factory;

import ini.cx3d.spatialOrganization.interfaces.ExactVector;
import ini.cx3d.spatialOrganization.interfaces.Rational;
import ini.cx3d.spatialOrganization.interfaces.Triangle3D;
import ini.cx3d.spatialOrganization.interfaces.Tetrahedron;
import ini.cx3d.swig.simulation.simulation;

/**
 * Factory that generates Triangle3D object and enables quick switching between Java and native CPP implementation
 */
public class Triangle3DFactory<T> {

    private static boolean NATIVE = simulation.useNativeTriangle3D;
    public static final boolean DEBUG = simulation.debugTriangle3D;

    public Triangle3D<T> create(ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> sn1, ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> sn2, ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> sn3,
                                Tetrahedron tetrahedron1, Tetrahedron tetrahedron2) {
        Triangle3D triangle = null;
        if (NATIVE) {
            return  (Triangle3D<T>) ini.cx3d.swig.simulation.Triangle3DT_PhysicalNode.create( sn1, sn2, sn3,
                    tetrahedron1, tetrahedron2);
        } else if (!DEBUG) {
            return  new ini.cx3d.spatialOrganization.Triangle3D<T>((ini.cx3d.spatialOrganization.SpaceNode) sn1,
                    (ini.cx3d.spatialOrganization.SpaceNode) sn2,
                    (ini.cx3d.spatialOrganization.SpaceNode) sn3,
                    (ini.cx3d.spatialOrganization.Tetrahedron) tetrahedron1,
                    (ini.cx3d.spatialOrganization.Tetrahedron) tetrahedron2);
        } else {
            throw new UnsupportedOperationException("Triangle3DDebug has not been implemented yet");
        }
    }

    public static double[] calculate3PlaneXPoint(double[][] normals,
                                                 double[] offsets) {
        if (NATIVE) {
            return ini.cx3d.swig.simulation.Triangle3DT_PhysicalNode.calculate3PlaneXPoint(normals, offsets);
        } else {
            double[] ret = ini.cx3d.spatialOrganization.Triangle3D.calculate3PlaneXPoint(normals, offsets);
            return ret;
        }
    }

    public static ExactVector calculate3PlaneXPoint(ExactVector[] normals,
                                                    Rational[] offsets, Rational normalDet){
        if (NATIVE) {
            return ini.cx3d.swig.simulation.Triangle3DT_PhysicalNode.calculate3PlaneXPoint(normals, offsets, normalDet);
        } else {
            ExactVector ret =  ini.cx3d.spatialOrganization.Triangle3D.calculate3PlaneXPoint(normals, offsets, normalDet);
            return ret;
        }
    }

    public static double[] calculate3PlaneXPoint(double[][] normals,
                                                 double[] offsets, double normalDet){
        if (NATIVE) {
            return ini.cx3d.swig.simulation.Triangle3DT_PhysicalNode.calculate3PlaneXPoint(normals, offsets, normalDet);
        } else {
            double[] ret =  ini.cx3d.spatialOrganization.Triangle3D.calculate3PlaneXPoint(normals, offsets, normalDet);
            return ret;
        }
    }
}
