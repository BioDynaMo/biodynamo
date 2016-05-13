#ifndef LOCAL_BIOLOGY_LOCAL_BIOLOGY_MODULE_H_
#define LOCAL_BIOLOGY_LOCAL_BIOLOGY_MODULE_H_

#include <memory>
#include <exception>

#include "sim_state_serializable.h"

namespace cx3d {
namespace local_biology {

class CellElement;

//fixme change to pure virtual function after porting has been finished
/**
 * Classes implementing this interface can be added in the CellElements, and be run.
 * They represent the biological model that CX3D is simulating.
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

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override {
    throw std::logic_error(
        "LocalBiologyModule::simStateToJson must never be called - Java must provide implementation at this point");
  }

  /** Performs your specific action */
  virtual void run() {
    throw std::logic_error(
        "LocalBiologyModule::run must never be called - Java must provide implementation at this point");
  }

  /** returns the cell element this module lives in */
  virtual CellElement* getCellElement() const {
    throw std::logic_error(
        "LocalBiologyModule::getCellElement must never be called - Java must provide implementation at this point");
  }

  /** @param cell_element the cell element this module lives in */
  virtual void setCellElement(CellElement* cell_element) {
    throw std::logic_error(
        "LocalBiologyModule::setCellElement must never be called - Java must provide implementation at this point");
  }

  /** returns a copy of itself */
  virtual UPtr getCopy() const {
    throw std::logic_error(
        "LocalBiologyModule::getCopy must never be called - Java must provide implementation at this point");
  }

  /**
   * Specifies if instances of LocalBiologicalModules are are copied into new branches.
   */
  virtual bool isCopiedWhenNeuriteBranches() const {
    throw std::logic_error(
        "LocalBiologyModule::isCopiedWhenNeuriteBranches must never be called - Java must provide implementation at this point");
  }

  /**
   * Specifies if instances of LocalBiologicalModules are copied when the soma divides.
   */
  virtual bool isCopiedWhenSomaDivides() const {
    throw std::logic_error(
        "LocalBiologyModule::isCopiedWhenSomaDivides must never be called - Java must provide implementation at this point");
  }

  /**
   * Specifies if instances of LocalBiologicalModules are copied when the neurite elongates
   * (not in new branches!).
   */
  virtual bool isCopiedWhenNeuriteElongates() const {
    throw std::logic_error(
        "LocalBiologyModule::isCopiedWhenNeuriteElongates must never be called - Java must provide implementation at this point");
  }

  /**
   * Specifies if instances of LocalBiologicalModules are copied into NeuriteElements in case of
   * extension of a new neurte from a soma.
   */
  virtual bool isCopiedWhenNeuriteExtendsFromSoma() const {
    throw std::logic_error(
        "LocalBiologyModule::isCopiedWhenNeuriteExtendsFromSoma must never be called - Java must provide implementation at this point");
  }

  /**
   * Specifies if instances of LocalBiologicalModules are deleted in a NeuriteElement that
   * has just bifurcated (and is thus no longer a terminal neurite element).
   */
  virtual bool isDeletedAfterNeuriteHasBifurcated() const {
    throw std::logic_error(
        "LocalBiologyModule::isDeletedAfterNeuriteHasBifurcated must never be called - Java must provide implementation at this point");
  }

 private:
  LocalBiologyModule(const LocalBiologyModule&) = delete;
  LocalBiologyModule& operator=(const LocalBiologyModule&) = delete;
};

}  // namespace local_biology
}  // namespace cx3d

#endif  // LOCAL_BIOLOGY_LOCAL_BIOLOGY_MODULE_H_
