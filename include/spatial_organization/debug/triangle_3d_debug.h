#ifndef SPATIAL_ORGANIZATION_DEBUG_TRIANGLE_3D_DEBUG_H_
#define SPATIAL_ORGANIZATION_DEBUG_TRIANGLE_3D_DEBUG_H_

#include "string_util.h"
#include "spatial_organization/triangle_3d.h"

namespace cx3d {
namespace spatial_organization {

template<class T> class Triangle3D;

/**
 * This class is used to generate debug output for the methods that are visible from
 * outside
 */
template<class T>
class Triangle3DDebug : public Triangle3D<T> {
 public:
  Triangle3DDebug(SpaceNode<T>* sn_1, SpaceNode<T>* sn_2, SpaceNode<T>* sn_3,
                  const std::shared_ptr<Tetrahedron<T>>& tetrahedron_1,
                  const std::shared_ptr<Tetrahedron<T>>& tetrahedron_2)
      : Triangle3D<T>(sn_1, sn_2, sn_3, tetrahedron_1, tetrahedron_2) {
    logConstr("Triangle3D", sn_1, sn_2, sn_3, tetrahedron_1, tetrahedron_2);
  }

  virtual ~Triangle3DDebug() {
  }

  bool isSimilarTo(const std::shared_ptr<Triangle3D<T>>& other_triangle) const override {
    logCall(other_triangle);
    auto ret = Triangle3D<T>::isSimilarTo(other_triangle);
    logReturn(ret);
    return ret;
  }

  double getSDDistance(const std::array<double, 3>& fourth_point) const override {
    logCall(fourth_point);
    auto ret = Triangle3D<T>::getSDDistance(fourth_point);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Rational> getSDDistanceExact(const std::array<double, 3>& fourth_point) const override {
    logCall(fourth_point);
    auto ret = Triangle3D<T>::getSDDistanceExact(fourth_point);
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> calculateCircumSphereCenter(const std::array<double, 3>& fourth_point) const override {
    logCall(fourth_point);
    auto ret = Triangle3D<T>::calculateCircumSphereCenter(fourth_point);
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> calculateCircumSphereCenterIfEasy(const std::array<double, 3>& fourth_point) const override {
    logCall(fourth_point);
    auto ret = Triangle3D<T>::calculateCircumSphereCenterIfEasy(fourth_point);
    logReturn(ret);
    return ret;
  }

  void informAboutNodeMovement() override {
    logCallParameterless();
    Triangle3D<T>::informAboutNodeMovement();
    logReturnVoid();
  }

  void updatePlaneEquationIfNecessary() override {
    logCallParameterless();
    Triangle3D<T>::updatePlaneEquationIfNecessary();
    logReturnVoid();
  }

  void update() override {
    logCallParameterless();
    Triangle3D<T>::update();
    logReturnVoid();
  }

  int orientationExact(const std::array<double, 3>& point_1, const std::array<double, 3>& point_2) const override {
    logCall(point_1, point_2);
    auto ret = Triangle3D<T>::orientationExact(point_1, point_2);
    logReturn(ret);
    return ret;
  }

  int circleOrientation(const std::array<double, 3>& point) override {
    logCall(point);
    auto ret = Triangle3D<T>::circleOrientation(point);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Tetrahedron<T>> getOppositeTetrahedron(
      const std::shared_ptr<Tetrahedron<T>>& incident_tetrahedron) const override {
    logCall(incident_tetrahedron);
    auto ret = Triangle3D<T>::getOppositeTetrahedron(incident_tetrahedron);
    logReturn(ret);
    return ret;
  }

  void removeTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) override {
    logCall(tetrahedron);
    Triangle3D<T>::removeTetrahedron(tetrahedron);
    logReturnVoid();
  }

  bool isOpenToSide(const std::array<double, 3>& point) override {
    logCall(point);
    auto ret = Triangle3D<T>::isOpenToSide(point);
    logReturn(ret);
    return ret;
  }

  void orientToSide(const std::array<double, 3>& position) override {
    logCall(position);
    Triangle3D<T>::orientToSide(position);
    logReturnVoid();
  }

  void orientToOpenSide() override {
    logCallParameterless();
    Triangle3D<T>::orientToOpenSide();
    logReturnVoid();
  }

  int orientationToUpperSide(const std::array<double, 3>& point) const override {
    logCall(point);
    auto ret = Triangle3D<T>::orientationToUpperSide(point);
    logReturn(ret);
    return ret;
  }

  bool onUpperSide(const std::array<double, 3>& point) const override {
    logCall(point);
    auto ret = Triangle3D<T>::onUpperSide(point);
    logReturn(ret);
    return ret;
  }

  bool trulyOnUpperSide(const std::array<double, 3>& point) const override {
    logCall(point);
    auto ret = Triangle3D<T>::trulyOnUpperSide(point);
    logReturn(ret);
    return ret;
  }

  double getTypicalSDDistance() const override {
    logCallParameterless();
    auto ret = Triangle3D<T>::getTypicalSDDistance();
    logReturn(ret);
    return ret;
  }

  bool equalTo(const std::shared_ptr<Triangle3D<T>>& other) {
    logCall(other);
    auto ret = Triangle3D<T>::equalTo(other);
    logReturn(ret);
    return ret;
  }

  bool isInfinite() const override {
    logCallParameterless();
    auto ret = Triangle3D<T>::isInfinite();
    logReturn(ret);
    return ret;
  }

  std::array<SpaceNode<T>*, 3> getNodes() const override {
    logCallParameterless();
    auto ret = Triangle3D<T>::getNodes();
    logReturn(ret);
    return ret;
  }

  void addTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) override {
    logCall(tetrahedron);
    Triangle3D<T>::addTetrahedron(tetrahedron);
    logReturnVoid();
  }

  bool wasCheckedAlready(int checking_index) override {
    logCall(checking_index);
    auto ret = Triangle3D<T>::wasCheckedAlready(checking_index);
    logReturn(ret);
    return ret;
  }

  bool isAdjacentTo(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) const override {
    logCall(tetrahedron);
    auto ret = Triangle3D<T>::isAdjacentTo(tetrahedron);
    logReturn(ret);
    return ret;
  }

  bool isAdjacentTo(SpaceNode<T>* node) const override {
    logCall(node);
    auto ret = Triangle3D<T>::isAdjacentTo(node);
    logReturn(ret);
    return ret;
  }

  bool isCompletelyOpen() const override {
    logCallParameterless();
    auto ret = Triangle3D<T>::isCompletelyOpen();
    logReturn(ret);
    return ret;
  }

  bool isClosed() const override {
    logCallParameterless();
    auto ret = Triangle3D<T>::isClosed();
    logReturn(ret);
    return ret;
  }

 protected:
  std::shared_ptr<ExactVector> getExactNormalVector() const override {
    logCallParameterless();
    auto ret = Triangle3D<T>::getExactNormalVector();
    logReturn(ret);
    return ret;
  }

  void updateNormalVector(const std::array<double, 3>& new_normal_vector) override {
    logCall(new_normal_vector);
    Triangle3D<T>::updateNormalVector(new_normal_vector);
    logReturnVoid();
  }

 private:
  Triangle3DDebug() = delete;
  Triangle3DDebug(const Triangle3DDebug&) = delete;
  Triangle3DDebug& operator=(const Triangle3DDebug&) = delete;
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_DEBUG_TRIANGLE_3D_DEBUG_H_
