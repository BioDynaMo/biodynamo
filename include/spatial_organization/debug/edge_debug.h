#ifndef SPATIAL_ORGANIZATION_DEBUG_EDGE_DEBUG_H_
#define SPATIAL_ORGANIZATION_DEBUG_EDGE_DEBUG_H_

#include "string_util.h"
#include "spatial_organization/edge.h"

namespace bdm {
namespace spatial_organization {

/**
 * This class is used to generate debug output for the methods that are visible from
 * outside
 */
template<class T>
class EdgeDebug : public Edge<T> {
 public:
  EdgeDebug(SpaceNode<T>* a, SpaceNode<T>* b)
      : Edge<T>(a, b) {
    logConstr("Edge", a, b);
  }

  virtual ~EdgeDebug() {
  }

  SpaceNode<T>* getOpposite(SpaceNode<T>* node) const override {
    logCall(node);
    auto ret = Edge < T > ::getOpposite(node);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<T> getOppositeElement(const std::shared_ptr<T>& first) const override {
    logCall(first);
    auto ret = Edge < T > ::getOppositeElement(first);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<T> getFirstElement() const override {
    logCallParameterless();
    auto ret = Edge < T > ::getFirstElement();
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<T> getSecondElement() const override {
    logCallParameterless();
    auto ret = Edge < T > ::getSecondElement();
    logReturn(ret);
    return ret;
  }

  double getCrossSection() const override {
    logCallParameterless();
    auto ret = Edge < T > ::getCrossSection();
    logReturn(ret);
    return ret;
  }

  bool equalTo(Edge<T>* other) override {
    logCall(other);
    auto ret = Edge < T > ::equalTo(other);
    logReturn(ret);
    return ret;
  }

  bool equals(SpaceNode<T>* a, SpaceNode<T>* b) const override {
    logCall(a, b);
    auto ret = Edge < T > ::equals(a, b);
    logReturn(ret);
    return ret;
  }

  void removeTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) override {
    logCall(tetrahedron);
    Edge < T > ::removeTetrahedron(tetrahedron);
    logReturnVoid();
  }

  void addTetrahedron(const std::shared_ptr<Tetrahedron<T>>& tetrahedron) override {
    logCall(tetrahedron);
    Edge < T > ::addTetrahedron(tetrahedron);
    logReturnVoid();
  }

  void remove() override {
    logCallParameterless();
    Edge < T > ::remove();
    logReturnVoid();
  }

  void changeCrossSectionArea(double change) override {
    logCall(change);
    Edge < T > ::changeCrossSectionArea(change);
    logReturnVoid();
  }

 private:
  EdgeDebug() = delete;
  EdgeDebug(const EdgeDebug&) = delete;
  EdgeDebug& operator=(const EdgeDebug&) = delete;
};

}  // namespace spatial_organization
}  // namespace bdm

#endif  // SPATIAL_ORGANIZATION_DEBUG_EDGE_DEBUG_H_
