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
  AbstractLocalBiologyModule();

  virtual ~AbstractLocalBiologyModule();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual void run() override = 0;

  virtual CellElement* getCellElement() const override;

  virtual void setCellElement(CellElement* cell_element) override;

  virtual UPtr getCopy() const override = 0;

  virtual bool isCopiedWhenNeuriteBranches() const override;

  virtual bool isCopiedWhenSomaDivides() const override;

  virtual bool isCopiedWhenNeuriteElongates() const override;

  virtual bool isCopiedWhenNeuriteExtendsFromSoma() const override;

  virtual bool isDeletedAfterNeuriteHasBifurcated() const override;

 protected:
  CellElement* cell_element_ = nullptr;

 private:
  AbstractLocalBiologyModule(const AbstractLocalBiologyModule&) = delete;
  AbstractLocalBiologyModule& operator=(const AbstractLocalBiologyModule&) = delete;
};

}  // namespace local_biology
}  // namespace cx3d

#endif  // LOCAL_BIOLOGY_ABSTRACT_LOCAL_BIOLOGY_MODULE_H_
