#ifndef SPATIAL_ORGANIZATION_BINARY_TREE_ELEMENT_H_
#define SPATIAL_ORGANIZATION_BINARY_TREE_ELEMENT_H_

#include <list>
#include <memory>
#include <string>

namespace cx3d {
namespace spatial_organization {

template<class T> class SpaceNode;
template<class T> class SimpleTriangulationNodeOrganizer;

template<class T>
class BinaryTreeElement {
  friend class SimpleTriangulationNodeOrganizer<T>;
 public:
  static BinaryTreeElement<T>* generateTreeHead();

  BinaryTreeElement(std::shared_ptr<SpaceNode<T>> content);
  virtual ~BinaryTreeElement();

  virtual bool contains(const std::shared_ptr<SpaceNode<T>>& content) const;

  virtual void insert(const std::shared_ptr<SpaceNode<T>>& content);

  virtual void remove(const std::shared_ptr<SpaceNode<T>>& content,
                      BinaryTreeElement<T>* parent);

  // todo replace with STL iterator
  virtual std::list<std::shared_ptr<SpaceNode<T>>>inOrderTraversal() const;

  std::string toString() const;

 protected:
  std::shared_ptr<SpaceNode<T>> content_;
  BinaryTreeElement<T>* smaller_;
  BinaryTreeElement<T>* bigger_;
  int content_id_ = 0;

 private:
  BinaryTreeElement() = delete;
  BinaryTreeElement(const BinaryTreeElement<T>&) = delete;
  BinaryTreeElement<T>& operator=(const BinaryTreeElement<T>&) = delete;

  int getHash(std::shared_ptr<SpaceNode<T>> content) const;

  bool contains(int id, const std::shared_ptr<SpaceNode<T>>& content) const;

  void insert(BinaryTreeElement* element);

  void remove(int id, const std::shared_ptr<SpaceNode<T>>& content, BinaryTreeElement<T>* parent);

  void changeLink(BinaryTreeElement<T>* old_el, BinaryTreeElement<T>* new_el);
};

template<class T>
class TreeHead : public BinaryTreeElement<T> {
 public:
  TreeHead();

  bool contains(const std::shared_ptr<SpaceNode<T>>& content) const override;

  void insert(const std::shared_ptr<SpaceNode<T>>& content) override;

  void remove(const std::shared_ptr<SpaceNode<T>>& content,
              BinaryTreeElement<T>* parent) override;

  std::list<std::shared_ptr<SpaceNode<T>>>inOrderTraversal() const override;

private:
  TreeHead(const TreeHead&) = delete;
  TreeHead& operator=(const TreeHead&) = delete;
};

} // namespace spatial_organization
} // namespace cx3d

#endif // SPATIAL_ORGANIZATION_BINARY_TREE_ELEMENT_H_
