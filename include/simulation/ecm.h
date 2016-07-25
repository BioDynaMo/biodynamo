#ifndef SIMULATION_ECM_H_
#define SIMULATION_ECM_H_

#include <vector>
#include <array>
#include <string>
#include <memory>
#include <exception>
#include <unordered_map>

#include <TObject.h>

#include "color.h"
#include "sim_state_serializable.h"
#include "cells/cell.h"
#include "physics/substance.h"
#include "physics/intracellular_substance.h"
#include "physics/physical_node.h"
#include "physics/physical_object.h"
#include "physics/physical_sphere.h"
#include "physics/physical_cylinder.h"
#include "physics/physical_bond.h"
#include "physics/substance.h"
#include "spatial_organization/space_node.h"

namespace bdm {

namespace local_biology {
class SomaElement;
class NeuriteElement;
}  // namespace local_biology

namespace synapse {
class PhysicalSpine;
class PhysicalBouton;
}  // namespace synapse

namespace simulation {

using cells::Cell;
using local_biology::SomaElement;
using local_biology::NeuriteElement;
using physics::PhysicalNode;
using physics::PhysicalSphere;
using physics::PhysicalCylinder;
using physics::Substance;
using physics::IntracellularSubstance;
using physics::SubstanceHash;
using physics::SubstanceEqual;
using spatial_organization::SpatialOrganizationNode;

/**
 * Contains some lists with all the elements of the simulation, and methods to add
 * or remove elements. Contains lists of "real" and "artificial" Substances.
 * Possibility to define a cubic region of space where elements are confined.
 */
class ECM : public TObject, public SimStateSerializable {
 public:
  static ECM* getInstance();

  ECM(TRootIOCtor*) { }  // only used for ROOT I/O
  virtual ~ECM();
  ECM(const ECM&) = delete;
  ECM& operator=(const ECM&) = delete;

  StringBuilder& simStateToJson(StringBuilder& sb) const override;

  // **************************************************************************
  // Artificial Wall
  // **************************************************************************
  /**
   * Set the boundaries of a pseudo wall, that maintains the PhysicalObjects in a closed volume.
   * Automatically turns on this mechanism for spheres.
   * @param Xmin
   * @param Xmax
   * @param Ymin
   * @param Ymax
   * @param Zmin
   * @param Zmax
   */
  void setBoundaries(double Xmin, double Xmax, double Ymin, double Ymax, double Zmin, double Zmax);

  /** If set to true, the PhysicalSpheres tend to stay inside a box,
   * who's boundaries are set with setBoundaries().
   * @param artificialWallsForSpheres
   */
  void setArtificialWallsForSpheres(bool artificial_walls);

  /** If true, the PhysicalSpheres tend to stay inside a box, who's boundaries are set with
   * setBoundaries().
   */
  bool getArtificialWallForSpheres() const;

  /** If set to true, the PhysicalCyliners tend to stay inside a box,
   * who's boundaries are set with setBoundaries().
   * @param artificialWallsForCylinders
   */
  void setArtificialWallsForCylinders(bool artificial_walls);

  /** If true, the PhysicalCyliners tend to stay inside a box, who's boundaries are set with
   * setBoundaries().
   */
  bool getArtificialWallForCylinders() const;

  /**
   * Returns a force that would be applied to a PhysicalSphere that left the boundaries
   * of the artificial wall.
   * @param location the center of the PhysicalSphere
   * @param radius the radius of the PhysicalSphere
   * @return [Fx,Fy,Fz] the force applied to the cell
   */
  std::array<double, 3> forceFromArtificialWall(const std::array<double, 3>& location, double radius);

  // **************************************************************************
  // SOM and Interaction with PO & CellElements (add, remove, ..)
  // **************************************************************************

  /**
   * Returns an instance of a class implementing SpatialOrganizationNode.
   * If it is the first node of the simulation, it will fix the Class type of the SpatialOrganizationNode.
   * CAUTION : NEVER call this method if there exist already SpatialOrganizationNodes in
   * the simulation, and initialNode in ECM has not been instatialized : there will then be
   * two different unconnected Delaunay
   * @param position
   * @param userObject
   * @return
   */
  SpatialOrganizationNode<PhysicalNode>::UPtr getSpatialOrganizationNodeInstance(const std::array<double, 3>& position,
                                                                                 PhysicalNode* user_object);

