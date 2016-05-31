#include "spatial_organization/abstract_triangulation_node_organizer.h"

#include "physics/physical_node.h"
#include "spatial_organization/triangle_3d.h"

namespace cx3d {
namespace spatial_organization {

template<typename T>
AbstractTriangulationNodeOrganizer<T>::AbstractTriangulationNodeOrganizer() {
}

template<typename T>
AbstractTriangulationNodeOrganizer<T>::~AbstractTriangulationNodeOrganizer() {
}

template<typename T>
void AbstractTriangulationNodeOrganizer<T>::addTriangleNodes(
    const std::shared_ptr<Triangle3D<T>>& triangle) {
  auto nodes = triangle->getNodes();
  addNode(nodes[1]);
  addNode(nodes[2]);
  addNode(nodes[0]);
}

template class AbstractTriangulationNodeOrganizer<cx3d::physics::PhysicalNode> ;

}  // namespace spatial_organization
}  // namespace cx3d