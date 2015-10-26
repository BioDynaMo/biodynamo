#ifndef LIST_ITERATOR_CPP_H_
#define LIST_ITERATOR_CPP_H_

#include <list>

namespace cx3d {

/**
 * LisIteratorCpp is a helper class used by the std_list_typemap
 * that performs all the iterator modifications needed to access
 * a std::list as AbstractSequentialList on the Java side
 */
template<class T>
class ListIteratorCpp {
 public:
  typedef const T& const_reference;

  explicit ListIteratorCpp(std::list<T>* l)
      : iterator_(l->begin()),
        list_ptr_(l),
        index_(0) {
  }

  virtual ~ListIteratorCpp() {
  }

  bool hasNext() {
    return iterator_ != list_ptr_->end();
  }

  T next() {
    T next = *iterator_;
    incrementIterator();
    return next;
  }

  bool hasPrevious() {
    return iterator_ != list_ptr_->begin();
  }

  T previous() {
    auto elem = *iterator_;
    decrementIterator();
    return elem;
  }

  int nextIndex() {
    return index_ + 1;
  }

  int previousIndex() {
    return index_ - 1;
  }

  void remove() {
    --iterator_;
    list_ptr_->erase(iterator_);
    iterator_ = list_ptr_->begin();
    index_ = 0;
  }

  void set(const_reference value) {
    decrementIterator();
    *iterator_ = value;
  }

  void add(const_reference value) {
    list_ptr_->push_back(value);
    iterator_ = list_ptr_->begin();
    index_ = 0;
  }

  void incrementIterator() {
    ++index_;
    ++iterator_;
  }

  void decrementIterator() {
    --index_;
    --iterator_;
  }

 private:
  typename std::list<T>::iterator iterator_;
  std::list<T>* list_ptr_;
  int index_;
};

}  // namespace cx3d

#endif  // LIST_ITERATOR_CPP_H_
