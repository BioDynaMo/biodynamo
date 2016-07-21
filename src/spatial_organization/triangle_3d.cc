#include "spatial_organization/triangle_3d.h"

#include <limits>

#include "matrix.h"
#include "string_util.h"
#include "spatial_organization/tetrahedron.h"

namespace bdm {
namespace spatial_organization {

using std::shared_ptr;

template<class T>
std::shared_ptr<Triangle3D<T>> Triangle3D<T>::create(SpaceNode<T>* sn_1, SpaceNode<T>* sn_2, SpaceNode<T>* sn_3,
                                                     const std::shared_ptr<Tetrahedron<T>>& tetrahedron_1,
                                                     const std::shared_ptr<Tetrahedron<T>>& tetrahedron_2) {
  std::shared_ptr < Triangle3D < T >> triangle(new Triangle3D(sn_1, sn_2, sn_3, tetrahedron_1, tetrahedron_2));
  return triangle;
}

template<class T>
std::array<double, 3> Triangle3D<T>::calculate3PlaneXPoint(const std::array<std::array<double, 3>, 3>& normals,
                                                           const std::array<double, 3>& offsets, double normal_det) {
  std::array<double, 3> result;
  if (normal_det != 0.0) {
    auto vec_1 = Matrix::scalarMult(offsets[0], Matrix::crossProduct(normals[1], normals[2]));
    auto vec_2 = Matrix::scalarMult(offsets[1], Matrix::crossProduct(normals[2], normals[0]));
    auto vec_3 = Matrix::scalarMult(offsets[2], Matrix::crossProduct(normals[0], normals[1]));

    auto sum_vec = Matrix::add(vec_1, Matrix::add(vec_2, vec_3));
    result = Matrix::scalarMult(1 / normal_det, sum_vec);
  } else {
    double max_value = std::numeric_limits<double>::max();
    for (int i = 0; i < 3; i++) {
      result[i] = max_value;
    }
  }
  return result;
}

template<class T>
std::array<double, 3> Triangle3D<T>::calculate3PlaneXPoint(const std::array<std::array<double, 3>, 3>& normals,
                                                           const std::array<double, 3>& offsets) {
  return calculate3PlaneXPoint(normals, offsets, Matrix::det(normals));
}

template<class T>
std::shared_ptr<ExactVector> Triangle3D<T>::calculate3PlaneXPoint(
    const std::array<std::shared_ptr<ExactVector>, 3>& normals, const std::array<std::shared_ptr<Rational>, 3>& offsets,
    const std::shared_ptr<Rational>& normal_det) {
  if (!normal_det->isZero()) {
    auto ret = normals[1]->crossProduct(normals[2])->multiplyBy(offsets[0])->increaseBy(
        normals[2]->crossProduct(normals[0])->multiplyBy(offsets[1])->increaseBy(
            normals[0]->crossProduct(normals[1])->multiplyBy(offsets[2])))->divideBy(normal_det);
    return ret;
  } else {
    double max_value = std::numeric_limits<double>::max();
    std::array<std::shared_ptr<Rational>, 3> rationals;
    for (int i = 0; i < 3; i++) {
      rationals[i] = Rational::create(max_value, 1);
    }
    auto ret = ExactVector::create(rationals);
    return ret;
  }
}

template<class T>
Triangle3D<T>::Triangle3D(SpaceNode<T>* sn_1, SpaceNode<T>* sn_2, SpaceNode<T>* sn_3,
                          const std::shared_ptr<Tetrahedron<T>>& tetrahedron_1,
                          const std::shared_ptr<Tetrahedron<T>>& tetrahedron_2)
    : Plane3D<T>(),
      nodes_( { sn_1, sn_2, sn_3 }),
      circum_center_ { 0.0, 0.0, 0.0 },
      plane_updated_(false),
      circum_center_updated_(false),
      upper_side_positive_(true),
      connection_checked_(-1) {

  if (sn_2 == nullptr) {
    nodes_[1] = sn_1;
    nodes_[0] = nullptr;
  }
  if (sn_3 == nullptr) {
    nodes_[2] = sn_1;
    nodes_[0] = nullptr;
  }
}

template<class T>
bool Triangle3D<T>::isSimilarTo(const std::shared_ptr<Triangle3D<T>>& other_triangle) const {
  auto other_nodes = other_triangle->getNodes();
  return isAdjacentTo(other_nodes[0]) && isAdjacentTo(other_nodes[1]) && isAdjacentTo(other_nodes[2]);
}

template<class T>
double Triangle3D<T>::getSDDistance(const std::array<double, 3>& fourth_point) const {
  if (!isInfinite() && onUpperSide(fourth_point)) {
    double sd_distance = calculateSDDistance(fourth_point);
    if (sd_distance != std::numeric_limits<double>::max()) {
      return (upper_side_positive_) ? sd_distance : -sd_distance;
    } else {
      return std::numeric_limits<double>::max();
    }
  } else {
    return std::numeric_limits<double>::max();
  }
}

template<class T>
std::shared_ptr<Rational> Triangle3D<T>::getSDDistanceExact(const std::array<double, 3>& fourth_point) const {
  if (!isInfinite() && onUpperSide(fourth_point)) {
    std::array<std::shared_ptr<ExactVector>, 4> points;
    std::array<std::shared_ptr<ExactVector>, 3> points_3;
    for (int i = 0; i < 3; i++) {
      points[i] = ExactVector::create(nodes_[i]->getPosition());
      points_3[i] = points[i];
    }
    points[3] = ExactVector::create(fourth_point);
    auto normal_vector = calculateExactNormalVector(points_3);
    if (normal_vector->dotProduct(ExactVector::create(this->normal_vector_))->compareTo(Rational::create(0, 1)) < 0) {
      normal_vector->negate();
    }
    if (upper_side_positive_) {
      return calculateSDDistanceExact(points, normal_vector);
    } else {
      return calculateSDDistanceExact(points, normal_vector)->negate();
    }
  } else {
    return Rational::create(std::numeric_limits < uint64_t > ::max(), 1);
  }
}

template<class T>
std::array<double, 3> Triangle3D<T>::calculateCircumSphereCenter(const std::array<double, 3>& fourth_point) const {
  if (!isInfinite()) {
    double sd = calculateSDDistance(fourth_point);
    return Matrix::add(circum_center_, Matrix::scalarMult(sd, this->normal_vector_));
  }
  throw std::logic_error("could not calculate circum sphere, because triangle is infinite");
}

template<class T>
std::array<double, 3> Triangle3D<T>::calculateCircumSphereCenterIfEasy(
    const std::array<double, 3>& fourth_point) const {
  if (circum_center_updated_) {
    return calculateCircumSphereCenter(fourth_point);
  }
  throw std::logic_error("could not calculate circum sphere, because triangle is infinite");
}

template<class T>
void Triangle3D<T>::informAboutNodeMovement() {
  circum_center_updated_ = false;
  plane_updated_ = false;
  this->normal_vector_updated_ = false;
}

template<class T>
void Triangle3D<T>::updatePlaneEquationIfNecessary() {
  if (!plane_updated_ && !isInfinite()) {
    auto node_0_position = nodes_[0]->getPosition();
    auto diff_1 = Matrix::subtract(nodes_[1]->getPosition(), node_0_position);
    auto diff_2 = Matrix::subtract(nodes_[2]->getPosition(), node_0_position);
    this->initPlane(diff_1, diff_2, node_0_position, false);
    plane_updated_ = true;
  }
}

template<class T>
void Triangle3D<T>::update() {
  updateCircumCenterIfNecessary();
  updatePlaneEquationIfNecessary();
}

template<class T>
int Triangle3D<T>::orientationExact(const std::array<double, 3>& point_1, const std::array<double, 3>& point_2) const {
  auto points = getExactPositionVectors();
  auto normal_vector = points[1]->subtract(points[0])->crossProduct(points[2]->subtract(points[0]));
  auto offset = normal_vector->dotProduct(points[0]);
  return normal_vector->dotProduct(ExactVector::create(point_1))->compareTo(offset)
      * normal_vector->dotProduct(ExactVector::create(point_2))->compareTo(offset);
}

template<class T>
int Triangle3D<T>::circleOrientation(const std::array<double, 3>& point) {
  updateCircumCenterIfNecessary();
  auto dummy = Matrix::subtract(point, circum_center_);
  double squared_distance = Matrix::dot(dummy, dummy);
  auto radial = Matrix::subtract(nodes_[0]->getPosition(), circum_center_);
  double squared_radius = Matrix::dot(radial, radial);
  double tolerance = squared_radius * Param::kDefaultTolerance;
  if (squared_distance < squared_radius + tolerance) {
    if (squared_distance > squared_radius - tolerance) {
      auto points = getExactPositionVectors();
      auto circum_center = calculateCircumCenterExact(points, calculateExactNormalVector(points));
      auto point_distance = circum_center->subtract(ExactVector::create(point))->squaredLength();
      auto squared_radius_x = circum_center->subtract(points[0])->squaredLength();
      return squared_radius_x->compareTo(point_distance);
    } else {
      return 1;
    }
  } else {
    return -1;
  }
}

template<class T>
std::shared_ptr<Tetrahedron<T>> Triangle3D<T>::getOppositeTetrahedron(
    const std::shared_ptr<Tetrahedron<T>>& incident_tetrahedron) const {
  if (adjacent_tetrahedra_[0].lock() == incident_tetrahedron) {
    return adjacent_tetrahedra_[1].lock();
  } else if (adjacent_tetrahedra_[1].lock() == incident_tetrahedron) {
    return adjacent_tetrahedra_[0].lock();
  } else {
    std::cout << __FUNCTION__ << std::endl;
    throw std::invalid_argument("Tetrahedron not known!");
  }
}

template<class T>
void Triangle3D<T>::removeTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) {
  if (adjacent_tetrahedra_[0].lock() == tetrahedron) {
    adjacent_tetrahedra_[0] = std::shared_ptr < Tetrahedron < T >> (nullptr);
  } else {
    adjacent_tetrahedra_[1] = std::shared_ptr < Tetrahedron < T >> (nullptr);
  }
}

template<class T>
bool Triangle3D<T>::isOpenToSide(const std::array<double, 3>& point) {
  if (adjacent_tetrahedra_[0].lock().get() == nullptr) {
    if (adjacent_tetrahedra_[1].lock().get() == nullptr) {
      return true;
    } else {
      if (adjacent_tetrahedra_[1].lock()->isInfinite()) {
        return true;
      }
      auto position = adjacent_tetrahedra_[1].lock()->getOppositeNode(this->shared_from_this())->getPosition();
      return !(this->onSameSide(position, point));
    }
  } else if (adjacent_tetrahedra_[1].lock().get() == nullptr) {
    if (adjacent_tetrahedra_[0].lock()->isInfinite()) {
      return true;
    }
    auto position = adjacent_tetrahedra_[0].lock()->getOppositeNode(this->shared_from_this())->getPosition();
    return !(this->onSameSide(position, point));
  } else {
    return false;
  }
}

template<class T>
void Triangle3D<T>::orientToSide(const std::array<double, 3>& position) {
  if (!isInfinite()) {
    updatePlaneEquationIfNecessary();
    double dot = Matrix::dot(position, this->normal_vector_);
    if (dot > this->offset_ + this->tolerance_) {
      upper_side_positive_ = true;
    } else if (dot < this->offset_ - this->tolerance_) {
      upper_side_positive_ = false;
    } else {
      auto points = getExactPositionVectors();
      auto normal_vector = calculateExactNormalVector(points);
      auto dot_1 = normal_vector->dotProduct(points[0]);
      auto dot_2 = normal_vector->dotProduct(ExactVector::create(position));
      int comparison = dot_1->compareTo(dot_2);
      if (comparison == 0) {
        throw std::logic_error("The triangle cannot be oriented to because that point lies in the plane!");
      }
      upper_side_positive_ = comparison < 0;
    }
  }
}

template<class T>
void Triangle3D<T>::orientToOpenSide() {
  if (!isInfinite()) {
    if (adjacent_tetrahedra_[0].lock().get() == nullptr) {
      if (adjacent_tetrahedra_[1].lock().get() == nullptr) {
        throw std::logic_error("The triangle has two open sides!");
      }
      if (!adjacent_tetrahedra_[1].lock()->isInfinite()) {
        orientToSide(adjacent_tetrahedra_[1].lock()->getOppositeNode(this->shared_from_this())->getPosition());
        upper_side_positive_ ^= true;
      }
    } else if (adjacent_tetrahedra_[1].lock().get() == nullptr) {
      if (!adjacent_tetrahedra_[0].lock()->isInfinite()) {
        orientToSide(adjacent_tetrahedra_[0].lock()->getOppositeNode(this->shared_from_this())->getPosition());
        upper_side_positive_ ^= true;
      }
    } else {
      throw std::logic_error("The triangle has no open side!");
    }
  }
}

template<class T>
int Triangle3D<T>::orientationToUpperSide(const std::array<double, 3>& point) const {
  double dot = Matrix::dot(point, this->normal_vector_);
  if (dot > this->offset_ + this->tolerance_) {
    return upper_side_positive_ ? 1 : -1;
  } else if (dot < this->offset_ - this->tolerance_) {
    return upper_side_positive_ ? -1 : 1;
  } else {
    auto points = getExactPositionVectors();
    auto normal_vector = calculateExactNormalVector(points);
    auto dot_1 = normal_vector->dotProduct(points[0]);
    auto dot_2 = normal_vector->dotProduct(ExactVector::create(point));
    if (dot_1->compareTo(dot_2) == 0) {
      return 0;
    } else {
      return ((dot_1->compareTo(dot_2) > 0) ^ upper_side_positive_) ? 1 : -1;
    }
  }
}

template<class T>
bool Triangle3D<T>::onUpperSide(const std::array<double, 3>& point) const {
  return orientationToUpperSide(point) >= 0;
}

template<class T>
bool Triangle3D<T>::trulyOnUpperSide(const std::array<double, 3>& point) const {
  return orientationToUpperSide(point) > 0;
}

template<class T>
double Triangle3D<T>::getTypicalSDDistance() const {
  if (isInfinite()) {
    return std::numeric_limits<double>::max();
  } else {
    auto dummy = Matrix::subtract(nodes_[0]->getPosition(), circum_center_);
    return Matrix::norm(dummy) / Matrix::norm(this->normal_vector_);
  }
}

template<class T>
std::string Triangle3D<T>::toString() const {
  return "T3D";
//  return "{(" + StringUtil::toStr(nodes_[0]) + "," + StringUtil::toStr(nodes_[1]) + ","
//      + StringUtil::toStr(nodes_[2]) + "), " + "(" + StringUtil::toStr(adjacent_tetrahedra_[0])
//      + "," + StringUtil::toStr(adjacent_tetrahedra_[1]) + "), " + StringUtil::toStr(circum_center_)
//      + ", " + StringUtil::toStr(plane_updated_) + ", " + StringUtil::toStr(circum_center_updated_)
//      + ", " + StringUtil::toStr(upper_side_positive_) + ", "
//      + StringUtil::toStr(connection_checked_) + ", " + StringUtil::toStr(this->normal_vector_)
//      + ", " + StringUtil::toStr(this->offset_) + ", " + StringUtil::toStr(this->tolerance_) + ", "
//      + StringUtil::toStr(this->normal_vector_updated_) + "}";
}

template<class T>
bool Triangle3D<T>::isInfinite() const {
  return nodes_[0] == nullptr;
}

template<class T>
std::array<SpaceNode<T>*, 3> Triangle3D<T>::getNodes() const {
  return nodes_;
}

template<class T>
void Triangle3D<T>::addTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) {
  if (adjacent_tetrahedra_[0].lock().get() == nullptr) {
    adjacent_tetrahedra_[0] = tetrahedron;
  } else {
    adjacent_tetrahedra_[1] = tetrahedron;
  }
  connection_checked_ = -1;
}

