#ifndef SPATIAL_ORGANIZATION_DEBUG_SPACE_NODE_DEBUG_H_
#define SPATIAL_ORGANIZATION_DEBUG_SPACE_NODE_DEBUG_H_

#include "string_util.h"
#include "spatial_organization/space_node.h"

namespace cx3d {
namespace spatial_organization {

/**
 * This class is used to generate debug output for the methods that are visible from
 * outside
 */
template<class T>
class SpaceNodeDebug : public SpaceNode<T> {
 public:
  std::shared_ptr<Tetrahedron<T> > searchInitialInsertionTetrahedron(
      const std::shared_ptr<Tetrahedron<T> >& start,
      const std::array<double, 3>& coordinate) {
    logCall(start, coordinate);
    auto ret = SpaceNode<T>::searchInitialInsertionTetrahedron(start,
                                                               coordinate);
    logReturn(ret);
    return ret;
  }

  SpaceNodeDebug(const std::array<double, 3>& position,
                 const std::shared_ptr<T> content)
      : SpaceNode<T>(position, content) {
    logConstr("SpaceNode", position, content);
  }

  SpaceNodeDebug(double x, double y, double z, const std::shared_ptr<T> content)
      : SpaceNode<T>(x, y, z, content) {
    logConstr("SpaceNode", x, y, z, content);
  }

  void addSpatialOrganizationNodeMovementListener(
      const std::shared_ptr<SpatialOrganizationNodeMovementListener<T> >& listener) {
    logCall(listener);
    SpaceNode<T>::addSpatialOrganizationNodeMovementListener(listener);
    logReturnVoid();
  }

  std::list<Edge<T>*> getEdges() const {
    logCallParameterless();
    auto ret = SpaceNode<T>::getEdges();
    logReturn(ret);
    return ret;
  }

  std::list<std::shared_ptr<T> > getNeighbors() const {
    logCallParameterless();
    auto ret = SpaceNode<T>::getNeighbors();
    logCall(ret);
    return ret;
  }

  SpaceNode<T>* getNewInstance(
      const std::array<double, 3>& position,
      const std::shared_ptr<T>& user_object) {
    logCall(position, user_object);
    auto ret = SpaceNode<T>::getNewInstance(position, user_object);
    logReturn(ret);
    return ret;
  }

