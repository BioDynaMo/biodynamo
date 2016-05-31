#ifndef SPATIAL_ORGANIZATION_DEBUG_FLAT_TETRAHEDRON_DEBUG_H_
#define SPATIAL_ORGANIZATION_DEBUG_FLAT_TETRAHEDRON_DEBUG_H_

#include "string_util.h"
#include "spatial_organization/flat_tetrahedron.h"

namespace bdm {
namespace spatial_organization {

/**
 * This class is used to generate debug output for the methods that are visible from
 * outside
 */
template<class T>
class FlatTetrahedronDebug : public FlatTetrahedron<T> {
 public:
  FlatTetrahedronDebug()
      : FlatTetrahedron<T>() {
    logConstrParameterless("FlatTetrahedron");
  }

  virtual ~FlatTetrahedronDebug() {
  }

  void calculateCircumSphere() override {
    logCallParameterless();
    FlatTetrahedron < T > ::calculateCircumSphere();
    logReturnVoid();
  }

  void updateCirumSphereAfterNodeMovement(SpaceNode<T>* moved_node) override {
    logCall(moved_node);
    FlatTetrahedron < T > ::updateCirumSphereAfterNodeMovement(moved_node);
    logReturnVoid();
  }

  int orientation(const std::array<double, 3>& point) override {
    logCall(point);
    auto ret = FlatTetrahedron < T > ::orientation(point);
    logReturn(ret);
    return ret;
  }

  bool isTrulyInsideSphere(const std::array<double, 3>& point) override {
    logCall(point);
    auto ret = FlatTetrahedron < T > ::isTrulyInsideSphere(point);
    logReturn(ret);
    return ret;
  }

  bool isInsideSphere(const std::array<double, 3>& point) override {
    logCall(point);
    auto ret = FlatTetrahedron < T > ::isInsideSphere(point);
    logReturn(ret);
    return ret;
  }

  bool equalTo(const std::shared_ptr<Tetrahedron<T>>& other) override {
    logCall(other);
    auto ret = FlatTetrahedron < T > ::equalTo(other);
    logReturn(ret);
    return ret;
  }

  std::array<std::shared_ptr<Triangle3D<T>>, 4> getAdjacentTriangles() const override {
    logCallParameterless();
    auto ret = FlatTetrahedron < T > ::getAdjacentTriangles();
    logReturn(ret);
    return ret;
  }

