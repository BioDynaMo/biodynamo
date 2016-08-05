#ifndef CELLS_CELL_H_
#define CELLS_CELL_H_

#include <array>
#include <string>
#include <vector>
#include <memory>

#include "color.h"
#include "sim_state_serializable.h"
#include "local_biology/soma_element.h"
#include "local_biology/neurite_element.h"
#include "cells/cell_module.h"

namespace bdm {

namespace simulation {
class ECM;
}  // namespace simulation

namespace cells {

using local_biology::SomaElement;
using local_biology::NeuriteElement;
using simulation::ECM;

/**
 * Class <code>Cell</code> implements the cell at biological level. Every cell is characterized
 * by a unique cellId, cellType (cell state), <code>LyonCellCycle</code> (cell cycle) and are eventually
 * organized in a cell lineage tree (<code>CellLinNode</code>).
 * This class contains the genome (for now a list of <code>Gene</code>), a list of <code>GeneSubstance</code>
 * (seen as the product of the genes in the Gene vector), and is characterized by a cell type (defined by the
 * relative concentrations of the GeneSubstances.
 */
class Cell : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<Cell>;

  /** defines types for the NeuroML export*/
  enum NeuroMLType {
    kInhibitory,
    kExcitatatory
  };

  static void reset();

  /**
   * Generate <code>Cell</code>. and registers the <code>Cell</code> to <code>ECM<</code>.
   * Every cell is identified by a unique cellID number.
   */
  Cell(TRootIOCtor*) { }  // only used for ROOT I/O

  Cell();

  ~Cell();

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  std::string toString() const;

  /**
   * Run Cell: run <code>Gene</code>, run <code>LyonCellCycle</code>, run Conditions, run EnergyProduction.
   * We move one step further in the simulation by running the <code>Gene</code>, <code>GeneSubstances</code>,
   * the <code>LyonCellCycle</code>, EnergyProduction and than we test conditions with ConditionTester.
   */
  void run();

  // *************************************************************************************
  // *      METHODS FOR DIVISION                                                         *
  // *************************************************************************************

  /**
   * Divide the cell. Of the two daughter cells, one is this one (but smaller, with half GeneSubstances etc.),
   * and the other one is instantiated de novo and is returned. Both cells have more or less the same volume,
   * the axis of division is random.
   * @return the other daughter cell.
   */
  Cell* divide();

  /**
   * Divide the cell. Of the two daughter cells, one is this one (but smaller, with half GeneSubstances etc.),
   * and the other one is instantiated de novo and is returned. The axis of division is random.
   * @param volumeRatio the ratio (Volume daughter 1)/(Volume daughter 2). 1.0 gives equal cells.
   * @return the second daughter cell.
   */
  Cell* divide(double volume_ratio);

  /**
   * @param axis specifies direction of division
   */
  Cell* divide(const std::array<double, 3>& axis);

  /**
   * Divide the cell. Of the two daughter cells, one is this one (but smaller, with half GeneSubstances etc.),
   * and the other one is instantiated de novo and is returned. The axis of division is random.
   * @param volumeRatio the ratio (Volume daughter 1)/(Volume daughter 2). 1.0 gives equal cells.
   * @param axis specifies direction of division
   * @return the second daughter cell
   */
  Cell* divide(double volume_ratio, const std::array<double, 3>& axis);

  /**
   * Divide mother cell in two daughter cells by coping <code>Cell</code>, <code>SomaElement</code>,
   * <code>PhysicalSphere</code>, list of <code>CellModules</code>.
   * <code>CellSubstances</code> are dispatched in the two cells.
   * The <code>CellClock</code>  and cell lineage, if present, are also copied..
   * When mother cell divides, by definition:
   * 1) the mother cell becomes the 1st daughter cell
   * 2) the new cell becomes the 2nd daughter cell and inherits a equal or bigger volume than the 1st
   *    daughter cell, which means that this cell will eventually inherit more differentiating factors
   *    and will be recorded in the left side of the lineage tree.
   *
   * @return the second daughter cell
   */
  Cell* divide(double volume_ratio, double phi, double theta);

  void addNeuriteElement(NeuriteElement::UPtr neurite);

  // *************************************************************************************
  // *      METHODS FOR CELL MODULES                                                     *
  // *************************************************************************************

  /**
   * Adds a <code>CellModule</code> that will be run at each time step.
   * @param m
   */
  void addCellModule(CellModule::UPtr m);
  /**
   * Removes a particular <code>CellModule</code> from this <code>Cell</code>.
   * It will therefore not be run anymore.
   * @param m
   */
  void removeCellModule(CellModule* m);

  /** Removes all the <code>CellModule</code> in this <code>Cell</code>.*/
  void cleanAllCellModules();

  /**
   * Sets the color for all the <code>PhysicalObjects</code> associated with the
   * <code>CellElements</code> of this Cell..
   * @param color
   */
  void setColorForAllPhysicalObjects(bdm::Color color);

  // *************************************************************************************
  // *      GETTERS & SETTERS                                                            *
  // *************************************************************************************

  /** Currently, there are two types of cells : Inhibitory_cells and Excitatory_cells.*/
  void setNeuroMLType(NeuroMLType neuro_ml_type);

  /** Currently, there are two types of cells :  <code>Inhibitory_cells</code> and  <code>Excitatory_cells</code>.*/
  NeuroMLType getNeuroMLType() const;

  /** Returns the cell type. This is just a convenient way to store some property for the cell.
   * Should not be confused with NeuroMLType.
   */
  std::string getType() const;

  /** Sets the cell type. This is just a convenient way to store some property for the cell.
   * Should not be confused with NeuroMLType.
   */
  void setType(const std::string& type);

  SomaElement* getSomaElement() const;

  void setSomaElement(SomaElement::UPtr soma);

  int getID() const;

  /**
   * @return a <code>Vector</code> containing all the <code>NeuriteElement</code>s of this cell.
   */
  std::vector<NeuriteElement*> getNeuriteElements() const;

 private:
  Cell(const Cell&) = delete;
  Cell& operator=(const Cell&) = delete;

  /* Unique identification for this Cell instance. */
  int id_ = 0;

  /* Counter to uniquely identify every cell. */
  static int id_counter_;

  /* Reference to the ECM. */
  static ECM* ecm_;

  /* List of all cell modules that are run at each time step*/
#ifdef __ROOTCLING__
  std::vector<CellModule*> cell_modules_;
#else
  std::vector<CellModule::UPtr> cell_modules_;
#endif

  /* List of the SomaElements belonging to the cell */
#ifdef __ROOTCLING__
  SomaElement* soma_;
#else
  SomaElement::UPtr soma_;
#endif

#ifdef __ROOTCLING__
  std::vector<NeuriteElement*> neurites_;
#else
  std::vector<NeuriteElement::UPtr> neurites_;
#endif

  /* List of the first Neurite of all Neurites belonging to the cell */
  std::vector<NeuriteElement*> neurite_root_list_;  // TODO: not working yet

  /* The electrophsiology type of this cell */
  NeuroMLType neuro_ml_type_ = NeuroMLType::kExcitatatory;

  /** Some convenient way to store properties of  for cells.
   * Should not be confused with neuroMLType. */
  std::string type_ = "";

  ClassDefOverride(Cell, 1);
};

}  // namespace cells
}  // namespace bdm

#endif  // CELLS_CELL_H_
