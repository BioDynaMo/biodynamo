package ini.cx3d.spatialOrganization.debug;

import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.spatialOrganization.PositionNotAllowedException;
import ini.cx3d.spatialOrganization.SpaceNode;
import ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener;
import ini.cx3d.spatialOrganization.interfaces.Edge;
import ini.cx3d.spatialOrganization.interfaces.Tetrahedron;
import ini.cx3d.utilities.DebugUtil;

import java.util.AbstractSequentialList;

public class SpaceNodeDebug extends SpaceNode{
    public SpaceNodeDebug(double[] position, Object content) {
        super(position, content);
        DebugUtil.logMethodCall("SpaceNode created", this, new Object[]{position, content});

    }

    public SpaceNodeDebug(double x, double y, double z, Object content) {
        super(x, y, z, content);
        DebugUtil.logMethodCall("SpaceNode created", this, new Object[]{x, y, z, content});
    }

    @Override
    public void addEdge(Edge newEdge) {
        DebugUtil.logMethodCall("addEdge", this, new Object[]{newEdge});
        super.addEdge(newEdge);
        DebugUtil.logMethodReturnVoid("addEdge", this);

    }

    @Override
    public Edge searchEdge(ini.cx3d.spatialOrganization.interfaces.SpaceNode oppositeNode) {
        DebugUtil.logMethodCall("searchEdge", this, new Object[]{oppositeNode});
        Edge ret = super.searchEdge(oppositeNode);
        DebugUtil.logMethodReturn("searchEdge", this, ret);
        return ret;

    }

    @Override
    public void removeEdge(Edge e) {
        DebugUtil.logMethodCall("removeEdge", this, new Object[]{e});
        super.removeEdge(e);
        DebugUtil.logMethodReturnVoid("removeEdge", this);

    }

    @Override
    public void removeTetrahedron(Tetrahedron tetrahedron) {
        DebugUtil.logMethodCall("removeTetrahedron", this, new Object[]{tetrahedron});
        super.removeTetrahedron(tetrahedron);
        DebugUtil.logMethodReturnVoid("removeTetrahedron", this);

    }

    @Override
    public AbstractSequentialList<Edge> getEdges() {
        DebugUtil.logMethodCall("getEdges", this, new Object[]{});
        AbstractSequentialList<Edge> ret = super.getEdges();
        DebugUtil.logMethodReturn("getEdges", this, ret);
        return ret;

    }

    @Override
    public PhysicalNode getUserObject() {
        DebugUtil.logMethodCall("getUserObject", this, new Object[]{});
        PhysicalNode ret = super.getUserObject();
        DebugUtil.logMethodReturn("getUserObject", this, ret);
        return ret;

    }

    @Override
    public AbstractSequentialList<PhysicalNode> getNeighbors() {
        DebugUtil.logMethodCall("getNeighbors", this, new Object[]{});
        AbstractSequentialList<PhysicalNode> ret = super.getNeighbors();
        DebugUtil.logMethodReturn("getNeighbors", this, ret);
        return ret;

    }

    @Override
    public AbstractSequentialList<PhysicalNode> getPermanentListOfNeighbors() {
        DebugUtil.logMethodCall("getPermanentListOfNeighbors", this, new Object[]{});
        AbstractSequentialList<PhysicalNode> ret = super.getPermanentListOfNeighbors();
        DebugUtil.logMethodReturn("getPermanentListOfNeighbors", this, ret);
        return ret;

    }

    @Override
    public Object[] getVerticesOfTheTetrahedronContaining(double[] position) {
        DebugUtil.logMethodCall("getVerticesOfTheTetrahedronContaining", this, new Object[]{position});
        Object[] ret = super.getVerticesOfTheTetrahedronContaining(position);
        DebugUtil.logMethodReturn("getVerticesOfTheTetrahedronContaining", this, ret);
        return ret;

    }

    @Override
    public void changeVolume(double change) {
        DebugUtil.logMethodCall("changeVolume", this, new Object[]{change});
        super.changeVolume(change);
        DebugUtil.logMethodReturnVoid("changeVolume", this);

    }

    @Override
    public double getVolume() {
        DebugUtil.logMethodCall("getVolume", this, new Object[]{});
        double ret = super.getVolume();
        DebugUtil.logMethodReturn("getVolume", this, ret);
        return ret;

    }

    @Override
    public SpaceNode getNewInstance(double[] position, PhysicalNode userObject) {
        DebugUtil.logMethodCall("getNewInstance", this, new Object[]{position, userObject});
        SpaceNode ret = super.getNewInstance(position, userObject);
        DebugUtil.logMethodReturn("getNewInstance", this, ret);
        return ret;

    }

