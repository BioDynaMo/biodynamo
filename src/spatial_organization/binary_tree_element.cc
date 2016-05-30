#include "spatial_organization/binary_tree_element.h"

#include <stdint.h>

#include <stack>
#include <sstream>

#include "string_util.h"
#include "physics/physical_node.h"
#include "spatial_organization/space_node.h"

namespace cx3d {
namespace spatial_organization {

template<class T>
BinaryTreeElement<T>* BinaryTreeElement<T>::generateTreeHead() {
  return new TreeHead<T>();
}

template<class T>
BinaryTreeElement<T>::BinaryTreeElement(SpaceNode<T>* content)
    : content_ { content },
      smaller_ { nullptr },
      bigger_ { nullptr } {
  if (content_ != nullptr) {
    content_id_ = getHash(content);
  } else {
    content_id_ = -1;
  }
}

template<class T>
BinaryTreeElement<T>::~BinaryTreeElement() {
  delete smaller_;
  delete bigger_;
}

template<class T>
bool BinaryTreeElement<T>::contains(
    SpaceNode<T>* content) const {
  return contains(getHash(content), content);
}

template<class T>
void BinaryTreeElement<T>::insert(
    SpaceNode<T>* content) {
  if (content != nullptr) {
    insert(new BinaryTreeElement<T>(content));
  }
}

template<class T>
void BinaryTreeElement<T>::remove(SpaceNode<T>* content,
                                  BinaryTreeElement* parent) {
  remove(getHash(content), content, parent);
}

template<class T>
std::string BinaryTreeElement<T>::toString() const {
  stringstream str_stream;
  str_stream << StringUtil::toStr(smaller_);
  str_stream << ", ";
  str_stream << StringUtil::toStr(content_);
  str_stream << ", ";
  str_stream << StringUtil::toStr(bigger_);
  return str_stream.str();
}

template<class T>
int BinaryTreeElement<T>::getHash(SpaceNode<T>* content) const {
  uint64_t id = content_->getId();
  uint64_t c = 7481;
  return (id * c) % 74317;
}

template<class T>
bool BinaryTreeElement<T>::contains(
    int id, SpaceNode<T>* content) const {
  return contains(getHash(content), content);
}

template<class T>
void BinaryTreeElement<T>::insert(BinaryTreeElement* element) {
  if (content_id_ == element->content_id_
      && content_ == element->content_) {
    delete element;
    return;
  } else if ((content_id_ >= element->content_id_)) {
    if ((smaller_ != nullptr)) {
      smaller_->insert(element);
    } else {
      smaller_ = element;
    }
  } else if (content_id_ < element->content_id_) {
    if (bigger_ != nullptr) {
      bigger_->insert(element);
    } else {
      bigger_ = element;
    }
  }
}

template<class T>
void BinaryTreeElement<T>::remove(int id,
                                  SpaceNode<T>* content,
                                  BinaryTreeElement* parent) {
  if ((content_id_ == id) && (content_ == content)) {
    if ((smaller_ == nullptr) && (bigger_ == nullptr)) {
      parent->changeLink(this, nullptr);
      //use of randomization in the next if showed no influence on the simulation outcome
    } else if ((smaller_ != nullptr) || (bigger_ == nullptr)) {
      parent->changeLink(this, smaller_);
      if (bigger_ != nullptr) {
        smaller_->insert(bigger_);
      }
    } else {
      parent->changeLink(this, bigger_);
      if (smaller_ != nullptr) {
        bigger_->insert(smaller_);
      }
    }
  } else {
    if ((content_id_ >= id) && (smaller_ != nullptr)) {
      smaller_->remove(id, content, this);

    } else if ((content_id_ < id) && (bigger_ != nullptr)) {
      bigger_->remove(id, content, this);
    }
  }
}

template<class T>
void BinaryTreeElement<T>::changeLink(BinaryTreeElement* old_el,
                                      BinaryTreeElement* new_el) {
  if (smaller_ == old_el) {
    smaller_ = new_el;
  } else if (bigger_ == old_el) {
    bigger_ = new_el;
  }
}

template<class T>
std::list<SpaceNode<T>*>BinaryTreeElement<T>::inOrderTraversal() const {
  std::list<SpaceNode<T>*> traversal;
  std::stack<const BinaryTreeElement<T>*> stack;
  const BinaryTreeElement<T>* dummy = this;
  while(dummy != nullptr) {
    stack.push(dummy);
    dummy = dummy->smaller_;
  }

  while(!stack.empty()) {
    dummy = stack.top();
    stack.pop();
    auto it = dummy->bigger_;
    while(it != nullptr) {
      stack.push(it);
      it = it->smaller_;
    }
    traversal.push_back(dummy->content_);
  }
  return traversal;
}

//-------------------------------------------------------------------------------------------------
// TreeHead

template<class T>
TreeHead<T>::TreeHead()
    : BinaryTreeElement<T>(nullptr) {

}

template<class T>
bool TreeHead<T>::contains(SpaceNode<T>* content) const {
  return
      BinaryTreeElement<T>::bigger_ != nullptr ?
          BinaryTreeElement<T>::bigger_->contains(content) : false;
}

template<class T>
void TreeHead<T>::insert(SpaceNode<T>* content) {
  if (BinaryTreeElement<T>::bigger_ != nullptr) {
    BinaryTreeElement<T>::bigger_->insert(content);
  } else {
    BinaryTreeElement<T>::bigger_ = new BinaryTreeElement<T>(content);
  }
}

template<class T>
void TreeHead<T>::remove(SpaceNode<T>* content,
                         BinaryTreeElement<T>* parent) {
  if (BinaryTreeElement<T>::bigger_ != nullptr) {
    BinaryTreeElement<T>::bigger_->remove(content, this);
  }
}

template<class T>
std::list<SpaceNode<T>*>TreeHead<T>::inOrderTraversal() const {
  if(BinaryTreeElement<T>::bigger_ != nullptr) {
    return BinaryTreeElement<T>::bigger_->inOrderTraversal();
  }
  return std::list<SpaceNode<T>*>();
}

template class BinaryTreeElement<cx3d::physics::PhysicalNode>;

}  // namespace spatial_organization
}  // namespace cx3d
