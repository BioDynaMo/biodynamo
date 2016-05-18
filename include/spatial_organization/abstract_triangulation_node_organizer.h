#ifndef SPATIAL_ORGANIZATION_ABSTRACT_TRIANGULATION_NODE_ORGANIZER_H_
#define SPATIAL_ORGANIZATION_ABSTRACT_TRIANGULATION_NODE_ORGANIZER_H_

#include <list>
#include <memory>
#include <string>

namespace cx3d {
namespace spatial_organization {

template<class T> class SpaceNode;
template<class T> class Triangle3D;

/**
 * Instances of child classes of this class are used to keep track of
 * nodes in an incomplete triangulation which might possibly become neighbors of open triangles.
 *
 * @param <T> The type of user objects associated with nodes in the current triangulation.
 */
template<class T>
class AbstractTriangulationNodeOrganizer {
 public:
  AbstractTriangulationNodeOrganizer() {
  }

  virtual ~AbstractTriangulationNodeOrganizer() {
  }

  virtual std::list<SpaceNode<T>* > getNodes(SpaceNode<T>* reference_point) = 0;

  virtual void addTriangleNodes(const std::shared_ptr<Triangle3D<T>>& triangle) = 0;

  virtual void removeNode(SpaceNode<T>* node) = 0;

  virtual void addNode(SpaceNode<T>* node) = 0;

  virtual SpaceNode<T>* getFirstNode() const = 0;

  virtual std::string toString() const = 0;

 private:
  AbstractTriangulationNodeOrganizer(const AbstractTriangulationNodeOrganizer<T>&) = delete;
  AbstractTriangulationNodeOrganizer<T>& operator=(const AbstractTriangulationNodeOrganizer<T>& ) = delete;
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif // SPATIAL_ORGANIZATION_ABSTRACT_TRIANGULATION_NODE_ORGANIZER_H_
