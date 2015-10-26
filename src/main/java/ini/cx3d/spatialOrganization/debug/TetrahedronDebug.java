package ini.cx3d.spatialOrganization.debug;

import ini.cx3d.spatialOrganization.OpenTriangleOrganizer;
import ini.cx3d.spatialOrganization.PositionNotAllowedException;
import ini.cx3d.spatialOrganization.SpaceNode;
import ini.cx3d.spatialOrganization.Tetrahedron;
import ini.cx3d.spatialOrganization.interfaces.Edge;
import ini.cx3d.spatialOrganization.interfaces.Triangle3D;
import ini.cx3d.utilities.DebugUtil;

public class TetrahedronDebug<T> extends Tetrahedron<T>{
    @Override
    public boolean equals(Object other) {
//fixme        DebugUtil.logMethodCall("equals", this, new Object[]{ other });
        boolean ret = super.equals(other);
//fixme        DebugUtil.logMethodReturn("equals", this, ret);
        return ret;

    }

    public TetrahedronDebug(Triangle3D oneTriangle, SpaceNode fourthPoint, OpenTriangleOrganizer org) {
        super(oneTriangle, fourthPoint, org);
        DebugUtil.logMethodCall("Tetrahedron created", this, new Object[]{oneTriangle, fourthPoint, org});

    }

    public TetrahedronDebug(Triangle3D triangleA, Triangle3D triangleB, Triangle3D triangleC, Triangle3D triangleD, SpaceNode nodeA, SpaceNode nodeB, SpaceNode nodeC, SpaceNode nodeD) {
        super(triangleA, triangleB, triangleC, triangleD, nodeA, nodeB, nodeC, nodeD);
        DebugUtil.logMethodCall("Tetrahedron created", this, new Object[]{triangleA, triangleB, triangleC, triangleD, nodeA, nodeB, nodeC, nodeD});

    }

    @Override
    public T[] getVerticeContents() {
        DebugUtil.logMethodCall("getVerticeContents", this, new Object[]{ });
        T[] ret = super.getVerticeContents();
//fixme        DebugUtil.logMethodReturn("getVerticeContents", this, ret);
        return ret;

    }

    @Override
    public boolean isInfinite() {
        DebugUtil.logMethodCall("isInfinite", this, new Object[]{ });
        boolean ret = super.isInfinite();
        DebugUtil.logMethodReturn("isInfinite", this, ret);
        return ret;

    }

    @Override
    public boolean isFlat() {
        DebugUtil.logMethodCall("isFlat", this, new Object[]{ });
        boolean ret = super.isFlat();
        DebugUtil.logMethodReturn("isFlat", this, ret);
        return ret;

    }

    @Override
    public void changeCrossSection(int number, double newValue) {
        DebugUtil.logMethodCall("changeCrossSection", this, new Object[]{number, newValue});
        super.changeCrossSection(number, newValue);
        DebugUtil.logMethodReturnVoid("changeCrossSection", this);

    }

    @Override
    public void updateCrossSectionAreas() {
        DebugUtil.logMethodCall("updateCrossSectionAreas", this, new Object[]{});
        super.updateCrossSectionAreas();
        DebugUtil.logMethodReturnVoid("updateCrossSectionAreas", this);

    }

    @Override
    public void calculateVolume() {
        DebugUtil.logMethodCall("calculateVolume", this, new Object[]{});
        super.calculateVolume();
        DebugUtil.logMethodReturnVoid("calculateVolume", this);

    }

    @Override
    public int orientationExact(double[] position) {
        DebugUtil.logMethodCall("orientationExact", this, new Object[]{ position});
        int ret = super.orientationExact(position);
        DebugUtil.logMethodReturn("orientationExact", this, ret);
        return ret;

    }

    @Override
    public void calculateCircumSphere() {
        DebugUtil.logMethodCall("calculateCircumSphere", this, new Object[]{});
        super.calculateCircumSphere();
        DebugUtil.logMethodReturnVoid("calculateCircumSphere", this);

    }

    @Override
    public void updateCirumSphereAfterNodeMovement(SpaceNode movedNode) {
        DebugUtil.logMethodCall("updateCirumSphereAfterNodeMovement", this, new Object[]{movedNode});
        super.updateCirumSphereAfterNodeMovement(movedNode);
        DebugUtil.logMethodReturnVoid("updateCirumSphereAfterNodeMovement", this);

    }

    @Override
    public int orientation(double[] point) {
        DebugUtil.logMethodCall("orientation", this, new Object[]{point});
        int ret = super.orientation(point);
        DebugUtil.logMethodReturn("orientation", this, ret);
        return ret;

    }