  /**
   * Returns an instance of a class implementing SpatialOrganizationNode.
   * If it is the first node of the simulation, it will fix the Class type of the SpatialOrganizationNode.
   * CAUTION : NEVER call this method if there exist already SpatialOrganizationNodes in
   * the simulation, and initialNode in ECM has not been instantiated : there will then be
   * two different unconnected Delaunay
   * @param n an already existing SpatialOrganizationNode close to the place where the new one should be
   * @param position
   * @param userObject
   * @return
   */
  SpatialOrganizationNode<PhysicalNode>::UPtr getSpatialOrganizationNodeInstance(
      SpatialOrganizationNode<PhysicalNode>* n, const std::array<double, 3>& position, PhysicalNode* user_object);

  /**
   * Creating some "dummy nodes", i.e. some  PhysicalSplace.
   * @param x1 lower boundary on the X axis
   * @param x2 upper boundary on the X axis
   * @param y1 upper boundary on the Y axis
   * @param y2 lower boundary on the Y axis
   * @param z1 upper boundary on the Z axis
   * @param z2 lower boundary on the Z axis
   * @param d inter-node distance
   * @return vector of unique PhysicalNode pointers - PhysicalNodes will be destructed if this vector gets destroyed
   */
  std::vector<PhysicalNode::UPtr> createGridOfPhysicalNodes(double x1, double x2, double y1, double y2, double z1,
                                                            double z2, double d);

  /**
   * Adds a "simple" PhysicalNode (with its SON) at a desired location.
   * @return unique PhysicalNode pointer
   */
  PhysicalNode::UPtr createPhysicalNodeInstance(const std::array<double, 3>& position);

  // Physical Objects-------------------------------------------
  // PhysicalCylinder and PhysicalSphere are also instances of PhysicalNode.
  // PhysicalNode contains a SpatialOrganizerNode.
  // So: add/remove PhysicalCylinder/PhysicalSphere makes a call to
  // add/remove-PhysicalNode.
  // the later also calls the remove() method of the associated SpatialOrganizationNode.

  void addPhysicalCylinder(PhysicalCylinder* cyl);

  void removePhysicalCylinder(PhysicalCylinder* cyl);

  void addPhysicalSphere(PhysicalSphere* sphere);

  void removePhysicalSphere(PhysicalSphere* sphere);

  void addPhysicalNode(PhysicalNode* node);

  void removePhysicalNode(PhysicalNode* node);

  //fixme implement
//     void addECMChemicalReaction(ECMChemicalReaction chemicalReaction);
//
//     void removeECMChemicalReaction(ECMChemicalReaction chemicalReaction);

  //  Cells

  void addCell(Cell::UPtr cell);

  void removeCell(Cell* cell);

  // Cell Elements--------------------------------------------------
  void addSomaElement(SomaElement* soma);

  void removeSomaElement(SomaElement* soma);

  void addNeuriteElement(NeuriteElement* neurite);

  void removeNeuriteElement(NeuriteElement* neurite);

  void resetTime();

  /**
   * Removes all the objects in the simulation, including SpaceNodes and the triangulation.
   */
  void clearAll();

  // *********************************************************************
  // *** Substances (real chemicals)
  // *********************************************************************

  /** Define a template for each (extra-cellular) <code>Substance</code> in the simulation that has
   * non-standard values for diffusion and degradation constant.*/
  void addNewSubstanceTemplate(Substance::UPtr s);

  /** Define a template for each <code>IntracellularSubstance</code> in the simulation that has
   * non-standard values for diffusion and degradation constant, and outside visibility and volume dependency.*/
  void addNewIntracellularSubstanceTemplate(IntracellularSubstance::UPtr s);

  /** Returns an instance of <code>Substance</code>. If a similar substance (with the same id)
   * has already been declared as a template Substance, a copy of it is made (with
   * same id, degradation and diffusion constant, but concentration and quantity 0).
   * If it is the first time that this id is requested, a new template Substance is made
   * (with by-default values) and stored, and a copy will be returned.
   * @param id
   * @return new Substance instance
   */
  Substance::UPtr substanceInstance(const std::string& id);