template<class T>
bool Triangle3D<T>::wasCheckedAlready(int checking_index) {
  if (checking_index == connection_checked_) {
    return true;
  } else {
    connection_checked_ = checking_index;
    return false;
  }
}

template<class T>
bool Triangle3D<T>::isAdjacentTo(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) const {
  return (adjacent_tetrahedra_[0].lock() == tetrahedron) || (adjacent_tetrahedra_[1].lock() == tetrahedron);
}

template<class T>
bool Triangle3D<T>::isAdjacentTo(SpaceNode<T>* node) const {
  return (nodes_[0] == node) || (nodes_[1] == node) || (nodes_[2] == node);
}

template<class T>
bool Triangle3D<T>::isCompletelyOpen() const {
  return (adjacent_tetrahedra_[0].lock().get() == nullptr) && (adjacent_tetrahedra_[1].lock().get() == nullptr);
}

template<class T>
bool Triangle3D<T>::isClosed() const {
  return (adjacent_tetrahedra_[0].lock().get() != nullptr) && (adjacent_tetrahedra_[1].lock().get() != nullptr);
}

template<class T>
std::shared_ptr<ExactVector> Triangle3D<T>::getExactNormalVector() const {
  return calculateExactNormalVector(getExactPositionVectors());
}

template<class T>
void Triangle3D<T>::updateNormalVector(const std::array<double, 3>& new_normal_vector) {
  this->normal_vector_ = new_normal_vector;
  this->offset_ = Matrix::dot(this->normal_vector_, nodes_[0]->getPosition());
  this->normal_vector_updated_ = true;
}

