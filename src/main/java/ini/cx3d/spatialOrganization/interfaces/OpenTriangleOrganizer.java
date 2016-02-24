package ini.cx3d.spatialOrganization.interfaces;

import ini.cx3d.spatialOrganization.AbstractTriangulationNodeOrganizer;

import java.util.AbstractSequentialList;
import java.util.LinkedList;

/**
 * Created by lukas on 19.02.16.
 */
public interface OpenTriangleOrganizer<T> {
	/**
	 * Starts the recording of newly created tetrahedra.
	 */
	void recoredNewTetrahedra();

	/**
	 * Returns the list of newly created tetrahedra.
	 * @return The list of newly created tetrahedra, if recording was turned on before
	 * (Use {@link #recoredNewTetrahedra()}) or <code>null</code> else.
	 */
	AbstractSequentialList<Tetrahedron> getNewTetrahedra();

	/**
	 * Informs this open triangle organizer that a new
	 * open triangle is available. In order to do so, the new open
	 * triangle is added to the hashmap.
	 * @param triangle The new open triangle.
	 */
	void putTriangle(Triangle3D triangle);

	/**
	 * Informs this open triangle organizer that an open triangle
	 * is no longer available. In order to do so, the new open
	 * triangle is removed from the hashmap.
	 * @param triangle The triangle that should be removed.
	 */
	void removeTriangle(Triangle3D triangle);

	/**
	 * Searches for a triangle which is incident to three specified nodes.
	 * If such an open triangle is not yet known to this
	 * open triangle organizer, it is created and added to the
	 * hashmap. Otherwise, it is removed from the hashmap (since it won't be
	 * open any more soon).
	 * @param a The first node incident to the requested triangle.
	 * @param b The second node incident to the requested triangle.
	 * @param c The third node incident to the requested triangle.
	 * @return A triangle with the requested three endpoints.
	 */
	Triangle3D<T> getTriangle(SpaceNode a, SpaceNode b,
							  SpaceNode c);

	/**
	 * Searches for a triangle which is incident to three specified nodes.
	 * If this triangle does not exist yet, it is created and added to the
	 * hashmap. In contrary to {@link #getTriangle(SpaceNode, SpaceNode, SpaceNode)},
	 * the triangle is not removed from the hashmap if it was already part of the
	 * hashmap.
	 * @param a The first node incident to the requested triangle.
	 * @param b The second node incident to the requested triangle.
	 * @param c The third node incident to the requested triangle.
	 * @return A triangle with the requested three endpoints.
	 */
	Triangle3D<T> getTriangleWithoutRemoving(
			SpaceNode a, SpaceNode b, SpaceNode c);

	/**
	 * Returns an arbitrary tetrahedron which was created during the process of
	 * triangulation.
	 * @return A tetrahedron which was created during triangulation.
	 */
	Tetrahedron<T> getANewTetrahedron();

	/**
	 * Removes one tetrahedron from the triangulation and possibly all adjacent tetrahedra that have the same circumsphere as the
	 * first tetrahedron.
	 * @param startingTetrahedron The first tetrahedron to remove.
	 */
	void removeAllTetrahedraInSphere(
			Tetrahedron startingTetrahedron);

	/**
	 * Finishes an incomplete triangulation. Before calling this function the open triangle organizer
	 * must be informed about all open triangles delimiting the non-triangulated volume in the
	 * tetrahedralization.
	 * <p>
	 * This function will successively pick an open triangle and find the corresponding point for this
	 * triangle. The triangle node organizer (given as argument to {@link #OpenTriangleOrganizer(int, AbstractTriangulationNodeOrganizer)})
	 * will provide the order in which the candidate nodes are tested.
	 * <p>
	 * All tetrahedra created by this function will be reported in
	 *
	 */
	void triangulate();
}
