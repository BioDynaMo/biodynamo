package ini.cx3d.spatialOrganization.interfaces;

/**
 * Common interface for EdgeHashKey implementations
 */
public interface EdgeHashKey<T> {

	/**
	 * @return endpoint A of this edge
	 */
	ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> getEndpointA();

	/**
	 * @return endpoint A of this edge
	 */
	ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> getEndpointB();

	/**
	 * computes the cosine between this edge to another point measured at the
	 * first endpoint of this edge.
	 * @param fourthPoint The other point.
	 * @return The cosine between this edge and an edge between the first
	 * endpoint of this edge and <code>fourthPoint</code>.
	 */
	double getCosine(double[] fourthPoint);

	/**
	 * Returns the opposite node of a given node if the latter is incident to this edge.
	 * @param node The given node.
	 * @return The incident node opposite to <code>node</code>.
	 */
	ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> oppositeNode(ini.cx3d.spatialOrganization.interfaces.SpaceNode<T> node);
}
