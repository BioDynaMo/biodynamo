#include "spatial_organization/edge.h"

#include "physics/physical_node.h"
#include "string_util.h"
#include "spatial_organization/tetrahedron.h"

namespace cx3d {
namespace spatial_organization {

template<class T>
Edge<T>::Edge(SpaceNode<T>* a, SpaceNode<T>* b)
    : a_(a),
      b_(b),
      adjacent_tetrahedra_(),
      cross_section_area_(0.0) {
}

template<class T>
Edge<T>::~Edge() {
}

template<class T>
SpatialOrganizationNode<T>* Edge<T>::getOpposite(const SpatialOrganizationNode<T>* node) const {
  if (node == a_) {
    return b_;
  } else if (node == b_) {
    return a_;
  } else {
    throw std::invalid_argument("The edge " + toString() + " is not adjacent to the node " + node->toString());
  }
}

template<class T>
T* Edge<T>::getOppositeElement(T* element) const {
  if (a_ != nullptr && b_ != nullptr) {
    if (element == a_->getUserObject()) {
      return b_->getUserObject();
    } else {
      return a_->getUserObject();
    }
  }
  return nullptr;
}

template<class T>
T* Edge<T>::getFirstElement() const {
  return a_->getUserObject();
}

template<class T>
T* Edge<T>::getSecondElement() const {
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
  str_stream << "Edge";
  str_stream << StringUtil::toStr(cross_section_area_);
//  str_stream << " - ";
//  str_stream << StringUtil::toStr(a_);
//  str_stream << " - ";
//  str_stream << StringUtil::toStr(b_);
//  str_stream << " - ";
//  str_stream << StringUtil::toStr(adjacent_tetrahedra_);
  str_stream << ")";
  return str_stream.str();
}

template<class T>
bool Edge<T>::equals(SpaceNode<T>* a, SpaceNode<T>* b) const {
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
  if (a_ != nullptr) {
    a_->removeEdge(this);
  }
  if (b_ != nullptr) {
    b_->removeEdge(this);
  }
}

template<class T>
std::list<std::shared_ptr<Tetrahedron<T>> > Edge<T>::getAdjacentTetrahedra() const {
  return adjacent_tetrahedra_;
}

template<class T>
void Edge<T>::changeCrossSectionArea(double change) {
  cross_section_area_ += change;
}

template class Edge<cx3d::physics::PhysicalNode> ;

}  // namespace spatial_organization
}  // namespace cx3d
