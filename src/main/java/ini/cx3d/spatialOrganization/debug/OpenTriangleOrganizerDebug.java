package ini.cx3d.spatialOrganization.debug;


import ini.cx3d.spatialOrganization.OpenTriangleOrganizer;
import ini.cx3d.spatialOrganization.interfaces.SpaceNode;
import ini.cx3d.spatialOrganization.interfaces.Tetrahedron;
import ini.cx3d.spatialOrganization.interfaces.Triangle3D;
import ini.cx3d.spatialOrganization.interfaces.TriangulationNodeOrganizer;
import ini.cx3d.utilities.DebugUtil;

import java.util.AbstractSequentialList;

public class OpenTriangleOrganizerDebug extends OpenTriangleOrganizer{
    @Override
    public void triangulate() {
        DebugUtil.logMethodCall("triangulate", this, new Object[]{});
        super.triangulate();
        DebugUtil.logMethodReturnVoid("triangulate", this);

    }

    public OpenTriangleOrganizerDebug(int preferredCapacity, TriangulationNodeOrganizer tno) {
        super(preferredCapacity, tno);
        DebugUtil.logMethodCall("OpenTriangleOrganizer created", this, new Object[]{preferredCapacity, tno});

    }

    @Override
    public void recoredNewTetrahedra() {
        DebugUtil.logMethodCall("recoredNewTetrahedra", this, new Object[]{});
        super.recoredNewTetrahedra();
        DebugUtil.logMethodReturnVoid("recoredNewTetrahedra", this);

    }

    @Override
    public AbstractSequentialList<Tetrahedron> getNewTetrahedra() {
        DebugUtil.logMethodCall("getNewTetrahedra", this, new Object[]{});
        AbstractSequentialList<Tetrahedron> ret = super.getNewTetrahedra();
        DebugUtil.logMethodReturn("getNewTetrahedra", this, ret);
        return ret;

    }

    @Override
    public void putTriangle(Triangle3D triangle) {
        DebugUtil.logMethodCall("putTriangle", this, new Object[]{triangle});
        super.putTriangle(triangle);
        DebugUtil.logMethodReturnVoid("putTriangle", this);
    }

    @Override
    public void removeTriangle(Triangle3D triangle) {
        DebugUtil.logMethodCall("removeTriangle", this, new Object[]{triangle});
        super.removeTriangle(triangle);
        DebugUtil.logMethodReturnVoid("removeTriangle", this);

    }

    @Override
    public Triangle3D getTriangle(SpaceNode a, SpaceNode b, SpaceNode c) {
        DebugUtil.logMethodCall("getTriangle", this, new Object[]{a, b, c});
        Triangle3D ret = super.getTriangle(a, b, c);
        DebugUtil.logMethodReturn("getTriangle", this, ret);
        return ret;

    }

    @Override
    public Triangle3D getTriangleWithoutRemoving(SpaceNode a, SpaceNode b, SpaceNode c) {
        DebugUtil.logMethodCall("getTriangleWithoutRemoving", this, new Object[]{a, b, c});
        Triangle3D ret = super.getTriangleWithoutRemoving(a, b, c);
        DebugUtil.logMethodReturn("getTriangleWithoutRemoving", this, ret);
        return ret;

    }

    @Override
    protected boolean contains(SpaceNode a, SpaceNode b, SpaceNode c) {
        DebugUtil.logMethodCall("contains", this, new Object[]{a, b, c});
        boolean ret = super.contains(a, b, c);
        DebugUtil.logMethodReturn("contains", this, ret);
        return ret;

    }

    @Override
    protected boolean isEmpty() {
        DebugUtil.logMethodCall("isEmpty", this, new Object[]{});
        boolean ret = super.isEmpty();
        DebugUtil.logMethodReturn("isEmpty", this, ret);
        return ret;

    }

    @Override
    public Tetrahedron getANewTetrahedron() {
        DebugUtil.logMethodCall("getANewTetrahedron", this, new Object[]{});
        Tetrahedron ret = super.getANewTetrahedron();
        DebugUtil.logMethodReturn("getANewTetrahedron", this, ret);
        return ret;

    }

    @Override
    public void removeAllTetrahedraInSphere(Tetrahedron startingTetrahedron) {
        DebugUtil.logMethodCall("removeAllTetrahedraInSphere", this, new Object[]{startingTetrahedron});
        super.removeAllTetrahedraInSphere(startingTetrahedron);
        DebugUtil.logMethodReturnVoid("removeAllTetrahedraInSphere", this);

    }
}
