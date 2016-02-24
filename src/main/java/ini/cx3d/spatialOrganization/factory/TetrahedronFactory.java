package ini.cx3d.spatialOrganization.factory;


import ini.cx3d.spatialOrganization.interfaces.SpaceNode;
import ini.cx3d.spatialOrganization.interfaces.Tetrahedron;
import ini.cx3d.spatialOrganization.interfaces.Triangle3D;
import ini.cx3d.swig.spatialOrganization.spatialOrganization;

import java.util.AbstractSequentialList;

/**
 * Factory that generates Tetrahedron objects and enables quick switching between Java and native CPP implementation
 * It also acts as a proxy for static method calls
 */
public class TetrahedronFactory<T> {

    private static final boolean NATIVE = spatialOrganization.useNativeTetrahedron;
    public static final boolean DEBUG = spatialOrganization.debugTetrahedron;

    public Tetrahedron create(Triangle3D<T> oneTriangle, SpaceNode<T> fourthPoint,
                              ini.cx3d.spatialOrganization.interfaces.OpenTriangleOrganizer oto) {
        if (NATIVE) {
            return  ini.cx3d.swig.spatialOrganization.TetrahedronT_PhysicalNode.create(oneTriangle, fourthPoint, oto);
        } else if (!DEBUG){
            return new ini.cx3d.spatialOrganization.Tetrahedron<T>(oneTriangle, fourthPoint, oto);
        } else {
            return new ini.cx3d.spatialOrganization.debug.TetrahedronDebug<T>(oneTriangle, fourthPoint, oto);
        }
    }

    public Tetrahedron create(Triangle3D<T> triangleA, Triangle3D<T> triangleB,
                              Triangle3D<T> triangleC, Triangle3D<T> triangleD,
                              SpaceNode<T> nodeA, SpaceNode<T> nodeB, SpaceNode<T> nodeC,
                              SpaceNode<T> nodeD) {
        if (NATIVE) {
            return ini.cx3d.swig.spatialOrganization.TetrahedronT_PhysicalNode.create(triangleA, triangleB, triangleC, triangleD, nodeA, nodeB, nodeC, nodeD);
        } else if (!DEBUG){
            return new ini.cx3d.spatialOrganization.Tetrahedron<T>(triangleA, triangleB, triangleC, triangleD, nodeA, nodeB, nodeC, nodeD);
        } else {
            return new ini.cx3d.spatialOrganization.debug.TetrahedronDebug<T>(triangleA, triangleB, triangleC, triangleD, nodeA, nodeB, nodeC, nodeD);
        }
    }

    public static <T> Tetrahedron createInitialTetrahedron(SpaceNode a,
                                                           SpaceNode b, SpaceNode c, SpaceNode d, ini.cx3d.spatialOrganization.interfaces.OpenTriangleOrganizer<T> oto) {
        Tetrahedron result = null;
        if (NATIVE) {
            result = ini.cx3d.swig.spatialOrganization.TetrahedronT_PhysicalNode.createInitialTetrahedron(a, b, c, d, oto);
        } else {
            result = ini.cx3d.spatialOrganization.Tetrahedron.createInitialTetrahedron_java(a, b, c, d);
        }
        return result;
    }

    public static int getEdgeNumber(int nodeNumber1, int nodeNumber2) {
        int result = 0;
        if (NATIVE) {
            result = ini.cx3d.swig.spatialOrganization.TetrahedronT_PhysicalNode.getEdgeNumber(nodeNumber1, nodeNumber2);
        } else {
            result = ini.cx3d.spatialOrganization.Tetrahedron.getEdgeNumber(nodeNumber1, nodeNumber2);
        }
        return result;
    }

    public static <T> Tetrahedron[] remove2FlatTetrahedra(
            Tetrahedron tetrahedronA, Tetrahedron tetrahedronB) {
        Tetrahedron[] result = null;
        if (NATIVE) {
            AbstractSequentialList<Tetrahedron> list = ini.cx3d.swig.spatialOrganization.TetrahedronT_PhysicalNode.remove2FlatTetrahedra(tetrahedronA, tetrahedronB);
            result = list.toArray(new Tetrahedron[list.size()]);
        } else {
            result = ini.cx3d.spatialOrganization.Tetrahedron.remove2FlatTetrahedra_java(tetrahedronA, tetrahedronB);
        }
        return result;
    }

    public static <T> Tetrahedron[] flip2to3(Tetrahedron tetrahedronA,
                                             Tetrahedron tetrahedronB) {
        Tetrahedron[] result = null;
        if (NATIVE) {
            result = ini.cx3d.swig.spatialOrganization.TetrahedronT_PhysicalNode.flip2to3(tetrahedronA, tetrahedronB);
        } else {
            result = ini.cx3d.spatialOrganization.Tetrahedron.flip2to3_java(tetrahedronA, tetrahedronB);
        }
        return result;
    }

    public static <T> Tetrahedron[] flip3to2(Tetrahedron tetrahedronA,
                                             Tetrahedron tetrahedronB, Tetrahedron tetrahedronC) {
        Tetrahedron[] result = null;
        if (NATIVE) {
            result = ini.cx3d.swig.spatialOrganization.TetrahedronT_PhysicalNode.flip3to2(tetrahedronA, tetrahedronB, tetrahedronC);
        } else {
            result = ini.cx3d.spatialOrganization.Tetrahedron.flip3to2_java(tetrahedronA, tetrahedronB, tetrahedronC);
        }
        return result;
    }
}

