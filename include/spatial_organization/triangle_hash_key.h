#ifndef SPATIAL_ORGANIZATION_TRIANGLE_HASH_KEY_H_
#define SPATIAL_ORGANIZATION_TRIANGLE_HASH_KEY_H_

#include <memory>
#include <string>
#include <array>

namespace bdm {
namespace spatial_organization {

template<class T> class SpaceNode;
template<class T> class Tetrahedron;
template<class T> struct TriangleHashKeyHash;
template<class T> struct TriangleHashKeyEqual;

/**
 * Class to provide hash values for triangles.
 * A simple algorithm is used to calculate a hash value for a triplet of nodes.
 * The calculated hash values do not depend on the order of nodes. Therefore, triangles can
 * be reliably found back even when they were initialized with the same endpoints in a different order.
 *
 * @param <T> The type of user objects associated with nodes in the triangulation.
 */
template<class T>
class TriangleHashKey {
 public:
  friend struct TriangleHashKeyHash<T> ;
  friend struct TriangleHashKeyEqual<T> ;

  /**
   * Creates a hash value for a triplet of nodes.
   * @param a The first node.
   * @param b The second node.
   * @param c The third node.
   */
  TriangleHashKey(SpaceNode<T>* a, SpaceNode<T>* b, SpaceNode<T>* c);

  TriangleHashKey(const TriangleHashKey& other);

  ~TriangleHashKey();

  /**
   * Creates a integer representation of this object.
   */
  int hashCode() const;

  /**
   * Determines whether this object is equal to another one.
   * @param other The object with which this <code>TriangleHashKey</code> should be compared.
   * @return <code>true</code>, iff <code>other</code> refers to the same three points as this
   * <code>TriangleHashKey</code>.
   */
  bool equalTo(const std::shared_ptr<TriangleHashKey<T>>& other) const;

 private:
  TriangleHashKey() = delete;
  TriangleHashKey& operator=(const TriangleHashKey&) = delete;

  /**
   * The points of the triangle for which a hash value should be calculated.
   */
  SpaceNode<T>* a_ = nullptr;
  SpaceNode<T>* b_ = nullptr;
  SpaceNode<T>* c_ = nullptr;

  /**
   * The hash value associated with this edge.
   */
  int hash_code_;

  /**
   * Internal function to create the hash value. Three integer values are needed to compute this value.
   * @param a_id The first node ID.
   * @param b_id The second node ID.
   * @param c_id The third node ID.
   */
  void createHashCode(int a_id, int b_id, int c_id);
};

template<class T>
struct TriangleHashKeyHash {
  std::size_t operator()(const TriangleHashKey<T>& key) const;
};

template<class T>
struct TriangleHashKeyEqual {
  bool operator()(const TriangleHashKey<T>& lhs, const TriangleHashKey<T>& rhs) const;
};

}  // namespace spatial_organization
}  // namespace bdm

#endif  // SPATIAL_ORGANIZATION_TRIANGLE_HASH_KEY_H_