  /** Returns an instance of <code>IntracellularSubstance</code>. If a similar
   * IntracellularSubstance (with the same id) has already been declared as a template
   * IntracellularSubstance, a copy of it is made (with same id, degradation constant,
   * diffusion constant, outside visibility and volume dependency, but concentration
   * and quantity 0).
   * If it is the first time that this id is requested, a new template IntracellularSubstance
   *  is made (with by-default values) and stored, and a copy will be returned.
   * @param id
   * @return new IntracellularSubstance instance
   */
  IntracellularSubstance::UPtr intracellularSubstanceInstance(const std::string& id);

  // *********************************************************************
  // *** Pre-defined cellType colors
  // *********************************************************************
  void addNewCellTypeColor(const std::string& cell_type, Color color);

  Color cellTypeColor(const std::string& cell_type);

  // *********************************************************************
  // *** Artificial concentration of chemicals
  // *********************************************************************

  /** Returns true if some artificial gradient (of any sorts) have been defined.*/
  bool thereAreArtificialGradients() const;

  /**
   * Defines a bell-shaped artificial concentration in ECM, along the Z axis (ie uniform along X,Y axis).
   * It is a continuous value, and not instances of the class Substance!.
   *
   * @param substance
   * @param max_concentration the value of the concentration at its peak
   * @param z_coord the location of the peak
   * @param sigma the thickness of the layer (= the variance)
   */
  void addArtificialGaussianConcentrationZ(Substance* substance, double max_concentration, double z_coord,
                                           double sigma);

  /**
   * Defines a bell-shaped artificial concentration in ECM, along the Z axis (ie uniform along X,Y axis).
   * It is a continuous value, and not instances of the class Substance!.
   *
   * @param substance_name
   * @param max_concentatration the value of the concentration at its peak
   * @param z_coord the location of the peak
   * @param sigma the thickness of the layer (= the variance)
   */
  void addArtificialGaussianConcentrationZ(const std::string& substance_name, double max_concentatration,
                                           double z_coord, double sigma);

  /**
   * Defines a linear artificial concentration in ECM, between two points along the Z axis. Outside this interval
   * the value will be 0. Between the interval the value is the linear interpolation between
   * the maximum value and 0.
   *
   * It is a continuous value, and not instances of the class Substance!
   *
   * @param substance
   * @param max_concentatration the value of the concentration at its peak
   * @param z_coord_max the location of the peak
   * @param z_coord_min the location where the concentration reaches the value 0
   */
  void addArtificialLinearConcentrationZ(Substance* substance, double max_concentatration, double z_coord_max,
                                         double z_coord_min);

  /**
   * Defines a linear artificial concentration in ECM, between two points along the Z axis. Outside this interval
   * the value will be 0. Between the interval the value is the linear interpolation between
   * the maximum value and 0.
   *
   * It is a continuous value, and not instances of the class Substance!
   *
   * @param substance_name
   * @param max_concentatration the value of the concentration at its peak
   * @param z_coord_max the location of the peak
   * @param z_coord_min the location where the concentration reaches the value 0
   */
  void addArtificialLinearConcentrationZ(const std::string& substance_name, double max_concentatration,
                                         double z_coord_max, double z_coord_min);

  /**
   * Defines a bell-shaped artificial concentration in ECM, along the X axis (ie uniform along Y,Z axis).
   * It is a continuous value, and not instances of the class Substance!.
   *
   * @param substance
   * @param max_concentatration the value of the concentration at its peak
   * @param x_coord the location of the peak
   * @param sigma the thickness of the layer (= the variance)
   */
  void addArtificialGaussianConcentrationX(Substance* substance, double max_concentatration, double x_coord,
                                           double sigma);

  /**
   * Defines a bell-shaped artificial concentration in ECM, along the X axis (ie uniform along Y,Z axis).
   * It is a continuous value, and not instances of the class Substance!.
   *
   * @param substance_name
   * @param max_concentatration the value of the concentration at its peak
   * @param x_coord the location of the peak
   * @param sigma the thickness of the layer (= the variance)
   */
  void addArtificialGaussianConcentrationX(const std::string& substance_name, double max_concentatration,
                                           double x_coord, double sigma);

  /**
   * Defines a linear artificial concentration in ECM, between two points along the X axis. Outside this interval
   * the value will be 0. Between the interval the value is the linear interpolation between
   * the maximum value and 0.
   *
   * It is a continuous value, and not instances of the class Substance!
   *
   * @param substance
   * @param max_concentatration the value of the concentration at its peak
   * @param x_coord_max the location of the peak
   * @param x_coord_min the location where the concentration reaches the value 0
   */
  void addArtificialLinearConcentrationX(Substance* substance, double max_concentatration, double x_coord_max,
                                         double x_coord_min);

