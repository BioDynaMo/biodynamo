#ifndef DEBUG_APPROACH_TEST_H_
#define DEBUG_APPROACH_TEST_H_

#include <string>
#include <array>
#include <memory>
#include <iostream>
#include <initializer_list>

#include "string_util.h"

namespace cx3d {

/**
 * Class that should be debugged
 */
class ClassToBeDebugged : public std::enable_shared_from_this<ClassToBeDebugged> {
 protected:
  explicit ClassToBeDebugged(double d) {
    std::cout << "inside constructor with param " << d << std::endl;
  }

 public:
  static std::shared_ptr<ClassToBeDebugged> create(double d);

  virtual ~ClassToBeDebugged() {
  }

  virtual void voidMethod(std::string s, int i) {
    std::cout << "inside voidMethod with param " << s << "and " << i << std::endl;
  }

  virtual std::array<int, 2> noParameterMethod() {
    std::array<int, 2> ret = { 2, 6 };
    return ret;
  }

  virtual void callOtherMethod() {
    std::cout << "inside callOtherMethod" << std::endl;
    voidMethod("call nested method", 5);
  }

  virtual bool equalTo(const std::shared_ptr<ClassToBeDebugged>& other) {
    return other.get() == this;
  }

  virtual std::string toString() const {
    return "toString of TestClass";
  }
};

class ClassToBeDebuggedDebug : public ClassToBeDebugged {
 public:
  explicit ClassToBeDebuggedDebug(double d)
      : ClassToBeDebugged(0) {
    logConstr("ClassToBeDebugged", d);
  }

  virtual ~ClassToBeDebuggedDebug() {
  }

  void voidMethod(std::string s, int i) override {
    logCall(s, i);
    ClassToBeDebugged::voidMethod(s, i);
    logReturnVoid();
  }

  std::array<int, 2> noParameterMethod() override {
    logCallParameterless();
    auto ret = ClassToBeDebugged::noParameterMethod();
    logReturn(ret);
    return ret;
  }

  void callOtherMethod() {
    logCallParameterless();
    ClassToBeDebugged::callOtherMethod();
    logReturnVoid();
  }

  bool equalTo(const std::shared_ptr<ClassToBeDebugged>& other) override {
    logCall(other);
    auto ret = ClassToBeDebugged::equalTo(other);
    logReturn(ret);
    return ret;
  }
};

}  // namespace cx3d

#endif  // DEBUG_APPROACH_TEST_H_

