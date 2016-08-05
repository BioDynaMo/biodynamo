#ifndef LOCAL_BIOLOGY_NEURITE_ELEMENT_H_
#define LOCAL_BIOLOGY_NEURITE_ELEMENT_H_

#include <vector>
#include <memory>
#include <exception>

#include "local_biology/cell_element.h"
#include "physics/physical_cylinder.h"

namespace bdm {

namespace local_biology {

class LocalBiologyModule;

using physics::PhysicalObject;
using physics::PhysicalCylinder;

/**
 * Class defining the biological properties of a neurite segment, if it contains
 * a <code>LacalBiologyModule</code>. This class is associated with a <code>PhysicalCylinder</code>.
 */
class NeuriteElement : public CellElement {
 public:
  using UPtr = std::unique_ptr<NeuriteElement>;

  NeuriteElement(TRootIOCtor*) { }  // only used for ROOT I/O
  
  NeuriteElement();

  virtual ~NeuriteElement();

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  NeuriteElement* getCopy() const;

  /** <b>Users should not use this method!</b>
   * It is called by the physicalObject associated with this neuriteElement, when it is deleted.*/
  void removeYourself();

  void run();

  /** Retracts the Cylinder associated with this NeuriteElement, if it is a terminal one.
   * @param speed the retraction speed in micron/h
   */
  void retractTerminalEnd(double speed);

  /** Moves the point mass of the Cylinder associated with this NeuriteElement, if it is a terminal one.
   *  BUT : if "direction" points in an opposite direction than the cylinder axis, i.e.
   *  if the dot product is negative, there is no movement (only elongation is possible).
   * @param speed
   * @param direction
   */
  void elongateTerminalEnd(double speed, const std::array<double, 3>& direction);

  /**
   * Makes a side branch, i.e. splits this cylinder into two and puts a daughter right at the proximal half.
   * @param newBranchDiameter
   * @param growthDirection (But will be automatically corrected if not at least 45 degrees from the cylinder's axis).
   * @return
   */
  NeuriteElement* branch(double newBranchDiameter, const std::array<double, 3>& direction);

  /**
   * Makes a side branch, i.e. splits this cylinder into two and puts a daughteRight at the proximal half.
   * @param growthDirection (But will be automatically corrected if not at least 45 degrees from the cylinder's axis).
   * @return
   */
  NeuriteElement* branch(const std::array<double, 3>& direction);

  /**
   * Makes a side branch, i.e. splits this cylinder into two and puts a daughter right at the proximal half.
   * @param diameter of the side branch
   * @return
   */
  NeuriteElement* branch(double diameter);

  /**
   * Makes a side branch, i.e. splits this cylinder into two and puts a daughter right at the proximal half.
   * @return
   */
  NeuriteElement* branch();

  /**
   Returns <code>true</code> if it is a terminal cylinder with length of at least 1micron.
   * @return
   */
  bool bifurcationPermitted() const;

  /**
   * Bifurcation of a growth come (only works for terminal segments).
   * Note : angles are corrected if they are pointing backward.
   * @param diameter_1  of new daughterLeft
   * @param diameter_2 of new daughterRight
   * @param direction_1
   * @param direction_2
   * @return
   */
  std::array<NeuriteElement*, 2> bifurcate(double diameter_1, double diameter_2,
                                           const std::array<double, 3>& direction_1,
                                           const std::array<double, 3>& direction_2);

  /**
   * Bifurcation of a growth come (only works for terminal segments).
   * Note : angles are corrected if they are pointing backward.
   * @param length of new branches
   * @param diameter_1  of new daughterLeft
   * @param diameter_2 of new daughterRight
   * @param direction_1
   * @param direction_2
   * @return
   */
  std::array<NeuriteElement*, 2> bifurcate(double length, double diameter_1, double diameter_2,
                                           const std::array<double, 3>& direction_1,
                                           const std::array<double, 3>& direction_2);

  std::array<NeuriteElement*, 2> bifurcate(const std::array<double, 3>& direction_1,
                                           const std::array<double, 3>& direction_2);

  std::array<NeuriteElement*, 2> bifurcate();

  // *************************************************************************************
  //   Synapses
  // *************************************************************************************

  /**
   * Makes spines (the physical and the biological part) on this NeuriteElement.
   * @param interval the average interval between the boutons.
   */
  void makeSpines(double interval);

  /**
   * Makes a single spine (the physical and the biological part) randomly on this NeuriteElement.
   */
  void makeSingleSpine();

  /**
   * Makes a single spine (the physical and the biological part) randomly on this NeuriteElement.
   */
  void makeSingleSpine(double dist_from_proximal_end);

  /**
   * Make boutons (the physical and the biological part) on this NeuriteElement.
   * @param interval the average interval between the boutons.
   */
  void makeBoutons(double interval);

  /**
   * Adds one bouton (the physical and the biological part) randomly on this NeuriteElement.
   */
  void makeSingleBouton(double dist_from_proximal_end);

  /**
   * Adds one bouton (the physical and the biological part) randomly on this NeuriteElement.
   */
  void makeSingleBouton();

  /**
   * Links the free boutons of this neurite element to adjacents free spines
   * @param probabilityToSynapse probability to make the link.
   * @return
   */
  int synapseBetweenExistingBS(double probability_to_synapse);

  // *************************************************************************************
  //   Getters & Setters
  // *************************************************************************************

  PhysicalObject* getPhysical() const override;

  void setPhysical(PhysicalObject::UPtr po) override;

  PhysicalCylinder* getPhysicalCylinder() const;

  void setPhysicalCylinder(PhysicalCylinder::UPtr pc);

  bool isAxon() const;

  void setAxon(bool is_axon);

  bool isANeuriteElement() const override;

  bool isASomaElement() const override;

  /**
   * @return the (first) distal <code>NeuriteElement</code>, if it exists,
   * i.e. if this is not the terminal segment (otherwise returns <code>null</code>).
   */
  NeuriteElement* getDaughterLeft() const;

  /**
   * @return the second distal <code>NeuriteElement</code>, if it exists
   * i.e. if there is a branching point just after this element (otherwise returns <code>null</code>).
   */
  NeuriteElement* getDaughterRight() const;

  /**
   * Adds to a Vector of NeuriteElements (NE) all the NE distal to this particular NE (including it).
   * @param elements the vector where it should be added.
   * @return
   */
  std::vector<NeuriteElement*> addYourselfAndDistalNeuriteElements(std::vector<NeuriteElement*>& elements);

 private:
  NeuriteElement(const NeuriteElement&) = delete;
  NeuriteElement& operator=(const NeuriteElement&) = delete;

#ifdef __ROOTCLING__
  PhysicalCylinder* physical_cylinder_;
#else
  PhysicalCylinder::UPtr physical_cylinder_;
#endif

  bool is_axon_ = false;

  ClassDefOverride(NeuriteElement, 1);
};

}  // namespace local_biology
}  // namespace bdm

#endif  // LOCAL_BIOLOGY_NEURITE_ELEMENT_H_
