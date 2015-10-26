#include <debug_approach_test.h>

namespace cx3d {

std::shared_ptr<ClassToBeDebugged> ClassToBeDebugged::create(double d) {
  ClassToBeDebuggedDebug* obj = new ClassToBeDebuggedDebug(d);
  std::shared_ptr < ClassToBeDebugged > ret(obj);
  return ret;
}

}  // namespace cx3d
