package ini.cx3d.spatialOrganization.interfaces;

import ini.cx3d.spatialOrganization.SpaceNode;
import ini.cx3d.spatialOrganization.SpatialOrganizationEdge;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;
import ini.cx3d.spatialOrganization.Tetrahedron;

import java.util.AbstractSequentialList;

/**
 * Common interface for Edge implementations
 */
public interface Edge<T> extends SpatialOrganizationEdge<T> {
	/**
	 * {@inheritDoc}
	 */
	SpatialOrganizationNode<T> getOpposite(SpaceNode<T> comingFrom);

	/**
	 * {@inheritDoc}
	 */
	T getOppositeElement(T element);

	/**
	 * {@inheritDoc}
	 */
	T getFirstElement();

	/**
	 * {@inheritDoc}
	 */
	T getSecondElement();

	/**
	 * {@inheritDoc}
	 */
	double getCrossSection();

	/**
	 *	@return A string representation of this edge
	 */
	String toString();

	boolean equals(SpaceNode<T> a, SpaceNode<T> b);

	/**
	 * Removes a tetrahedron from this edge's list of tetrahedra. If this edge is not incident to
	 * any tetrahedra after the removal of the specified tetrahedron, the edge removes itself from
	 * the triangulation by calling {@link #remove()}.
	 * @param tetrahedron A tetrahedron incident to this edge which should be removed.
	 */
	void removeTetrahedron(Tetrahedron<T> tetrahedron);

	/**
	 * Adds a tetrahedron to this edge's list of tetrahedra.
	 * @param tetrahedron A tetrahedron incident to this edge which should be added.
	 */
	void addTetrahedron(Tetrahedron<T> tetrahedron);

	/**
	 * Removes this edge from the triangulation. To do so, the two endpoints are informed
	 * that the edge was removed.
	 *
	 */
	void remove();

	/**
	 * Returns the list of incident tetrahedra.
	 * @return The list of incident tetrahedra.
	 */
	AbstractSequentialList<Tetrahedron> getAdjacentTetrahedra();

	/**
	 * Changes the cross section area of this edge.
	 * @param change The value by which the cross section area of this edge has changed.
	 * At initialization, this area is set to zero and all tetrahedra that are registered as
	 * incident tetrahedra increase the cross section area.
	 */
	void changeCrossSectionArea(double change);
}
