#ifndef SPATIAL_ORGANIZATION_DEBUG_TETRAHEDRON_DEBUG_H_
#define SPATIAL_ORGANIZATION_DEBUG_TETRAHEDRON_DEBUG_H_

#include "string_util.h"
#include "spatial_organization/tetrahedron.h"

namespace cx3d {
namespace spatial_organization {

/**
 * This class is used to generate debug output for the methods that are visible from
 * outside
 */
template<class T>
class TetrahedronDebug : public Tetrahedron<T> {
 public:
  TetrahedronDebug()
      : Tetrahedron<T>() {
  }

  virtual ~TetrahedronDebug() {
  }

  void calculateCircumSphere() override {
    logCallParameterless();
    Tetrahedron < T > ::calculateCircumSphere();
    logReturnVoid();
  }

  void updateCirumSphereAfterNodeMovement(SpaceNode<T>* moved_node) override {
    logCall(moved_node);
    Tetrahedron < T > ::updateCirumSphereAfterNodeMovement(moved_node);
    logReturnVoid();
  }

  int orientation(const std::array<double, 3>& point) override {
    logCall(point);
    auto ret = Tetrahedron < T > ::orientation(point);
    logReturn(ret);
    return ret;
  }

  bool isTrulyInsideSphere(const std::array<double, 3>& point) override {
    logCall(point);
    auto ret = Tetrahedron < T > ::isTrulyInsideSphere(point);
    logReturn(ret);
    return ret;
  }

  bool isInsideSphere(const std::array<double, 3>& point) override {
    logCall(point);
    auto ret = Tetrahedron < T > ::isInsideSphere(point);
    logReturn(ret);
    return ret;
  }

//  bool equalTo(const std::shared_ptr<Tetrahedron<T>>& other) override {
//    logCall(other);
//    auto ret = Tetrahedron<T>::equalTo(other);
//    logReturn(ret);
//    return ret;
//  }

  std::array<std::shared_ptr<Triangle3D<T>>, 4> getAdjacentTriangles() const override {
    logCallParameterless();
    auto ret = Tetrahedron < T > ::getAdjacentTriangles();
    logReturn(ret);
    return ret;
  }

