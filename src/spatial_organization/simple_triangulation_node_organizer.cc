#include "spatial_organization/simple_triangulation_node_organizer.h"

#include <sstream>

#include "string_util.h"
#include "physics/physical_node.h"
#include "spatial_organization/triangle_3d.h"
#include "spatial_organization/binary_tree_element.h"

namespace bdm {
namespace spatial_organization {

template<class T>
std::shared_ptr<SimpleTriangulationNodeOrganizer<T>> SimpleTriangulationNodeOrganizer<T>::create() {
  return std::shared_ptr < SimpleTriangulationNodeOrganizer < T >> (new SimpleTriangulationNodeOrganizer());
}

template<class T>
SimpleTriangulationNodeOrganizer<T>::SimpleTriangulationNodeOrganizer()
    : tree_head_ { BinaryTreeElement < T > ::generateTreeHead() } {
}

template<class T>
SimpleTriangulationNodeOrganizer<T>::~SimpleTriangulationNodeOrganizer() {
  delete tree_head_;
}

template<class T>
std::list<SpaceNode<T>*> SimpleTriangulationNodeOrganizer<T>::getNodes(SpaceNode<T>* reference_point) {
  return tree_head_->inOrderTraversal();
}

template<class T>
void SimpleTriangulationNodeOrganizer<T>::removeNode(SpaceNode<T>* node) {
  tree_head_->remove(node, nullptr);
}

template<class T>
void SimpleTriangulationNodeOrganizer<T>::addNode(SpaceNode<T>* node) {
  tree_head_->insert(node);
}

template<class T>
SpaceNode<T>* SimpleTriangulationNodeOrganizer<T>::getFirstNode() const {
  return tree_head_->bigger_->content_;
}

template<class T>
std::string SimpleTriangulationNodeOrganizer<T>::toString() const {
  std::stringstream str_stream;
  str_stream << "[";
  str_stream << StringUtil::toStr(tree_head_);
  str_stream << "]";
  return str_stream.str();
}

template<typename T>
bool SimpleTriangulationNodeOrganizer<T>::equalTo(const std::shared_ptr<SimpleTriangulationNodeOrganizer<T>>& other) {
  return this == other.get();
}

template class SimpleTriangulationNodeOrganizer<bdm::physics::PhysicalNode> ;

}  // namespace spatial_organization
}  // namespace bdm

