#ifndef JAVA_DEFINED_CLASS_TEST_H_
#define JAVA_DEFINED_CLASS_TEST_H_

#include <array>
#include <list>
#include <stdexcept>
#include <memory>

namespace cx3d {

/**
 * Class declaration that is implemented in Java
 * only defines methods that needs to be accessed by the C++ part
 */
class NotPorted {
 public:
  virtual ~NotPorted() {
  }

  virtual int multBy2(int i) {
    throw std::runtime_error(
        "must never be called - Java must provide implementation");
    return 0;
  }
};

/**
 * This class has already been ported to C++, but needs to interact with NotPorted
 */
class Ported {
 private:
  NotPorted* not_ported_;

 public:
  explicit Ported(NotPorted* not_ported)
      : not_ported_(not_ported) {
  }

  virtual ~Ported() {
    delete not_ported_;
  }

  virtual int multBy4(int i) {
    int by_2 = not_ported_->multBy2(i);
    return not_ported_->multBy2(by_2);
  }

  virtual int multBy2Cpp(int i) {
    return i << 1;
  }

  virtual void callJdcMultby2(int num_calls) {
    for (int i = 0; i < num_calls; i++) {
      not_ported_->multBy2(i);
    }
  }

  virtual std::array<std::shared_ptr<NotPorted>, 2> getNotPortedArray(
      const std::array<std::shared_ptr<NotPorted>, 2>& arr) {
    return arr;
  }

  virtual std::shared_ptr<NotPorted> getNotPorted() {
    return std::shared_ptr<NotPorted>(not_ported_);
  }

  virtual std::shared_ptr<NotPorted> getNotPorted1(
      const std::shared_ptr<NotPorted>& arg) {
    return std::shared_ptr<NotPorted>(not_ported_);
  }

  virtual std::list<std::shared_ptr<NotPorted>> getNotPortedList(
      const std::list<std::shared_ptr<NotPorted>>& list) {
    return list;
  }
};

// --------------------------------------------------------------------------------
// same scenario but with templated classes
// --------------------------------------------------------------------------------

/**
 * Class declaration that is implemented in Java
 * only defines methods that needs to be accessed by the C++ part
 */
template<class T>
class NotPortedTemplated {
 public:
  virtual ~NotPortedTemplated() {
  }

  virtual int multBy2(int i) {
    throw std::runtime_error(
        "must never be called - Java must provide implementation");
    return 0;
  }
};

/**
 * This class has already been ported to C++, but needs to interact with NotPortedTemplated
 */
template<class T>
class PortedTemplated {
 private:
  NotPortedTemplated<T>* not_ported_templated_;

 public:
  explicit PortedTemplated(NotPortedTemplated<T>* not_ported_templated)
      : not_ported_templated_(not_ported_templated) {
  }

  virtual ~PortedTemplated() {
    delete not_ported_templated_;
  }

  virtual int multBy4(int i) {
    int by_2 = not_ported_templated_->multBy2(i);
    return not_ported_templated_->multBy2(by_2);
  }

  virtual std::shared_ptr<NotPortedTemplated<T>> getNotPortedTemplated() {
    return std::shared_ptr<NotPortedTemplated<T>>(not_ported_templated_);
  }

  virtual std::array<std::shared_ptr<NotPortedTemplated<T>>, 2> getNotPortedTemplatedArray(
      const std::array<std::shared_ptr<NotPortedTemplated<T>>, 2>& arr) {
    return arr;
  }

  virtual std::list<std::shared_ptr<NotPortedTemplated<T>> > getNotPortedemplatedList(
      const std::list<std::shared_ptr<NotPortedTemplated<T>> >& list) {
    return list;
  }
};

}  // namespace cx3d

#endif  // JAVA_DEFINED_CLASS_TEST_H_
