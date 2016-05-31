#ifndef LOCAL_BIOLOGY_CELL_ELEMENT_H_
#define LOCAL_BIOLOGY_CELL_ELEMENT_H_

#include <list>
#include <vector>
#include <memory>
#include <string>
#include <exception>

#include "sim_state_serializable.h"
#include "local_biology_module.h"
#include "physics/physical_object.h"

namespace cx3d {

namespace cells {
class Cell;
}  // namespace cells

namespace simulation {
class ECM;
}  // namespace simulation

namespace local_biology {

class LocalBiologyModule;

using cells::Cell;
using physics::PhysicalObject;
using simulation::ECM;

/**
 * Super class for the local biological discrete elements (SomaElement & NeuriteElement).
 * Contains a <code>Vector</code> of <code>LocalBiologyModule</code>.
 */
class CellElement : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<CellElement>;

  static void reset();

  CellElement();

  virtual ~CellElement();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual std::string toString() const;

  /** Adds the argument to the <code>LocalBiologyModule</code> list, and registers this as it's
   * <code>CellElements</code>.*/
  virtual void addLocalBiologyModule(LocalBiologyModule::UPtr m);

  /** Removes the argument from the <code>LocalBiologyModule</code> list.*/
  virtual void removeLocalBiologyModule(LocalBiologyModule* m);

  /** Removes all the <code>LocalBiologyModule</code> in this <code>CellElements</code>.*/
  virtual void cleanAllLocalBiologyModules();

  /** Returns the localBiologyModule List (not a copy).*/
  virtual std::vector<LocalBiologyModule*> getLocalBiologyModulesList();

  // *************************************************************************************
  // *      METHODS FOR SETTING CELL                                                     *
  // *************************************************************************************

  /**
   * Sets the <code>Cell</code> this <code>CellElement</code> is part of.
   * @param cell
   */
  virtual void setCell(Cell* c);

  /**
   *
   * @return the <code>Cell</code> this <code>CellElement</code> is part of.
   */
  virtual Cell* getCell() const;

  // *************************************************************************************
  // *      METHODS FOR DEFINING TYPE (neurite element vs soma element)                                                  *
  // *************************************************************************************

  /** Returns <code>true</code> if is a <code>NeuriteElement</code>. */
  virtual bool isANeuriteElement() const = 0;
  /** Returns <code>true</code> if is a <code>SomaElement</code>. */
  virtual bool isASomaElement() const = 0;

  // *************************************************************************************
  // *      METHODS FOR CALLS TO PHYSICS (POSITION, ETC)                                 *
  // *************************************************************************************

  /** Returns the location of the point mass of the <code>PhysicalObject</code>
   * associated with this <code>CellElement</code>.
   */
  virtual std::array<double, 3> getLocation();

  /** The <code>PhysicalSphere or <code>PhysicalCylinder</code> linked with this <code>CellElement</code>.*/
  virtual PhysicalObject* getPhysical() const = 0;

  /** The <code>PhysicalSphere or <code>PhysicalCylinder</code> linked with this <code>CellElement</code>.*/
  virtual void setPhysical(PhysicalObject::UPtr p) = 0;

  /**
   * Displaces the point mass of the <code>PhysicalObject</code> associated with
   * this <code>CellElement</code>.
   * @param speed in microns/hours
   * @param direction (norm not taken into account)
   */
  virtual void move(double speed, std::array<double, 3>& direction);

 protected:
  static ECM* ecm_;

  Cell* cell_ = nullptr;

  std::vector<LocalBiologyModule*> local_biology_modules_;

  /**
  * Calls the run() method in all the <code>SubElements</code>.
  * Is done automatically during the simulation, and thus doesn't have to be called by the user
  */
  void runLocalBiologyModules();

 private:
  CellElement(const CellElement&) = delete;
  CellElement& operator=(const CellElement&) = delete;

  static std::size_t id_counter_;

  /** Unique identification for this CellElement instance.*/
  std::size_t id_;
};

}  // namespace local_biology
}  // namespace cx3d

#endif  // LOCAL_BIOLOGY_CELL_ELEMENT_H_
