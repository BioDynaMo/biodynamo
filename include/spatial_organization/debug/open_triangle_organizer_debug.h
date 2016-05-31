#ifndef SPATIAL_ORGANIZATION_DEBUG_OPEN_TRIANGLE_ORGANIZER_DEBUG_H_
#define SPATIAL_ORGANIZATION_DEBUG_OPEN_TRIANGLE_ORGANIZER_DEBUG_H_

#include "string_util.h"
#include "spatial_organization/open_triangle_organizer.h"

namespace cx3d {
namespace spatial_organization {

template<class T> class SimpleTriangulationNodeOrganizer;

/**
 * This class is used to generate debug output for the methods that are visible from
 * outside
 */
template<class T>
class OpenTriangleOrganizerDebug : public OpenTriangleOrganizer<T> {
 public:
  OpenTriangleOrganizerDebug(int preffered_size, const std::shared_ptr<SimpleTriangulationNodeOrganizer<T>>& tno)
      : OpenTriangleOrganizer<T>(preffered_size, tno) {
    logConstr("OpenTriangleOrganizer", preffered_size, tno);
  }

  virtual ~OpenTriangleOrganizerDebug() {
  }

  void recoredNewTetrahedra() {
    logCallParameterless();
    OpenTriangleOrganizer < T > ::recoredNewTetrahedra();
    logReturnVoid();
  }

  std::list<std::shared_ptr<Tetrahedron<T> > > getNewTetrahedra() {
    logCallParameterless();
    auto ret = OpenTriangleOrganizer < T > ::getNewTetrahedra();
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Tetrahedron<T>> getANewTetrahedron() {
    logCallParameterless();
    auto ret = OpenTriangleOrganizer < T > ::getANewTetrahedron();
    logReturn(ret);
    return ret;
  }

  void removeAllTetrahedraInSphere(const std::shared_ptr<Tetrahedron<T>>& starting_tetrahedron) {
    logCall(starting_tetrahedron);
    OpenTriangleOrganizer < T > ::removeAllTetrahedraInSphere(starting_tetrahedron);
    logReturnVoid();
  }

  void putTriangle(const std::shared_ptr<Triangle3D<T>>& triangle) {
    logCall(triangle);
    OpenTriangleOrganizer < T > ::putTriangle(triangle);
    logReturnVoid();
  }

  void removeTriangle(const std::shared_ptr<Triangle3D<T> >& triangle) {
    logCall(triangle);
    OpenTriangleOrganizer < T > ::removeTriangle(triangle);
    logReturnVoid();
  }

  std::shared_ptr<Triangle3D<T>> getTriangle(SpaceNode<T>* a, SpaceNode<T>* b, SpaceNode<T>* c) {
    logCall(a, b, c);
    auto ret = OpenTriangleOrganizer < T > ::getTriangle(a, b, c);
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Triangle3D<T>> getTriangleWithoutRemoving(SpaceNode<T>* a, SpaceNode<T>* b, SpaceNode<T>* c) {
    logCall(a, b, c);
    auto ret = OpenTriangleOrganizer < T > ::getTriangleWithoutRemoving(a, b, c);
    logReturn(ret);
    return ret;
  }

  void triangulate() {
    logCallParameterless();
    OpenTriangleOrganizer < T > ::triangulate();
    logReturnVoid();
  }

 private:
  OpenTriangleOrganizerDebug() = delete;
  OpenTriangleOrganizerDebug(const OpenTriangleOrganizerDebug&) = delete;
  OpenTriangleOrganizerDebug& operator=(const OpenTriangleOrganizerDebug&) = delete;
};

}  // spatial_organization
}  // cx3d

#endif // SPATIAL_ORGANIZATION_DEBUG_OPEN_TRIANGLE_ORGANIZER_DEBUG_H_