template<class T>
std::shared_ptr<ExactVector> Triangle3D<T>::calculateCircumCenterExact(
    const std::array<std::shared_ptr<ExactVector>, 3>& points, const std::shared_ptr<ExactVector>& normal_vector) {
  auto a = points[0];
  // Start by calculating the normal vectors:
  std::array<std::shared_ptr<ExactVector>, 3> n = { points[1]->subtract(a), points[2]->subtract(a), normal_vector };
  std::array<std::shared_ptr<Rational>, 3> rationals = { points[1]->add(a)->dotProduct(n[0])->divideBy(
      Rational::create(2, 1)), points[2]->add(a)->dotProduct(n[1])->divideBy(Rational::create(2, 1)), a->dotProduct(
      n[2]) };
  return calculate3PlaneXPoint(n, rationals, ExactVector::det(n));
}

template<class T>
double Triangle3D<T>::calculateSDDistance(const std::array<double, 3>& fourth_point) const {
  if (!isInfinite()) {
    // calc that distance within 6 subtractions, 3 additions, 1 division
    // and 9 multiplications. Beat that!
    auto ad = Matrix::subtract(nodes_[0]->getPosition(), fourth_point);
    double denominator = Matrix::dot(ad, this->normal_vector_);
    if ((denominator != 0.0) && (std::abs(denominator) < this->tolerance_)) {
      auto n0_vector = ExactVector::create(nodes_[0]->getPosition());
      auto v1 = n0_vector->subtract(ExactVector::create(nodes_[1]->getPosition()));
      auto v2 = n0_vector->subtract(ExactVector::create(nodes_[2]->getPosition()));
      auto normal_vector = v1->crossProduct(v2);
      auto dot = normal_vector->dotProduct(n0_vector->subtract(ExactVector::create(fourth_point)));
      if (dot->isZero()) {
        denominator = 0.0;
      } else {
        denominator = dot->doubleValue();
        dot = normal_vector->dotProduct(ExactVector::create(this->normal_vector_));
        if (dot->compareTo(Rational::create(0, 1)) < 0) {
          denominator = 0 - denominator;
        }
      }
    }
    if (denominator != 0) {
      double sd_distance = Matrix::dot(
          ad,
          Matrix::subtract(Matrix::scalarMult(0.5, Matrix::add(nodes_[0]->getPosition(), fourth_point)),
                           circum_center_)) / denominator;
      return sd_distance;
    }
  }
  return std::numeric_limits<double>::max();
}

