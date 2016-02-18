package ini.cx3d.spatialOrganization.interfaces;

import ini.cx3d.spatialOrganization.DistanceReporter;

import java.util.AbstractSequentialList;

public interface TriangulationNodeOrganizer<T> {
	String toString();

	void addNode(SpaceNode<T> node);

	void removeNode(SpaceNode<T> node);

	SpaceNode<T> getFirstNode();

	AbstractSequentialList<SpaceNode> getNodes(final SpaceNode<T> referencePoint);

	void addTriangleNodes(Triangle3D<T> triangle);
}
