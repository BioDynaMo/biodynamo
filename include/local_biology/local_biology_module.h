#ifndef LOCAL_BIOLOGY_LOCAL_BIOLOGY_MODULE_H_
#define LOCAL_BIOLOGY_LOCAL_BIOLOGY_MODULE_H_

#include <memory>
#include <exception>

#include "sim_state_serializable.h"

namespace bdm {
namespace local_biology {

class CellElement;

/**
 * Classes implementing this interface can be added in the CellElements, and be run.
 * They represent the biological model that BioDynaMo is simulating.
 * Each instance of a localBiologyModule "lives" inside a particular CellElement.
 * At SomaElement division or NeuriteElement branching, a cloned version is inserted into the new CellElement.
 * If the clone() method is overwritten to return null, than the new CellElement doesn't contain a copy of the module.
 */
class LocalBiologyModule : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<LocalBiologyModule>;

  LocalBiologyModule() {
  }

  virtual ~LocalBiologyModule() {
  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override = 0;

  /** Performs your specific action */
  virtual void run() = 0;

  /** returns the cell element this module lives in */
  virtual CellElement* getCellElement() const = 0;

  /** @param cell_element the cell element this module lives in */
  virtual void setCellElement(CellElement* cell_element) = 0;

  /** returns a copy of itself */
  virtual UPtr getCopy() const = 0;

  /**
   * Specifies if instances of LocalBiologicalModules are are copied into new branches.
   */
  virtual bool isCopiedWhenNeuriteBranches() const = 0;

  /**
   * Specifies if instances of LocalBiologicalModules are copied when the soma divides.
   */
  virtual bool isCopiedWhenSomaDivides() const = 0;

  /**
   * Specifies if instances of LocalBiologicalModules are copied when the neurite elongates
   * (not in new branches!).
   */
  virtual bool isCopiedWhenNeuriteElongates() const = 0;

  /**
   * Specifies if instances of LocalBiologicalModules are copied into NeuriteElements in case of
   * extension of a new neurte from a soma.
   */
  virtual bool isCopiedWhenNeuriteExtendsFromSoma() const = 0;

  /**
   * Specifies if instances of LocalBiologicalModules are deleted in a NeuriteElement that
   * has just bifurcated (and is thus no longer a terminal neurite element).
   */
  virtual bool isDeletedAfterNeuriteHasBifurcated() const = 0;

 private:
  LocalBiologyModule(const LocalBiologyModule&) = delete;
  LocalBiologyModule& operator=(const LocalBiologyModule&) = delete;

  ClassDefOverride(LocalBiologyModule, 0);
};

}  // namespace local_biology
}  // namespace bdm

#endif  // LOCAL_BIOLOGY_LOCAL_BIOLOGY_MODULE_H_
