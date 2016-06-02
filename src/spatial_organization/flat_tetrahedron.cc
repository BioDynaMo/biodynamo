#include "spatial_organization/flat_tetrahedron.h"

#include "physics/physical_node.h"
#include "spatial_organization/triangle_3d.h"
#include "spatial_organization/open_triangle_organizer.h"

namespace bdm {
namespace spatial_organization {

template<class T>
std::shared_ptr<Tetrahedron<T>> FlatTetrahedron<T>::create(const std::shared_ptr<Triangle3D<T>>& one_triangle,
                                                           SpaceNode<T>* fourth_point,
                                                           const std::shared_ptr<OpenTriangleOrganizer<T>>& oto) {
  FlatTetrahedron < T > *tetrahedron = new FlatTetrahedron<T>();
  tetrahedron->initializationHelper(one_triangle, fourth_point, oto);
  std::shared_ptr < Tetrahedron < T >> ret(tetrahedron);
  return ret;
}

template<class T>
std::shared_ptr<Tetrahedron<T>> FlatTetrahedron<T>::create(const std::shared_ptr<Triangle3D<T>>& triangle_a,
                                                           const std::shared_ptr<Triangle3D<T>>& triangle_b,
                                                           const std::shared_ptr<Triangle3D<T>>& triangle_c,
                                                           const std::shared_ptr<Triangle3D<T>>& triangle_d,
                                                           SpaceNode<T>* node_a, SpaceNode<T>* node_b,
                                                           SpaceNode<T>* node_c, SpaceNode<T>* node_d) {
  FlatTetrahedron < T > *tetrahedron = new FlatTetrahedron<T>();
  tetrahedron->initializationHelper(triangle_a, triangle_b, triangle_c, triangle_d, node_a, node_b, node_c, node_d);
  std::shared_ptr < Tetrahedron < T >> ret(tetrahedron);
  return ret;
}

template<class T>
FlatTetrahedron<T>::~FlatTetrahedron() {
}

template<class T>
void FlatTetrahedron<T>::updateCirumSphereAfterNodeMovement(SpaceNode<T>* moved_node) {
  for (size_t i = 0; i < 4; i++) {
    if (this->adjacent_nodes_[i] != moved_node) {
      this->adjacent_triangles_[i]->informAboutNodeMovement();
    }
  }
}

template<class T>
void FlatTetrahedron<T>::calculateVolume() {
  this->volume_ = 0.0;
}

template<class T>
void FlatTetrahedron<T>::updateCrossSectionAreas() {
  for (size_t i = 0; i < 6; i++) {
    this->changeCrossSection(i, 0.0);
  }
}

template<class T>
void FlatTetrahedron<T>::calculateCircumSphere() {
}

template<class T>
bool FlatTetrahedron<T>::isFlat() const {
  return true;
}

template<class T>
int FlatTetrahedron<T>::orientation(const std::array<double, 3>& point) {
  this->adjacent_triangles_[0]->updatePlaneEquationIfNecessary();
  int orientation = this->adjacent_triangles_[0]->orientation(point, point);
  if (orientation == 0) {
    int memory = -1;
    for (size_t i = 0; i < 4; i++) {
      if (this->adjacent_triangles_[i] != nullptr) {
        int dummy = this->adjacent_triangles_[i]->circleOrientation(point);
        if (dummy == 1) {
          return 1;
        } else if (dummy == 0) {
          memory = 0;
        }
      }
    }
    return memory;
  } else {
    return orientation;
  }
}

template<class T>
bool FlatTetrahedron<T>::isTrulyInsideSphere(const std::array<double, 3>& point) {
  return orientation(point) > 0;
}

template<class T>
bool FlatTetrahedron<T>::isInsideSphere(const std::array<double, 3>& point) {
  return orientation(point) >= 0;
}

template<class T>
bool FlatTetrahedron<T>::isPointInConvexPosition(const std::array<double, 3>& point,
                                                 size_t connecting_triangle_number) const {
  this->adjacent_triangles_[0]->updatePlaneEquationIfNecessary();
  return this->adjacent_triangles_[0]->orientation(point, point) == 0;
}

template<class T>
int FlatTetrahedron<T>::isInConvexPosition(const std::array<double, 3>& point,
                                           size_t connecting_triangle_number) const {
  this->adjacent_triangles_[0]->updatePlaneEquationIfNecessary();
  if (this->adjacent_triangles_[0]->orientation(point, point) == 0) {
    return 0;
  } else {
    return -1;
  }
}

template<class T>
FlatTetrahedron<T>::FlatTetrahedron()
    : Tetrahedron<T>() {
}

template class FlatTetrahedron<bdm::physics::PhysicalNode> ;

}  // namespace spatial_organization
}  // namespace bdm
