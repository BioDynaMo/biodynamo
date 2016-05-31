#ifndef SPATIAL_ORGANIZATION_SIMPLE_TRIANGULATION_NODE_ORGANIZER_H_
#define SPATIAL_ORGANIZATION_SIMPLE_TRIANGULATION_NODE_ORGANIZER_H_

#include "spatial_organization/abstract_triangulation_node_organizer.h"

namespace bdm {
namespace spatial_organization {

template<class T> class BinaryTreeElement;

/**
 * This class is a very simple implementation of {@link AbstractTriangulationNodeOrganizer}.
 * All nodes are stored in a binary tree in order to obtain a good performance when
 * checking whether a certain node is already added to this organizer.
 *
 * @param <T> The type of user objects associated with the nodes in the current triangulation.
 */
template<class T>
class SimpleTriangulationNodeOrganizer : public AbstractTriangulationNodeOrganizer<T> {
 public:
  static std::shared_ptr<SimpleTriangulationNodeOrganizer<T>> create();

  SimpleTriangulationNodeOrganizer();

  virtual ~SimpleTriangulationNodeOrganizer();

  virtual void removeNode(SpaceNode<T>* node) override;

  virtual void addNode(SpaceNode<T>* node) override;

  virtual SpaceNode<T>* getFirstNode() const override;

  virtual std::string toString() const override;

  std::vector<SpaceNode<T>*> getNodes(SpaceNode<T>* reference_point) override;

  bool equalTo(const std::shared_ptr<SimpleTriangulationNodeOrganizer<T>>& other);

 private:
  BinaryTreeElement<T>* tree_head_;

  SimpleTriangulationNodeOrganizer(const SimpleTriangulationNodeOrganizer<T>&) = delete;
  SimpleTriangulationNodeOrganizer& operator=(const SimpleTriangulationNodeOrganizer<T>&) = delete;
};

}  // namespace spatial_organization
}  // namespace bdm

#endif // SPATIAL_ORGANIZATION_SIMPLE_TRIANGULATION_NODE_ORGANIZER_H_
