#include "spatial_organization/plane_3d.h"

#include "matrix.h"
#include "spatial_organization/space_node.h"
#include "spatial_organization/tetrahedron.h"
#include "spatial_organization/exact_vector.h"
#include "spatial_organization/rational.h"
#include "physics/physical_node.h"

namespace cx3d {
namespace spatial_organization {

template<class T>
Plane3D<T>::Plane3D()
    : normal_vector_ { 0.0, 0.0, 0.0 },
      offset_(0.0),
      tolerance_(0.0),
      normal_vector_updated_(false) {
}

template<class T>
Plane3D<T>::Plane3D(const std::array<double, 3>& normal_vector, double offset)
    : normal_vector_(normal_vector),
      offset_(offset),
      tolerance_(Matrix::dot(normal_vector_, normal_vector_) * 0.000000001),
      normal_vector_updated_(false) {
}

template<class T>
Plane3D<T>::Plane3D(const std::array<double, 3>& direction_vector_1,
                    const std::array<double, 3>& direction_vector_2,
                    const std::array<double, 3>& position_vector, bool normalize)
    : normal_vector_({ 0.0, 0.0, 0.0 }),
      offset_(0.0),
      tolerance_(0.0),
      normal_vector_updated_(false) {
  initPlane(direction_vector_1, direction_vector_2, position_vector, normalize);
}

template<class T>
Plane3D<T>::Plane3D(const std::array<double, 3>& direction_vector_1,
                    const std::array<double, 3>& direction_vector_2,
                    const std::array<double, 3>& position_vector)
    : normal_vector_({ 0.0, 0.0, 0.0 }),
      offset_(0.0),
      tolerance_(0.0),
      normal_vector_updated_(false) {
  initPlane(direction_vector_1, direction_vector_2, position_vector, Plane3D::normalize_);
}

template<class T>
Plane3D<T>::Plane3D(const std::array<SpaceNode<T>*, 4>& nodes,
                    SpaceNode<T>* non_used_node, bool normalize)
    : normal_vector_({ 0.0, 0.0, 0.0 }),
      offset_(0.0),
      tolerance_(0.0),
      normal_vector_updated_(false) {
  int first = (nodes[0] == non_used_node) ? 1 : 0;

  int idx = 2;
  if (first == 0 && nodes[1] != non_used_node) {
    idx = 1;
  }
  auto direction_vec_1 = Matrix::subtract(nodes[first]->getPosition(), nodes[idx]->getPosition());

  idx = 3;
  if (nodes[3] == non_used_node) {
    idx = 2;
  }
  auto direction_vec_2 = Matrix::subtract(nodes[first]->getPosition(), nodes[idx]->getPosition());

  initPlane(direction_vec_1, direction_vec_2, nodes[first]->getPosition(), normalize);
  defineUpperSide(non_used_node->getPosition());
}

template<class T>
Plane3D<T>::Plane3D(const std::array<SpaceNode<T>*, 4>& nodes,
                    SpaceNode<T>* non_used_node)
    : Plane3D(nodes, non_used_node, Plane3D::normalize_) {
}

template<class T>
Plane3D<T>::Plane3D(const std::shared_ptr<Tetrahedron<T>>& tetrahedron,
                    SpaceNode<T>* non_used_node)
    : Plane3D(tetrahedron->getAdjacentNodes(), non_used_node) {
}

template<class T>
void Plane3D<T>::initPlane(const std::array<double, 3>& direction_vector_1,
                           const std::array<double, 3>& direction_vector_2,
                           const std::array<double, 3>& position_vector, bool normalize) {
  if (!normal_vector_updated_) {
    normal_vector_updated_ = true;
    normal_vector_ = Matrix::crossProduct(direction_vector_1, direction_vector_2);
    tolerance_ = Matrix::dot(normal_vector_, normal_vector_) * 0.000000001;
    if (tolerance_ == 0.0) {
      throw std::range_error("tolerance was set to 0!");
    }
  }
  if (normalize) {
    double norm = Matrix::norm(normal_vector_);
    normal_vector_[0] /= norm;
    normal_vector_[1] /= norm;
    normal_vector_[2] /= norm;
    tolerance_ = 0.000000001;
  }
  offset_ = Matrix::dot(normal_vector_, position_vector);
}

template<class T>
void Plane3D<T>::changeUpperSide() {
  offset_ = -offset_;
  normal_vector_[0] = -normal_vector_[0];
  normal_vector_[1] = -normal_vector_[1];
  normal_vector_[2] = -normal_vector_[2];
}

template<class T>
void Plane3D<T>::defineUpperSide(const std::array<double, 3>& point) {
  if (Matrix::dot(point, normal_vector_) + tolerance_ < offset_) {
    changeUpperSide();
  }
}

template<class T>
int Plane3D<T>::orientation(const std::array<double, 3>& point_1,
                            const std::array<double, 3>& point_2) const {
  double dot_1 = Matrix::dot(point_1, normal_vector_);
  double dot_2 = Matrix::dot(point_2, normal_vector_);
  if (dot_1 > offset_ + tolerance_) {
    if (dot_2 < offset_ - tolerance_) {
      return -1;
    } else if (dot_2 > offset_ + tolerance_) {
      return 1;
    } else {
      return orientationExact(point_1, point_2);
    }
  } else if (dot_1 < offset_ - tolerance_) {
    if (dot_2 > offset_ + tolerance_) {
      return -1;
    } else if (dot_2 < offset_ - tolerance_) {
      return 1;
    } else {
      return orientationExact(point_1, point_2);
    }
  } else {
    return orientationExact(point_1, point_2);
  }
}

template<class T>
bool Plane3D<T>::trulyOnSameSide(const std::array<double, 3>& point_1,
                                 const std::array<double, 3>& point_2) {
  return orientation(point_1, point_2) > 0;
}

template<class T>
bool Plane3D<T>::trulyOnDifferentSides(const std::array<double, 3>& point_1,
                                       const std::array<double, 3>& point_2) {
  return orientation(point_1, point_2) < 0;
}

template<class T>
bool Plane3D<T>::onSameSide(const std::array<double, 3>& point_1,
                            const std::array<double, 3>& point_2) const {
  return orientation(point_1, point_2) >= 0;
}

template<class T>
std::array<double, 3> Plane3D<T>::getNormalVector() {
  return normal_vector_;
}

template<class T>
int Plane3D<T>::orientationExact(const std::array<double, 3>& point_1,
                                 const std::array<double, 3>& point_2) const {
  auto exact_normal_vector = ExactVector::create(normal_vector_);
  auto offset = Rational::create(offset_);
  auto dot_1 = exact_normal_vector->dotProduct(ExactVector::create(point_1));
  auto dot_2 = exact_normal_vector->dotProduct(ExactVector::create(point_2));
  return dot_1->compareTo(offset) * dot_2->compareTo(offset);
}

// define templates that should be compiled
template class Plane3D<cx3d::physics::PhysicalNode>;

}  // namespace spatial_organization
}  // namespace cx3d

