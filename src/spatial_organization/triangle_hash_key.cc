#include "spatial_organization/triangle_hash_key.h"

#include <cmath>

#include "physics/physical_node.h"
#include "spatial_organization/space_node.h"

namespace cx3d {
namespace spatial_organization {

template<class T>
TriangleHashKey<T>::TriangleHashKey(SpaceNode<T>* a,
                                    SpaceNode<T>* b,
                                    SpaceNode<T>* c)
    : a_(a),
      b_(b),
      c_(c),
      hash_code_(0) {
  int a_id = a_ != nullptr ? a_->getId() : -1;
  int b_id = b_ != nullptr ? b_->getId() : -1;
  int c_id = c_ != nullptr ? c_->getId() : -1;
  createHashCode(a_id, b_id, c_id);
}

template<class T>
TriangleHashKey<T>::TriangleHashKey(const TriangleHashKey& other) {
    hash_code_ = other.hash_code_;
    a_ = other.a_;
    b_ = other.b_;
    c_ = other.c_;
}

template<class T>
TriangleHashKey<T>::~TriangleHashKey() {
}

template<class T>
int TriangleHashKey<T>::hashCode() const {
  return hash_code_;
}

template<class T>
bool TriangleHashKey<T>::equalTo(const std::shared_ptr<TriangleHashKey<T>>& other) const {
  return (a_ == other->a_
      && ((b_ == other->b_ && c_ == other->c_) || (b_ == other->c_ && c_ == other->b_)))
      || (a_ == other->b_
          && ((b_ == other->a_ && c_ == other->c_) || (b_ == other->c_ && c_ == other->a_)))
      || (a_ == other->c_
          && ((b_ == other->a_ && c_ == other->b_) || (b_ == other->b_ && c_ == other->a_)));
}

template<class T>
void TriangleHashKey<T>::createHashCode(int a_id, int b_id, int c_id) {
  int min = std::min(a_id, std::min(b_id, c_id));
  int max = std::max(a_id, std::max(b_id, c_id));
  hash_code_ = (min * 31 + max * 11 + a_id + b_id + c_id) % 2000000001;
}

template<class T>
std::size_t TriangleHashKeyHash<T>::operator()(const TriangleHashKey<T>& key) const {
  return key.hash_code_;
}

template<class T>
bool TriangleHashKeyEqual<T>::operator()(const TriangleHashKey<T>& lhs,
                                         const TriangleHashKey<T>& rhs) const {
  return (lhs.a_ == rhs.a_
      && ((lhs.b_ == rhs.b_ && lhs.c_ == rhs.c_)
          || (lhs.b_ == rhs.c_ && lhs.c_ == rhs.b_)))
      || (lhs.a_ == rhs.b_
          && ((lhs.b_ == rhs.a_ && lhs.c_ == rhs.c_)
              || (lhs.b_ == rhs.c_ && lhs.c_ == rhs.a_)))
      || (lhs.a_ == rhs.c_
          && ((lhs.b_ == rhs.a_ && lhs.c_ == rhs.b_)
              || (lhs.b_ == rhs.b_ && lhs.c_ == rhs.a_)));
}

template class TriangleHashKey<cx3d::physics::PhysicalNode>;
template struct TriangleHashKeyHash<cx3d::physics::PhysicalNode>;
template struct TriangleHashKeyEqual<cx3d::physics::PhysicalNode>;

}  // namespace spatial_organization
}  // namespace cx3d