    @Override
    public boolean isTrulyInsideSphere(double[] point) {
        DebugUtil.logMethodCall("isTrulyInsideSphere", this, new Object[]{point});
        boolean ret = super.isTrulyInsideSphere(point);
        DebugUtil.logMethodReturn("isTrulyInsideSphere", this, ret);
        return ret;

    }

    @Override
    public boolean isInsideSphere(double[] point) {
        DebugUtil.logMethodCall("isInsideSphere", this, new Object[]{point});
        boolean ret = super.isInsideSphere(point);
        DebugUtil.logMethodReturn("isInsideSphere", this, ret);
        return ret;

    }

    @Override
    public void remove() {
        DebugUtil.logMethodCall("remove", this, new Object[]{});
        super.remove();
        DebugUtil.logMethodReturnVoid("remove", this);

    }

    @Override
    public void replaceTriangle(Triangle3D oldTriangle, Triangle3D newTriangle) {
        DebugUtil.logMethodCall("replaceTriangle", this, new Object[]{oldTriangle, newTriangle});
        super.replaceTriangle(oldTriangle, newTriangle);
        DebugUtil.logMethodReturnVoid("replaceTriangle", this);

    }

    @Override
    public int getNodeNumber(SpaceNode node) {
        DebugUtil.logMethodCall("getNodeNumber", this, new Object[]{node});
        int ret = super.getNodeNumber(node);
        DebugUtil.logMethodReturn("getNodeNumber", this, ret);
        return ret;

    }

    @Override
    public int getTriangleNumber(Triangle3D triangle) {
        DebugUtil.logMethodCall("getTriangleNumber", this, new Object[]{triangle});
        int ret = super.getTriangleNumber(triangle);
        DebugUtil.logMethodReturn("getTriangleNumber", this, ret);
        return ret;

    }

    @Override
    public Edge getEdge(int nodeNumber1, int nodeNumber2) {
        DebugUtil.logMethodCall("getEdge", this, new Object[]{ nodeNumber1, nodeNumber2});
        Edge ret = super.getEdge(nodeNumber1, nodeNumber2);
        DebugUtil.logMethodReturn("getEdge", this, ret);
        return ret;

    }

    @Override
    public int getEdgeNumber(SpaceNode a, SpaceNode b) {
        DebugUtil.logMethodCall("getEdgeNumber", this, new Object[]{ a, b});
        int ret = super.getEdgeNumber(a, b);
        DebugUtil.logMethodReturn("getEdgeNumber", this, ret);
        return ret;

    }

    @Override
    public Edge getEdge(SpaceNode a, SpaceNode b) {
        DebugUtil.logMethodCall("getEdge", this, new Object[]{ a, b});
        Edge ret = super.getEdge(a, b);
        DebugUtil.logMethodReturn("getEdge", this, ret);
        return ret;

    }

    @Override
    public Triangle3D getOppositeTriangle(SpaceNode node) {
        DebugUtil.logMethodCall("getOppositeTriangle", this, new Object[]{ node });
        Triangle3D ret = super.getOppositeTriangle(node);
        DebugUtil.logMethodReturn("getOppositeTriangle", this, ret);
        return ret;

    }

    @Override
    public SpaceNode getOppositeNode(Triangle3D triangle) {
        DebugUtil.logMethodCall("getOppositeNode", this, new Object[]{ triangle });
        SpaceNode ret = super.getOppositeNode(triangle);
        DebugUtil.logMethodReturn("getOppositeNode", this, ret);
        return ret;

    }

    @Override
    public Triangle3D getConnectingTriangle(ini.cx3d.spatialOrganization.interfaces.Tetrahedron tetrahedron) {
        DebugUtil.logMethodCall("getConnectingTriangle", this, new Object[]{ tetrahedron })
        ;
        Triangle3D ret = super.getConnectingTriangle(tetrahedron);
        DebugUtil.logMethodReturn("getConnectingTriangle", this, ret);
        return ret;

    }

    @Override
    public int getConnectingTriangleNumber(ini.cx3d.spatialOrganization.interfaces.Tetrahedron tetrahedron) {
        DebugUtil.logMethodCall("getConnectingTriangleNumber", this, new Object[]{ tetrahedron });
        int ret = super.getConnectingTriangleNumber(tetrahedron);
        DebugUtil.logMethodReturn("getConnectingTriangleNumber", this, ret);
        return ret;

    }

    @Override
    public Triangle3D[] getTouchingTriangles(Triangle3D base) {
        DebugUtil.logMethodCall("getTouchingTriangles", this, new Object[]{ base });
        Triangle3D[] ret = super.getTouchingTriangles(base);
        DebugUtil.logMethodReturn("getTouchingTriangles", this, ret);
        return ret;

    }

