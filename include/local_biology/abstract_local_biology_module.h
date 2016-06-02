#ifndef LOCAL_BIOLOGY_ABSTRACT_LOCAL_BIOLOGY_MODULE_H_
#define LOCAL_BIOLOGY_ABSTRACT_LOCAL_BIOLOGY_MODULE_H_

#include <memory>
#include <exception>

#include "local_biology/local_biology_module.h"

namespace bdm {
namespace local_biology {

/**
 * Abstract class implementing the <code>LocalBiologyModule</code> interface. This class can be extended
 * to design new local modules. By default, each copy method returns <code>false</code>.
 */
class AbstractLocalBiologyModule : public LocalBiologyModule {
 public:
  AbstractLocalBiologyModule();

  virtual ~AbstractLocalBiologyModule();

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  void run() override = 0;

  CellElement* getCellElement() const override;

  void setCellElement(CellElement* cell_element) override;

  UPtr getCopy() const override = 0;

  bool isCopiedWhenNeuriteBranches() const override;

  bool isCopiedWhenSomaDivides() const override;

  bool isCopiedWhenNeuriteElongates() const override;

  bool isCopiedWhenNeuriteExtendsFromSoma() const override;

  bool isDeletedAfterNeuriteHasBifurcated() const override;

 protected:
  CellElement* cell_element_ = nullptr;

 private:
  AbstractLocalBiologyModule(const AbstractLocalBiologyModule&) = delete;
  AbstractLocalBiologyModule& operator=(const AbstractLocalBiologyModule&) = delete;
};

}  // namespace local_biology
}  // namespace bdm

#endif  // LOCAL_BIOLOGY_ABSTRACT_LOCAL_BIOLOGY_MODULE_H_