  std::list<std::shared_ptr<T> > getPermanentListOfNeighbors() const {
    logCallParameterless();
    auto ret = SpaceNode<T>::getPermanentListOfNeighbors();
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> getPosition() const {
    logCallParameterless();
    auto ret = SpaceNode<T>::getPosition();
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<T> getUserObject() const {
    logCallParameterless();
    auto ret = SpaceNode<T>::getUserObject();
    logReturn(ret);
    return ret;
  }

  std::array<std::shared_ptr<T>, 4> getVerticesOfTheTetrahedronContaining(
      const std::array<double, 3>& position, std::array<int, 1>& returned_null) const {
    logCall(position, returned_null);
    auto ret = SpaceNode<T>::getVerticesOfTheTetrahedronContaining(position, returned_null);
//    logReturn(ret);
    return ret;
  }

  double getVolume() const {
    logCallParameterless();
    auto ret = SpaceNode<T>::getVolume();
    logReturn(ret);
    return ret;
  }

  void moveFrom(const std::array<double, 3>& delta) {
    logCall(delta);
    SpaceNode<T>::moveFrom(delta);
    logReturnVoid();
  }

  void remove() {
    logCallParameterless();
    SpaceNode<T>::remove();
    logReturnVoid();
  }

  std::list<std::shared_ptr<Tetrahedron<T> > > getAdjacentTetrahedra() const {
    logCallParameterless();
    auto ret = SpaceNode<T>::getAdjacentTetrahedra();
    logReturn(ret);
    return ret;
  }

  void addAdjacentTetrahedron(
      const std::shared_ptr<Tetrahedron<T> >& tetrahedron) {
    logCall(tetrahedron);
    SpaceNode<T>::addAdjacentTetrahedron(tetrahedron);
    logReturnVoid();
  }

  void removeTetrahedron(
      const std::shared_ptr<Tetrahedron<T> >& tetrahedron) {
    logCall(tetrahedron);
    SpaceNode<T>::removeTetrahedron(tetrahedron);
    logReturnVoid();
  }

  void moveTo(const std::array<double, 3>& new_position) {
    logCall(new_position);
    SpaceNode<T>::moveTo(new_position);
    logReturnVoid();
  }

  void changeVolume(double change) {
    logCall(change);
    SpaceNode<T>::changeVolume(change);
    logReturnVoid();
  }

  void addEdge(Edge<T>* edge) {
    logCall(edge);
    SpaceNode<T>::addEdge(edge);
    logReturnVoid();
  }

  int getId() const {
    logCallParameterless();
    auto ret = SpaceNode<T>::getId();
    logReturn(ret);
    return ret;
  }

  Edge<T>* searchEdge(
      SpaceNode<T>* opposite_node) {
    logCall(opposite_node);
    auto ret = SpaceNode<T>::searchEdge(opposite_node);
    logReturn(ret);
    return ret;
  }

  void removeEdge(Edge<T>* edge) {
    logCall(edge);
    SpaceNode<T>::removeEdge(edge);
    logReturnVoid();
  }

  void setListenerList(
      const std::list<std::shared_ptr<SpatialOrganizationNodeMovementListener<T> > >& listeners) {
    logCall(listeners);
    SpaceNode<T>::setListenerList(listeners);
    logReturnVoid();
  }

  std::shared_ptr<Tetrahedron<T> > searchInitialInsertionTetrahedron(
      const std::shared_ptr<Tetrahedron<T> >& start) {
    logCall(start);
    auto ret = SpaceNode<T>::searchInitialInsertionTetrahedron(start);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Tetrahedron<T> > insert(
      const std::shared_ptr<Tetrahedron<T> >& start) {
    logCall(start);
    auto ret = SpaceNode<T>::insert(start);
    logReturn(ret);
    return ret;
  }

  void restoreDelaunay() {
    logCallParameterless();
    SpaceNode<T>::restoreDelaunay();
    logReturnVoid();
  }

  std::array<double, 3> proposeNewPosition() {
    logCallParameterless();
    auto ret = SpaceNode<T>::proposeNewPosition();
    logReturn(ret);
    return ret;
  }

  std::list<Edge<T>*> getAdjacentEdges() const {
    logCallParameterless();
    auto ret = SpaceNode<T>::getAdjacentEdges();
    logReturn(ret);
    return ret;
  }

//  bool equalTo(SpaceNode<T>* other) {
//    logCall(other);
//    auto ret = SpaceNode<T>::equalTo(other);
//    logReturn(ret);
//    return ret;
//  }

  int createNewCheckingIndex() {
    logCallParameterless();
    auto ret = SpaceNode<T>::createNewCheckingIndex();
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Tetrahedron<T> > removeAndReturnCreatedTetrahedron() {
    logCallParameterless();
    auto ret = SpaceNode<T>::removeAndReturnCreatedTetrahedron();
    logReturn(ret);
    return ret;
  }

  void processTetrahedron(
      std::shared_ptr<Tetrahedron<T> >& tetrahedron,
      std::list<std::shared_ptr<Triangle3D<T> > >& queue,
      std::shared_ptr<OpenTriangleOrganizer<T> >& oto) {
    logCall(tetrahedron, queue, oto);
    SpaceNode<T>::processTetrahedron(tetrahedron, queue, oto);
    logReturnVoid();
  }

  bool checkIfTriangulationIsStillValid(
      const std::array<double, 3>& new_position) {
    logCall(new_position);
    auto ret = SpaceNode<T>::checkIfTriangulationIsStillValid(new_position);
    logReturn(ret);
    return ret;
  }

  bool removeTetrahedronDuringCleanUp(
      std::shared_ptr<Tetrahedron<T> >& tetrahedron_to_remove,
      std::list<std::shared_ptr<Tetrahedron<T> > >& list,
      std::list<SpaceNode<T>* >& node_list,
      std::shared_ptr<OpenTriangleOrganizer<T> >& oto) {
    logCall(tetrahedron_to_remove, list, node_list, oto);
    auto ret = SpaceNode<T>::removeTetrahedronDuringCleanUp(tetrahedron_to_remove, list, node_list, oto);
    logReturn(ret);
    return ret;
  }

  void cleanUp(
      const std::list<std::shared_ptr<Tetrahedron<T> > >& messed_up_tetrahedra) {
    logCall(messed_up_tetrahedra);
    SpaceNode<T>::cleanUp(messed_up_tetrahedra);
    logReturnVoid();
  }

 private:
  SpaceNodeDebug() = delete;
  SpaceNodeDebug(const SpaceNodeDebug&) = delete;
  SpaceNodeDebug& operator=(const SpaceNodeDebug&) = delete;
};

}  // spatial_organization
}  // cx3d

#endif // SPATIAL_ORGANIZATION_DEBUG_SPACE_NODE_DEBUG_H_
