#ifndef LOCAL_BIOLOGY_ABSTRACT_LOCAL_BIOLOGY_MODULE_H_
#define LOCAL_BIOLOGY_ABSTRACT_LOCAL_BIOLOGY_MODULE_H_

#include <memory>
#include <exception>

#include "local_biology/local_biology_module.h"

namespace cx3d {
namespace local_biology {

/**
 * Abstract class implementing the <code>LocalBiologyModule</code> interface. This class can be extended
 * to design new local modules. By default, each copy method returns <code>false</code>.
 */
class AbstractLocalBiologyModule : public LocalBiologyModule {
 public:
  AbstractLocalBiologyModule() {
  }

  virtual ~AbstractLocalBiologyModule() {
  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override {
    sb.append("{");
    //cellElement is circular reference
    return sb;
  }

  virtual void run() override = 0;

  virtual CellElement* getCellElement() const override {
    return cell_element_;
  }

  virtual void setCellElement(CellElement* cell_element) override {
    cell_element_ = cell_element;
  }

  virtual UPtr getCopy() const override = 0;

  virtual bool isCopiedWhenNeuriteBranches() const override {
    return false;
  }

  virtual bool isCopiedWhenSomaDivides() const override {
    return false;
  }

  virtual bool isCopiedWhenNeuriteElongates() const override {
    return false;
  }

  virtual bool isCopiedWhenNeuriteExtendsFromSoma() const override {
    return false;
  }

  virtual bool isDeletedAfterNeuriteHasBifurcated() const override {
    return false;
  }

 protected:
  CellElement* cell_element_ = nullptr;

 private:
  AbstractLocalBiologyModule(const AbstractLocalBiologyModule&) = delete;
  AbstractLocalBiologyModule& operator=(const AbstractLocalBiologyModule&) = delete;
};

}  // namespace local_biology
}  // namespace cx3d

#endif  // LOCAL_BIOLOGY_ABSTRACT_LOCAL_BIOLOGY_MODULE_H_