  /**
   * Defines a linear artificial concentration in ECM, between two points along the X axis. Outside this interval
   * the value will be 0. Between the interval the value is the linear interpolation between
   * the maximum value and 0.
   *
   * It is a continuous value, and not instances of the class Substance!
   *
   * @param substance_name
   * @param max_concentatration the value of the concentration at its peak
   * @param x_coord_max the location of the peak
   * @param x_coord_min the location where the concentration reaches the value 0
   */
  void addArtificialLinearConcentrationX(const std::string& substance_name, double max_concentatration,
                                         double x_coord_max, double x_coord_min);

  /**
   * Gets the value of a chemical, at a specific position in space
   * @param nameOfTheChemical
   * @param position the location [x,y,z]
   * @return
   */
  double getValueArtificialConcentration(const std::string& nameOfTheChemical,
                                         const std::array<double, 3>& position) const;

  /**
   * Gets the value of a chemical, at a specific position in space
   * @param nameOfTheChemical
   * @param position the location [x,y,z]
   * @return
   */
  double getValueArtificialConcentration(Substance* substance, const std::array<double, 3>& position) const;
  ///////// GRADIENT
  /**
   * Gets the gradient of a chemical, at a specific altitude
   * @param substance_name
   * @param position the location [x,y,z]
   * @return the gradient [dc/dx , dc/dy , dc/dz]
   */
  std::array<double, 3> getGradientArtificialConcentration(const std::string& substance_name,
                                                           const std::array<double, 3>& position) const;

  double getGradientArtificialConcentration(Substance* s, const std::array<double, 3>& position) const;

  std::vector<PhysicalNode*> getPhysicalNodeList() const;

  std::vector<PhysicalSphere*> getPhysicalSphereList() const;

  std::vector<PhysicalCylinder*> getPhysicalCylinderList() const;

  std::vector<NeuriteElement*> getNeuriteElementList() const;

  std::vector<SomaElement*> getSomaElementList() const;

  bool isAnyArtificialGradientDefined() const;

  std::unordered_map<Substance*, std::array<double, 3>, SubstanceHash, SubstanceEqual> getGaussianArtificialConcentrationZ() const;

  std::unordered_map<Substance*, std::array<double, 3>, SubstanceHash, SubstanceEqual> getLinearArtificialConcentrationZ() const;

  std::unordered_map<Substance*, std::array<double, 3>, SubstanceHash, SubstanceEqual> getGaussianArtificialConcentrationX() const;

  std::unordered_map<Substance*, std::array<double, 3>, SubstanceHash, SubstanceEqual> getLinearArtificialConcentrationX() const;

  double getECMtime() const;

  void setECMtime(double ECMtime);

  void increaseECMtime(double deltaT);

  std::array<double, 3> getMinBounds() const;

  std::array<double, 3> getMaxBounds() const;

  PhysicalCylinder* getPhysicalCylinder(int i) const;

  NeuriteElement* getNeuriteElement(int i) const;

  PhysicalNode* getPhysicalNode(int i) const;

  PhysicalSphere* getPhysicalSphere(int i) const;

  SomaElement* getSomaElement(int i) const;

  Cell* getCell(int i) const;

  int getCellListSize() const;

  int getPhysicalNodeListSize() const;

  int getPhysicalCylinderListSize() const;

  int getPhysicalSphereListSize() const;

  int getSomaElementListSize() const;

  int getNeuriteElementListSize() const;

 private:
  // List of all the BioDynaMoRunnable objects in the simulation ............................

  /** List of all the PhysicalNode instances. */
  std::vector<PhysicalNode*> physical_nodes_;
  /** List of all the PhysicalSphere instances. */
  std::vector<PhysicalSphere*> physical_spheres_;
  /** List of all the PhysicalCylinder instances. */
  std::vector<PhysicalCylinder*> physical_cylinders_;
  /** List of all the SomaElement instances. */
  std::vector<SomaElement*> soma_elements_;
  /** List of all the NeuriteElement instances. */
  std::vector<NeuriteElement*> neurite_elements_;
  /** List of all the Cell instances. */
#ifdef __ROOTCLING__
  std::vector<Cell*> cells_;
#else
  std::vector<Cell::UPtr> cells_;
#endif
  /** List of all the Chemical reactions instances. */
  //fixme implement std::vector<std::shared_ptr<physics::ECMChemicalReaction>> ecmChemicalReactionList;
  /* Reference time throughout the simulation (in hours) */
  double time_ = 0;

