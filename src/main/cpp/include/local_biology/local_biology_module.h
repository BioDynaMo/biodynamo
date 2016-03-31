#ifndef LOCAL_BIOLOGY_LOCAL_BIOLOGY_MODULE_H_
#define LOCAL_BIOLOGY_LOCAL_BIOLOGY_MODULE_H_

#include <memory>
#include <exception>

namespace cx3d {

namespace local_biology {

class LocalBiologyModule {
 public:
  virtual ~LocalBiologyModule() {
  }

  virtual std::shared_ptr<LocalBiologyModule> getCopy() const {
    throw std::logic_error(
                "LocalBiologyModule::getCopy must never be called - Java must provide implementation at this point");
  }

  virtual bool isCopiedWhenNeuriteElongates() const {
    throw std::logic_error(
            "LocalBiologyModule::isCopiedWhenNeuriteElongates must never be called - Java must provide implementation at this point");
  }
};

}  // namespace local_biology
}  // namespace cx3d

#endif  // LOCAL_BIOLOGY_LOCAL_BIOLOGY_MODULE_H_