    @Override
    public SpaceNode getFirstOtherNode(SpaceNode nodeA, SpaceNode nodeB) {
        DebugUtil.logMethodCall("getFirstOtherNode", this, new Object[]{ nodeA, nodeB });
        SpaceNode ret = super.getFirstOtherNode(nodeA, nodeB);
        DebugUtil.logMethodReturn("getFirstOtherNode", this, ret);
        return ret;

    }

    @Override
    public SpaceNode getSecondOtherNode(SpaceNode nodeA, SpaceNode nodeB) {
        DebugUtil.logMethodCall("getSecondOtherNode", this, new Object[]{ nodeA, nodeB});
        SpaceNode ret = super.getSecondOtherNode(nodeA, nodeB);
        DebugUtil.logMethodReturn("getSecondOtherNode", this, ret);
        return ret;

    }

    @Override
    public boolean isPointInConvexPosition(double[] point, int connectingTriangleNumber) {
        DebugUtil.logMethodCall("isPointInConvexPosition", this, new Object[]{point, connectingTriangleNumber});
        boolean ret = super.isPointInConvexPosition(point, connectingTriangleNumber);
        DebugUtil.logMethodReturn("isPointInConvexPosition", this, ret);
        return ret;

    }

    @Override
    public int isInConvexPosition(double[] point, int connectingTriangleNumber) {
        DebugUtil.logMethodCall("isInConvexPosition", this, new Object[]{point, connectingTriangleNumber});
        int ret = super.isInConvexPosition(point, connectingTriangleNumber);
        DebugUtil.logMethodReturn("isInConvexPosition", this, ret);
        return ret;

    }

    @Override
    public SpaceNode[] getAdjacentNodes() {
        DebugUtil.logMethodCall("getAdjacentNodes", this, new Object[]{ });
        SpaceNode[] ret = super.getAdjacentNodes();
        DebugUtil.logMethodReturn("getAdjacentNodes", this, ret);
        return ret;

    }

    @Override
    public ini.cx3d.spatialOrganization.interfaces.Tetrahedron getAdjacentTetrahedron(int number) {
        DebugUtil.logMethodCall("getAdjacentTetrahedron", this, new Object[]{ number});
        ini.cx3d.spatialOrganization.interfaces.Tetrahedron ret = super.getAdjacentTetrahedron(number);
        DebugUtil.logMethodReturn("getAdjacentTetrahedron", this, ret);
        return ret;

    }

    @Override
    public Triangle3D[] getAdjacentTriangles() {
        DebugUtil.logMethodCall("getAdjacentTriangles", this, new Object[]{ });
        Triangle3D[] ret = super.getAdjacentTriangles();
        DebugUtil.logMethodReturn("getAdjacentTriangles", this, ret);
        return ret;

    }

    @Override
    public boolean isAdjacentTo(SpaceNode node) {
        DebugUtil.logMethodCall("isAdjacentTo", this, new Object[]{ node});
        boolean ret = super.isAdjacentTo(node);
        DebugUtil.logMethodReturn("isAdjacentTo", this, ret);
        return ret;

    }

    @Override
    public ini.cx3d.spatialOrganization.interfaces.Tetrahedron walkToPoint(double[] coordinate, int[] order) throws PositionNotAllowedException {
        DebugUtil.logMethodCall("walkToPoint", this, new Object[]{ coordinate, order});
        ini.cx3d.spatialOrganization.interfaces.Tetrahedron ret = super.walkToPoint(coordinate, order);
        DebugUtil.logMethodReturn("walkToPoint", this, ret);
        return ret;

    }

    @Override
    public void testPosition(double[] position) throws PositionNotAllowedException {
        DebugUtil.logMethodCall("testPosition", this, new Object[]{position});
        super.testPosition(position);
        DebugUtil.logMethodReturnVoid("testPosition", this);

    }

    @Override
    public boolean isValid() {
        DebugUtil.logMethodCall("isValid", this, new Object[]{ });
        boolean ret = super.isValid();
        DebugUtil.logMethodReturn("isValid", this, ret);
        return ret;

    }

    @Override
    public boolean isNeighbor(ini.cx3d.spatialOrganization.interfaces.Tetrahedron otherTetrahedron) {
        DebugUtil.logMethodCall("isNeighbor", this, new Object[]{ otherTetrahedron});
        boolean ret = super.isNeighbor(otherTetrahedron);
        DebugUtil.logMethodReturn("isNeighbor", this, ret);
        return ret;

    }
}
