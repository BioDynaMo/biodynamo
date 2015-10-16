#include "spatial_organization/edge.h"

#include <sstream>
#include <stdexcept>

#include "physical_node.h"
#include "string_util.h"
#include "spatial_organization/space_node.h"
#include "spatial_organization/tetrahedron.h"

namespace cx3d {
namespace spatial_organization {

using cx3d::StringUtil;

using cx3d::spatial_organization::SpatialOrganizationEdge;
using cx3d::spatial_organization::Edge;

template<class T>
Edge<T>::Edge(const std::shared_ptr<SpaceNode<T>>& a, const std::shared_ptr<SpaceNode<T>>& b)
    : a_(a),
      b_(b),
      cross_section_area_(0.0),
      adjacent_tetrahedra_() {
}

template<class T>
std::shared_ptr<SpaceNode<T>> Edge<T>::getOpposite(
    const std::shared_ptr<SpaceNode<T>>& node) const {
  if (node == a_) {
    return b_;
  } else if (node == b_) {
    return a_;
  } else {
    throw std::invalid_argument(
        "The edge " + toString() + " is not adjacent to the node " + node->toString());
  }
}

template<class T>
std::shared_ptr<T> Edge<T>::getOppositeElement(const std::shared_ptr<T>& element) const {
  if (a_.get() != nullptr && b_.get() != nullptr) {
    if (element == a_->getUserObject()) {
      return b_->getUserObject();
    } else {
      return a_->getUserObject();
    }
  }
  return std::shared_ptr<T>(nullptr);
}

template<class T>
std::shared_ptr<T> Edge<T>::getFirstElement() const {
  return a_->getUserObject();
}

template<class T>
std::shared_ptr<T> Edge<T>::getSecondElement() const {
  return b_->getUserObject();
}

template<class T>
double Edge<T>::getCrossSection() const {
  return cross_section_area_;
}

template<class T>
const std::string Edge<T>::toString() const {
  std::ostringstream str_stream;
  str_stream << "(";
  str_stream << StringUtil::toStr(cross_section_area_);
  str_stream << " - ";
  str_stream << a_->toString();
  str_stream << " - ";
  str_stream << b_->toString();
  str_stream << " - ";
  str_stream << StringUtil::toStr(adjacent_tetrahedra_);
  str_stream << ")";
  return str_stream.str();
}

template<class T>
bool Edge<T>::equalTo(const std::shared_ptr<Edge<T>>& other) {
  return other.get() == this;
}

template<class T>
bool Edge<T>::equals(const std::shared_ptr<SpaceNode<T>>& a,
                     const std::shared_ptr<SpaceNode<T>>& b) const {
  return ((a_ == a) && (b_ == b)) || ((b_ == a) && (a_ == b));
}

template<class T>
void Edge<T>::removeTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) {
  adjacent_tetrahedra_.remove(tetrahedron);
  if (adjacent_tetrahedra_.empty()) {
    remove();
  }
}

template<class T>
void Edge<T>::addTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) {
  adjacent_tetrahedra_.push_back(tetrahedron);
}

template<class T>
void Edge<T>::remove() {
  if (a_.get() != nullptr) {
    a_->removeEdge(this->shared_from_this());
  }
  if (b_.get() != nullptr) {
    b_->removeEdge(this->shared_from_this());
  }
}

template<class T>
std::list<std::shared_ptr<Tetrahedron<T>> >Edge<T>::getAdjacentTetrahedra() const {
  return adjacent_tetrahedra_;
}

template<class T>
void Edge<T>::changeCrossSectionArea(double change) {
  cross_section_area_ += change;
}
template<class T>
void Edge<T>::initializationHelper() {
  if (a_.get() != nullptr) {
    a_->addEdge(this->shared_from_this());
  }
  if (b_.get() != nullptr) {
    b_->addEdge(this->shared_from_this());
  }
}

template class Edge<cx3d::PhysicalNode>;

}  // namespace spatial_organization
}  // namespace cx3d