  /* An SON used to get new SON instances from*/
  SpatialOrganizationNode<PhysicalNode>* initial_node_ = nullptr;

  /* In here we keep a template for each (extra-cellular) Substance in the simulation that have
   * non-standard value for diffusion and degradation constant.*/
#ifdef __ROOTCLING__
  std::unordered_map<std::string, Substance*> substance_lib_;
#else
  std::unordered_map<std::string, Substance::UPtr> substance_lib_;
#endif

  /* In here we keep a template for each (intra-cellular) Substance in the simulation that have
   * non-standard value for diffusion and degradation constant.*/
#ifdef __ROOTCLING__
  std::unordered_map<std::string, IntracellularSubstance*> intracellular_substance_lib_;
#else
  std::unordered_map<std::string, IntracellularSubstance::UPtr> intracellular_substance_lib_;
#endif

  /* In here we store a color attributed to specific cell types.*/
  std::unordered_map<std::string, Color> cell_color_lib_;

  // Artificial walls ...................................................................

  // if true, PhysicalSphere instances (and only those) are forced to stay in a defined volume.
  bool articicial_walls_for_spheres_ = false;

  // if true, PhysicalCylinders instances (and only those) are forced to stay in a defined volume.
  bool articicial_walls_for_cylinders_ = false;

  /* Boundaries of the simulation area, within which the PhysicalSpheres
   * have to stay (but PhysicalCylinders and Substances don't...) */
  double x_min_ = -100;
  double x_max_ = 100;
  double y_min_ = -100;
  double y_max_ = 100;
  double z_min_ = -100;
  double z_max_ = 300;

  // Artificial gradients ...............................................................

  // whether we use real artificial gradient:
  bool any_artificial_gradient_defined_ = false;
  // (in the next: all hash tables are public for View.paint)
  /* List of all the chemicals with a gaussian distribution along the Z-axis
   * max value, mean (z-coord of the max value), sigma2 (thickness). */
  std::unordered_map<Substance*, std::array<double, 3>, SubstanceHash, SubstanceEqual> gaussian_artificial_concentration_z_;

  /* List of all the chemicals with a linear distribution along the Z-axis
   * max value, TOP (z-coord of the max value), DOWN (z-coord of the 0 value). */
  std::unordered_map<Substance*, std::array<double, 3>, SubstanceHash, SubstanceEqual> linear_artificial_concentration_z_;

  /* List of all the chemicals with a gaussian distribution along the X-axis
   * max value, mean (x-coord of the max value), sigma2 (thickness). */
  std::unordered_map<Substance*, std::array<double, 3>, SubstanceHash, SubstanceEqual> gaussian_artificial_concentration_x_;

  /* List of all the chemicals with a linear distribution along the X-axis
   * max value, TOP (x-coord of the max value), DOWN (x-coord of the 0 value). */
  std::unordered_map<Substance*, std::array<double, 3>, SubstanceHash, SubstanceEqual> linear_artificial_concentration_x_;

  /* to link the one instance of Substance we have used in the definition of the gradient, with the name of
   * the chemical that can be given as argument in the methods to know the concentration/grad.. */
#ifdef __ROOTCLING__
  std::unordered_map<std::string, Substance*> all_artificial_substances_;
#else
  std::unordered_map<std::string, Substance::UPtr> all_artificial_substances_;
#endif

  ECM();

  // *********************************************************************
  // *** Artificial concentration of chemicals
  // *********************************************************************

  /** If as Substance is not already registered, we register it for you. No charges! Order now!*/
  Substance* getRegisteredArtificialSubstance(Substance* substance);

  /** If as Substance is not already registered, we register it for you. No charges! Order now!*/
  Substance* getRegisteredArtificialSubstance(const std::string& substanceId);

  ClassDefOverride(ECM,1);
};

}  // namespace simulation
}  // namespace bdm

#endif  // SIMULATION_ECM_H_
