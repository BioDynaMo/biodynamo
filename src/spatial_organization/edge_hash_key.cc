#include "spatial_organization/edge_hash_key.h"

#include <sstream>

#include "physics/physical_node.h"
#include "matrix.h"

namespace bdm {
namespace spatial_organization {

template<class T>
EdgeHashKey<T>::EdgeHashKey(SpaceNode<T>* a, SpaceNode<T>* b, SpaceNode<T>* opposite_node)
    : a_(a),
      b_(b),
      ab_ { 0.0, 0.0, 0.0 },
      last_normal_vector_ { 0.0, 0.0, 0.0 },
      hash_code_(0) {
  ab_ = Matrix::subtract(b_->getPosition(), a_->getPosition());
  auto subtraction = Matrix::subtract(opposite_node->getPosition(), a_->getPosition());
  last_normal_vector_ = Matrix::normalize(Matrix::crossProduct(ab_, subtraction));
  hash_code_ = std::max(a_->getId(), b_->getId()) * 11 + std::min(a_->getId(), b_->getId()) * 31;
}

template<class T>
EdgeHashKey<T>::EdgeHashKey(const EdgeHashKey<T>& other) {
  hash_code_ = other.hash_code_;
}

template<class T>
EdgeHashKey<T>::~EdgeHashKey() {
}

template<class T>
EdgeHashKey<T>& EdgeHashKey<T>::operator=(const EdgeHashKey<T>& rhs) {
  if (this == &rhs) {
    return *this;
  }
  a_ = rhs.a_;
  b_ = rhs.b_;
  hash_code_ = rhs.hash_code_;
  ab_ = rhs.ab_;
  last_normal_vector_ = rhs.last_normal_vector_;
  return *this;
}

template<class T>
std::string EdgeHashKey<T>::toString() const {
  std::ostringstream str_stream;
  str_stream << "(";
  str_stream << a_->toString();
  str_stream << ", ";
  str_stream << b_->toString();
  str_stream << ")";
  return str_stream.str();
}

template<class T>
int EdgeHashKey<T>::hashCode() const {
  return hash_code_;
}

template<class T>
bool EdgeHashKey<T>::equalTo(const std::shared_ptr<EdgeHashKey<T>>& other) const {
  return other.get() == this;
}

template<class T>
double EdgeHashKey<T>::getCosine(const std::array<double, 3>& fourth_point) const {
  auto difference = Matrix::subtract(fourth_point, a_->getPosition());
  auto cross_product = Matrix::crossProduct(ab_, difference);
  auto normal = Matrix::normalize(cross_product);
  double cosine = Matrix::dot(normal, last_normal_vector_);
  if (cosine > 0.999999999) {
    return 1;
  } else if (cosine < -0.99999999) {
    return -1;
  }
  return cosine;
}

template<class T>
SpaceNode<T>* EdgeHashKey<T>::getEndpointA() const {
  return a_;
}

template<class T>
SpaceNode<T>* EdgeHashKey<T>::getEndpointB() const {
  return b_;
}

template<class T>
SpaceNode<T>* EdgeHashKey<T>::oppositeNode(SpaceNode<T>* node) const {
  if (node == a_) {
    return b_;
  } else if (node == b_) {
    return a_;
  } else {
    throw std::invalid_argument("Could not find an opposite node for" + node->toString());
  }
}

template<class T>
std::size_t EdgeHashKeyHash<T>::operator()(const EdgeHashKey<T>& element) const {
  return element.hash_code_;
}

template<class T>
bool EdgeHashKeyEqual<T>::operator()(const EdgeHashKey<T>& lhs, const EdgeHashKey<T>& rhs) const {
  return lhs.hash_code_ == rhs.hash_code_;
}

template class EdgeHashKey<bdm::physics::PhysicalNode> ;
template struct EdgeHashKeyHash<bdm::physics::PhysicalNode> ;
template struct EdgeHashKeyEqual<bdm::physics::PhysicalNode> ;

}  // namespace spatial_organization
}  // namespace bdm