template<class T>
std::shared_ptr<Rational> Triangle3D<T>::calculateSDDistanceExact(
    const std::array<std::shared_ptr<ExactVector>, 4>& points,
    const std::shared_ptr<ExactVector>& normal_vector) const {
  if (!isInfinite()) {
    // calc that distance within 6 subtractions, 3 additions, 1 division
    // and 9 multiplications. Beat that!
    auto ad = points[0]->subtract(points[3]);
    auto denominator = ad->dotProduct(normal_vector);
    if (!denominator->isZero()) {
      std::array<std::shared_ptr<ExactVector>, 3> points_3;
      for (std::size_t i = 0; i < 3; i++) {
        points_3[i] = points[i];
      }
      auto circum_center = calculateCircumCenterExact(points_3, normal_vector);
      return points[0]->add(points[3])->divideBy(Rational::create(2, 1))->decreaseBy(circum_center)->dotProduct(ad)
          ->divideBy(denominator);
    }
  }
  return Rational::create(std::numeric_limits < int64_t > ::max());
}

template<class T>
void Triangle3D<T>::updateCircumCenterIfNecessary() {
  if (!circum_center_updated_ && !isInfinite()) {
    circum_center_updated_ = true;
    auto a = nodes_[0]->getPosition();
    // Start by calculating the normal vectors:
    std::array<std::array<double, 3>, 3> n;
    auto line_1 = Matrix::subtract(nodes_[1]->getPosition(), a);
    auto line_2 = Matrix::subtract(nodes_[2]->getPosition(), a);
    n[0] = Matrix::normalize(line_1);
    n[1] = Matrix::normalize(line_2);
    n[2] = Matrix::crossProduct(n[0], n[1]);
    updateNormalVector(n[2]);
    this->normal_vector_updated_ = true;
    this->tolerance_ = Matrix::dot(this->normal_vector_, this->normal_vector_) * Param::kDefaultTolerance;
    this->normal_vector_updated_ = true;
    // cut the three planes:
    circum_center_ = calculate3PlaneXPoint(
        n,
        { Matrix::dot(Matrix::add(a, nodes_[1]->getPosition()), n[0]) * 0.5, Matrix::dot(
            Matrix::add(a, nodes_[2]->getPosition()), n[1]) * 0.5, Matrix::dot(a, n[2]) });
  }
}

template<class T>
std::array<std::shared_ptr<ExactVector>, 3> Triangle3D<T>::getExactPositionVectors() const {
  std::array<std::shared_ptr<ExactVector>, 3> result;
  for (std::size_t i = 0; i < 3; i++) {
    result[i] = ExactVector::create(nodes_[i]->getPosition());
  }
  return result;
}

template<class T>
std::shared_ptr<ExactVector> Triangle3D<T>::calculateExactNormalVector(
    const std::array<std::shared_ptr<ExactVector>, 3>& points) const {
  return points[1]->subtract(points[0])->crossProduct(points[2]->subtract(points[0]));
}

// define templates that should be compiled
template class Triangle3D<bdm::physics::PhysicalNode> ;

}  // namespace spatial_organization
}  // namespace bdm