  bool isAdjacentTo(SpaceNode<T>* node) const override {
    logCall(node);
    auto ret = FlatTetrahedron < T > ::isAdjacentTo(node);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Tetrahedron<T>> walkToPoint(const std::array<double, 3>& coordinate,
                                              const std::array<int, 4>& triangle_order) override {
    logCall(coordinate, triangle_order);
    auto ret = FlatTetrahedron < T > ::walkToPoint(coordinate, triangle_order);
    logReturn(ret);
    return ret;
  }

  std::array<SpaceNode<T>*, 4> getAdjacentNodes() const override {
    logCallParameterless();
    auto ret = FlatTetrahedron < T > ::getAdjacentNodes();
    logReturn(ret);
    return ret;
  }

  std::array<std::shared_ptr<T>, 4> getVerticeContents() const override {
    logCallParameterless();
    auto ret = FlatTetrahedron < T > ::getVerticeContents();
    logReturn(ret);
    return ret;
  }

  bool isInfinite() const override {
    logCallParameterless();
    auto ret = FlatTetrahedron < T > ::isInfinite();
    logReturn(ret);
    return ret;
  }

  bool isFlat() const {
    logCallParameterless();
    auto ret = FlatTetrahedron < T > ::isFlat();
    logReturn(ret);
    return ret;
  }

  void changeCrossSection(int number, double new_value) override {
    logCall(number, new_value);
    FlatTetrahedron < T > ::changeCrossSection(number, new_value);
    logReturnVoid();
  }

  void updateCrossSectionAreas() override {
    logCallParameterless();
    FlatTetrahedron < T > ::updateCrossSectionAreas();
    logReturnVoid();
  }

  void calculateVolume() override {
    logCallParameterless();
    FlatTetrahedron < T > ::calculateVolume();
    logReturnVoid();
  }

  int orientationExact(const std::array<double, 3>& position) const override {
    logCall(position);
    auto ret = FlatTetrahedron < T > ::orientationExact(position);
    logReturn(ret);
    return ret;
  }

  void replaceTriangle(const std::shared_ptr<Triangle3D<T>>& old_triangle,
                       const std::shared_ptr<Triangle3D<T>>& new_triangle) override {
    logCall(old_triangle, new_triangle);
    FlatTetrahedron < T > ::replaceTriangle(old_triangle, new_triangle);
    logReturnVoid();
  }

  int getNodeNumber(SpaceNode<T>* node) const override {
    logCall(node);
    auto ret = FlatTetrahedron < T > ::getNodeNumber(node);
    logReturn(ret);
    return ret;
  }

  int getTriangleNumber(const std::shared_ptr<Triangle3D<T>>& triangle) const override {
    logCall(triangle);
    auto ret = FlatTetrahedron < T > ::getTriangleNumber(triangle);
    logReturn(ret);
    return ret;
  }

  Edge<T>* getEdge(int node_number_1, int node_number_2) const override {
    logCall(node_number_1, node_number_2);
    auto ret = FlatTetrahedron < T > ::getEdge(node_number_1, node_number_2);
    logReturn(ret);
    return ret;
  }

  Edge<T>* getEdge(SpaceNode<T>* a, SpaceNode<T>* b) const override {
    logCall(a, b);
    auto ret = FlatTetrahedron < T > ::getEdge(a, b);
    logReturn(ret);
    return ret;
  }

  int getEdgeNumber(SpaceNode<T>* a, SpaceNode<T>* b) const override {
    logCall(a, b);
    auto ret = FlatTetrahedron < T > ::getEdgeNumber(a, b);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Triangle3D<T>> getOppositeTriangle(SpaceNode<T>* node) const override {
    logCall(node);
    auto ret = FlatTetrahedron < T > ::getOppositeTriangle(node);
    logReturn(ret);
    return ret;
  }

  SpaceNode<T>* getOppositeNode(const std::shared_ptr<Triangle3D<T>>& triangle) const override {
    logCall(triangle);
    auto ret = FlatTetrahedron < T > ::getOppositeNode(triangle);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Triangle3D<T>> getConnectingTriangle(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) const
      override {
    logCall(tetrahedron);
    auto ret = FlatTetrahedron < T > ::getConnectingTriangle(tetrahedron);
    logReturn(ret);
    return ret;
  }

  int getConnectingTriangleNumber(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) const override {
    logCall(tetrahedron);
    auto ret = FlatTetrahedron < T > ::getConnectingTriangleNumber(tetrahedron);
    logReturn(ret);
    return ret;
  }

  std::array<std::shared_ptr<Triangle3D<T>>, 3> getTouchingTriangles(const std::shared_ptr<Triangle3D<T>>& base) const
      override {
    logCall(base);
    auto ret = FlatTetrahedron < T > ::getTouchingTriangles(base);
    logReturn(ret);
    return ret;
  }

  void remove() override {
    logCallParameterless();
    FlatTetrahedron < T > ::remove();
    logReturnVoid();
  }

  bool isPointInConvexPosition(const std::array<double, 3>& point, int connecting_triangle_number) const override {
    logCall(point, connecting_triangle_number);
    auto ret = FlatTetrahedron < T > ::isPointInConvexPosition(point, connecting_triangle_number);
    logReturn(ret);
    return ret;
  }

  int isInConvexPosition(const std::array<double, 3>& point, int connecting_triangle_number) const override {
    logCall(point, connecting_triangle_number);
    auto ret = FlatTetrahedron < T > ::isInConvexPosition(point, connecting_triangle_number);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Tetrahedron<T>> getAdjacentTetrahedron(int number) override {
    logCall(number);
    auto ret = FlatTetrahedron < T > ::getAdjacentTetrahedron(number);
    logReturn(ret);
    return ret;
  }

  void testPosition(const std::array<double, 3>& position) const throw (std::exception) override {
    logCall(position);
    FlatTetrahedron < T > ::testPosition(position);
    logReturnVoid();
  }

  bool isValid() const override {
    logCallParameterless();
    auto ret = FlatTetrahedron < T > ::isValid();
    logReturn(ret);
    return ret;
  }

  bool isNeighbor(const std::shared_ptr<Tetrahedron<T>>& other_tetrahedron) const override {
    logCall(other_tetrahedron);
    auto ret = FlatTetrahedron < T > ::isNeighbor(other_tetrahedron);
    logReturn(ret);
    return ret;
  }

  SpaceNode<T>* getFirstOtherNode(SpaceNode<T>* node_a, SpaceNode<T>* node_b) const override {
    logCall(node_a, node_b);
    auto ret = FlatTetrahedron < T > ::getFirstOtherNode(node_a, node_b);
    logReturn(ret);
    return ret;
  }

  SpaceNode<T>* getSecondOtherNode(SpaceNode<T>* node_a, SpaceNode<T>* node_b) const override {
    logCall(node_a, node_b);
    auto ret = FlatTetrahedron < T > ::getSecondOtherNode(node_a, node_b);
    logReturn(ret);
    return ret;
  }

 private:
  FlatTetrahedronDebug(const FlatTetrahedronDebug&) = delete;
  FlatTetrahedronDebug& operator=(const FlatTetrahedronDebug&) = delete;
};

}  // namespace spatial_organization
}  // namespace bdm

#endif  // SPATIAL_ORGANIZATION_DEBUG_FLAT_TETRAHEDRON_DEBUG_H_