    @Override
    public void setListenerList(AbstractSequentialList abstractSequentialList) {
        DebugUtil.logMethodCall("setListenerList", this, new Object[]{abstractSequentialList});
        super.setListenerList(abstractSequentialList);
        DebugUtil.logMethodReturnVoid("setListenerList", this);

    }

    @Override
    public void addSpatialOrganizationNodeMovementListener(SpatialOrganizationNodeMovementListener listener) {
        DebugUtil.logMethodCall("addSpatialOrganizationNodeMovementListener", this, new Object[]{listener});
        super.addSpatialOrganizationNodeMovementListener(listener);
        DebugUtil.logMethodReturnVoid("addSpatialOrganizationNodeMovementListener", this);

    }

    @Override
    public AbstractSequentialList<Tetrahedron> getAdjacentTetrahedra() {
        DebugUtil.logMethodCall("getAdjacentTetrahedra", this, new Object[]{});
        AbstractSequentialList<Tetrahedron> ret = super.getAdjacentTetrahedra();
        DebugUtil.logMethodReturn("getAdjacentTetrahedra", this, ret);
        return ret;

    }

    @Override
    public void addAdjacentTetrahedron(Tetrahedron tetrahedron) {
        DebugUtil.logMethodCall("addAdjacentTetrahedron", this, new Object[]{tetrahedron});
        super.addAdjacentTetrahedron(tetrahedron);
        DebugUtil.logMethodReturnVoid("addAdjacentTetrahedron", this);

    }

    @Override
    public double[] getPosition() {
        DebugUtil.logMethodCall("getPosition", this, new Object[]{});
        double[] ret = super.getPosition();
        DebugUtil.logMethodReturn("getPosition", this, ret);
        return ret;

    }

    @Override
    public int getId() {
        DebugUtil.logMethodCall("getId", this, new Object[]{});
        int ret = super.getId();
        DebugUtil.logMethodReturn("getId", this, ret);
        return ret;

    }

    @Override
    public void remove() {
        DebugUtil.logMethodCall("remove", this, new Object[]{});
        super.remove();
        DebugUtil.logMethodReturnVoid("remove", this);

    }

    @Override
    public Tetrahedron searchInitialInsertionTetrahedron(Tetrahedron start) throws PositionNotAllowedException {
        DebugUtil.logMethodCall("searchInitialInsertionTetrahedron", this, new Object[]{start});
        Tetrahedron ret = super.searchInitialInsertionTetrahedron(start);
        DebugUtil.logMethodReturn("searchInitialInsertionTetrahedron", this, ret);
        return ret;

    }

    @Override
    public Tetrahedron insert(Tetrahedron start) throws PositionNotAllowedException {
        DebugUtil.logMethodCall("insert", this, new Object[]{start});
        Tetrahedron ret = super.insert(start);
        DebugUtil.logMethodReturn("insert", this, ret);
        return ret;

    }

    @Override
    public void restoreDelaunay() {
        DebugUtil.logMethodCall("restoreDelaunay", this, new Object[]{});
        super.restoreDelaunay();
        DebugUtil.logMethodReturnVoid("restoreDelaunay", this);

    }

    @Override
    public void moveFrom(double[] delta) throws PositionNotAllowedException {
        DebugUtil.logMethodCall("moveFrom", this, new Object[]{delta});
        super.moveFrom(delta);
        DebugUtil.logMethodReturnVoid("moveFrom", this);

    }

    @Override
    public void moveTo(double[] newPosition) throws PositionNotAllowedException {
        DebugUtil.logMethodCall("moveTo", this, new Object[]{newPosition});
        super.moveTo(newPosition);
        DebugUtil.logMethodReturnVoid("moveTo", this);

    }

    @Override
    public double[] proposeNewPosition() {
        DebugUtil.logMethodCall("proposeNewPosition", this, new Object[]{});
        double[] ret = super.proposeNewPosition();
        DebugUtil.logMethodReturn("proposeNewPosition", this, ret);
        return ret;

    }

    @Override
    public AbstractSequentialList<Edge> getAdjacentEdges() {
        DebugUtil.logMethodCall("getAdjacentEdges", this, new Object[]{});
        AbstractSequentialList<Edge> ret = super.getAdjacentEdges();
        DebugUtil.logMethodReturn("getAdjacentEdges", this, ret);
        return ret;

    }
}
