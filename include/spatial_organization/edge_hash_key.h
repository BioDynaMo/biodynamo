#ifndef SPATIAL_ORGANIZATION_EDGE_HASH_KEY_H_
#define SPATIAL_ORGANIZATION_EDGE_HASH_KEY_H_

#include <memory>
#include <string>
#include <array>

namespace bdm {
namespace spatial_organization {

template<class T> class SpaceNode;
template<class T> class Tetrahedron;
template<class T> struct EdgeHashKeyHash;
template<class T> struct EdgeHashKeyEqual;

/**
 * Class to provide hash values for edges between two nodes.
 * A simple algorithm is used to calculate a hash value for a pair of nodes.
 * The hash values do not depend on a direction of an edge. Therefore, edges can
 * be reliably found back even when they were initialized with the same endpoints
 * in opposite order.
 *
 * This class also provides basic functions for comparisons of two edges.
 *
 * @param <T> The type of user objects associated with nodes in this triangulation.
 */
template<class T>
class EdgeHashKey {
 public:
  friend struct EdgeHashKeyHash<T> ;
  friend struct EdgeHashKeyEqual<T> ;

  /**
   * Creates a new instance of <code>EdgeHashKey</code> with the two endpoints
   * <code>a</code> and <code>b</code>. A third node is expected as additional
   * parameter which defines the direction of the non-open side of this edge.
   * @param a The first endpoint of the represented edge.
   * @param b The second enpoint of the represented edge.
   * @param opposite_node A node on the non-open side of this edge.
   */
  EdgeHashKey(SpaceNode<T>* a, SpaceNode<T>* b, SpaceNode<T>* opposite_node);

  EdgeHashKey(const EdgeHashKey& other);

  EdgeHashKey<T>& operator=(const EdgeHashKey<T>& rhs);

  ~EdgeHashKey();

  /**
   * Creates a string representation of this object.
   */
  std::string toString() const;

  /**
   * Creates a integer representation of this object.
   */
  int hashCode() const;

  /**
   * Compares the represented edge with another object.
   * @param other the object with which this edge should be compared.
   * @return <code>true</code>, if other has the same endpoints as this edge.
   * <code>false</code> is returned in all other cases.
   */
  bool equalTo(const std::shared_ptr<EdgeHashKey<T>>& other) const;

  /**
   * computes the cosine between this edge to another point measured at the
   * first endpoint of this edge.
   * @param fourth_point The other point.
   * @return The cosine between this edge and an edge between the first
   * endpoint of this edge and <code>fourthPoint</code>.
   */
  double getCosine(const std::array<double, 3>& fourth_point) const;

  /**
   * @return endpoint A of this edge
   */
  SpaceNode<T>* getEndpointA() const;

  /**
   * @return endpoint B of this edge
   */
  SpaceNode<T>* getEndpointB() const;

  /**
   * Returns the opposite node of a given node if the latter is incident to this edge.
   * @param node The given node.
   * @return The incident node opposite to <code>node</code>.
   */
  SpaceNode<T>* oppositeNode(SpaceNode<T>* node) const;

 private:
  EdgeHashKey() = delete;

  /**
   * The endpoints of the edge for which a hash value should be calculated.
   */
  SpaceNode<T>* a_ = nullptr;
  SpaceNode<T>* b_ = nullptr;

  /**
   * The vector connecting the positions of <code>a</code> and <code>b</code>.
   */
  std::array<double, 3> ab_;

  /**
   * A vector which is orthogonal to <code>ab</code> and points into the direction of
   * the non-open side of this edge.
   */
  std::array<double, 3> last_normal_vector_;

  /**
   * The hash value associated with this edge.
   */
  int hash_code_;
};

template<class T>
struct EdgeHashKeyHash {
  std::size_t operator()(const EdgeHashKey<T>& element) const;
};

template<class T>
struct EdgeHashKeyEqual {
  bool operator()(const EdgeHashKey<T>& lhs, const EdgeHashKey<T>& rhs) const;
};

}  // namespace spatial_organization
}  // namespace bdm

#endif  // SPATIAL_ORGANIZATION_EDGE_HASH_KEY_H_