  bool isAdjacentTo(SpaceNode<T>* node) const override {
    logCall(node);
    auto ret = Tetrahedron < T > ::isAdjacentTo(node);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Tetrahedron<T>> walkToPoint(const std::array<double, 3>& coordinate,
                                              const std::array<int, 4>& triangle_order) override {
    logCall(coordinate, triangle_order);
    auto ret = Tetrahedron < T > ::walkToPoint(coordinate, triangle_order);
    logReturn(ret);
    return ret;
  }

  std::array<SpaceNode<T>*, 4> getAdjacentNodes() const override {
    logCallParameterless();
    auto ret = Tetrahedron < T > ::getAdjacentNodes();
    logReturn(ret);
    return ret;
  }

  std::array<std::shared_ptr<T>, 4> getVerticeContents() const override {
    logCallParameterless();
    auto ret = Tetrahedron < T > ::getVerticeContents();
    logReturn(ret);
    return ret;
  }

  bool isInfinite() const override {
    logCallParameterless();
    auto ret = Tetrahedron < T > ::isInfinite();
    logReturn(ret);
    return ret;
  }

  bool isFlat() const {
    logCallParameterless();
    auto ret = Tetrahedron < T > ::isFlat();
    logReturn(ret);
    return ret;
  }

  void changeCrossSection(int number, double new_value) override {
    logCall(number, new_value);
    Tetrahedron < T > ::changeCrossSection(number, new_value);
    logReturnVoid();
  }

  void updateCrossSectionAreas() override {
    logCallParameterless();
    Tetrahedron < T > ::updateCrossSectionAreas();
    logReturnVoid();
  }

  void calculateVolume() override {
    logCallParameterless();
    Tetrahedron < T > ::calculateVolume();
    logReturnVoid();
  }

  int orientationExact(const std::array<double, 3>& position) const override {
    logCall(position);
    auto ret = Tetrahedron < T > ::orientationExact(position);
    logReturn(ret);
    return ret;
  }

  void replaceTriangle(const std::shared_ptr<Triangle3D<T>>& old_triangle,
                       const std::shared_ptr<Triangle3D<T>>& new_triangle) override {
    logCall(old_triangle, new_triangle);
    Tetrahedron < T > ::replaceTriangle(old_triangle, new_triangle);
    logReturnVoid();
  }

  int getNodeNumber(SpaceNode<T>* node) const override {
    logCall(node);
    auto ret = Tetrahedron < T > ::getNodeNumber(node);
    logReturn(ret);
    return ret;
  }

  int getTriangleNumber(const std::shared_ptr<Triangle3D<T>>& triangle) const override {
    logCall(triangle);
    auto ret = Tetrahedron < T > ::getTriangleNumber(triangle);
    logReturn(ret);
    return ret;
  }

  Edge<T>* getEdge(int node_number_1, int node_number_2) const override {
    logCall(node_number_1, node_number_2);
    auto ret = Tetrahedron < T > ::getEdge(node_number_1, node_number_2);
    logReturn(ret);
    return ret;
  }

  Edge<T>* getEdge(SpaceNode<T>* a, SpaceNode<T>* b) const override {
    logCall(a, b);
    auto ret = Tetrahedron < T > ::getEdge(a, b);
    logReturn(ret);
    return ret;
  }

  int getEdgeNumber(SpaceNode<T>* a, SpaceNode<T>* b) const override {
    logCall(a, b);
    auto ret = Tetrahedron < T > ::getEdgeNumber(a, b);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Triangle3D<T>> getOppositeTriangle(SpaceNode<T>* node) const override {
    logCall(node);
    auto ret = Tetrahedron < T > ::getOppositeTriangle(node);
    logReturn(ret);
    return ret;
  }

  SpaceNode<T>* getOppositeNode(const std::shared_ptr<Triangle3D<T>>& triangle) const override {
    logCall(triangle);
    auto ret = Tetrahedron < T > ::getOppositeNode(triangle);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Triangle3D<T>> getConnectingTriangle(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) const
      override {
    logCall(tetrahedron);
    auto ret = Tetrahedron < T > ::getConnectingTriangle(tetrahedron);
    logReturn(ret);
    return ret;
  }

  int getConnectingTriangleNumber(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) const override {
    logCall(tetrahedron);
    auto ret = Tetrahedron < T > ::getConnectingTriangleNumber(tetrahedron);
    logReturn(ret);
    return ret;
  }

  std::array<std::shared_ptr<Triangle3D<T>>, 3> getTouchingTriangles(const std::shared_ptr<Triangle3D<T>>& base) const
      override {
    logCall(base);
    auto ret = Tetrahedron < T > ::getTouchingTriangles(base);
    logReturn(ret);
    return ret;
  }

  void remove() override {
    logCallParameterless();
    Tetrahedron < T > ::remove();
    logReturnVoid();
  }

  bool isPointInConvexPosition(const std::array<double, 3>& point, int connecting_triangle_number) const override {
    logCall(point, connecting_triangle_number);
    auto ret = Tetrahedron < T > ::isPointInConvexPosition(point, connecting_triangle_number);
    logReturn(ret);
    return ret;
  }

  int isInConvexPosition(const std::array<double, 3>& point, int connecting_triangle_number) const override {
    logCall(point, connecting_triangle_number);
    auto ret = Tetrahedron < T > ::isInConvexPosition(point, connecting_triangle_number);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Tetrahedron<T>> getAdjacentTetrahedron(int number) override {
    logCall(number);
    auto ret = Tetrahedron < T > ::getAdjacentTetrahedron(number);
    logReturn(ret);
    return ret;
  }

  void testPosition(const std::array<double, 3>& position) const throw (std::exception) override {
    logCall(position);
    Tetrahedron < T > ::testPosition(position);
    logReturnVoid();
  }

  bool isValid() const override {
    logCallParameterless();
    auto ret = Tetrahedron < T > ::isValid();
    logReturn(ret);
    return ret;
  }

  bool isNeighbor(const std::shared_ptr<Tetrahedron<T>>& other_tetrahedron) const override {
    logCall(other_tetrahedron);
    auto ret = Tetrahedron < T > ::isNeighbor(other_tetrahedron);
    logReturn(ret);
    return ret;
  }

  SpaceNode<T>* getFirstOtherNode(SpaceNode<T>* node_a, SpaceNode<T>* node_b) const override {
    logCall(node_a, node_b);
    auto ret = Tetrahedron < T > ::getFirstOtherNode(node_a, node_b);
    logReturn(ret);
    return ret;
  }

  SpaceNode<T>* getSecondOtherNode(SpaceNode<T>* node_a, SpaceNode<T>* node_b) const override {
    logCall(node_a, node_b);
    auto ret = Tetrahedron < T > ::getSecondOtherNode(node_a, node_b);
    logReturn(ret);
    return ret;
  }

 private:
  TetrahedronDebug(const TetrahedronDebug&) = delete;
  TetrahedronDebug& operator=(const TetrahedronDebug&) = delete;
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_DEBUG_TETRAHEDRON_DEBUG_H_
