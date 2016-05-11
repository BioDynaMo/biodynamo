#ifndef SIMULATION_ECM_H_
#define SIMULATION_ECM_H_

#include <vector>
#include <array>
#include <string>
#include <memory>
#include <exception>
#include <list>
#include <unordered_map>

#include "java_util.h"
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
#include "physics/physical_node_movement_listener.h"
#include "physics/substance.h"
#include "spatial_organization/space_node.h"

namespace cx3d {

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
using physics::SubstanceHash;
using physics::SubstanceEqual;

/**
 * Contains some lists with all the elements of the simulation, and methods to add
 * or remove elements. Contains lists of "real" and "artificial" Substances.
 * Possibility to define a cubic region of space where elements are confined.
 */
class ECM : public SimStateSerializable {
 public:
  static std::shared_ptr<ECM> getInstance();
  static void setJavaUtil(const std::shared_ptr<cx3d::JavaUtil2>& java) {
    java_ = java;
  }

  ECM();  // todo make private after porting has been finished
  virtual ~ECM();
  ECM(const ECM&) = delete;
  ECM& operator=(const ECM&) = delete;

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

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
  virtual bool getArtificialWallForSpheres() const;

  /** If set to true, the PhysicalCyliners tend to stay inside a box,
   * who's boundaries are set with setBoundaries().
   * @param artificialWallsForCylinders
   */
  void setArtificialWallsForCylinders(bool artificial_walls);

  /** If true, the PhysicalCyliners tend to stay inside a box, who's boundaries are set with
   * setBoundaries().
   */
  virtual bool getArtificialWallForCylinders() const;

  /**
   * Returns a force that would be applied to a PhysicalSphere that left the boundaries
   * of the artificial wall.
   * @param location the center of the PhysicalSphere
   * @param radius the radius of the PhysicalSphere
   * @return [Fx,Fy,Fz] the force applied to the cell
   */
  virtual std::array<double, 3> forceFromArtificialWall(const std::array<double, 3>& location, double radius);

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
  virtual std::shared_ptr<cx3d::spatial_organization::SpaceNode<physics::PhysicalNode> > getSpatialOrganizationNodeInstance(
      const std::array<double, 3>& position, const std::shared_ptr<physics::PhysicalNode>& user_object);

  /**
   * Returns an instance of a class implementing SpatialOrganizationNode.
   * If it is the first node of the simulation, it will fix the Class type of the SpatialOrganizationNode.
   * CAUTION : NEVER call this method if there exist already SpatialOrganizationNodes in
   * the simulation, and initialNode in ECM has not been instantiated : there will then be
   * two different unconnected Delaunay
   * @param n an already existing SpaceNode close to the place where the new one should be
   * @param position
   * @param userObject
   * @return
   */
  std::shared_ptr<cx3d::spatial_organization::SpaceNode<physics::PhysicalNode> > getSpatialOrganizationNodeInstance(
      const std::shared_ptr<spatial_organization::SpaceNode<physics::PhysicalNode> >& n,
      const std::array<double, 3>& position, const std::shared_ptr<physics::PhysicalNode>& user_object);

  /**
   * Adding some "dummy nodes", i.e. some  PhysicalSplace.
   * @param x1 lower boundary on the X axis
   * @param x2 upper boundary on the X axis
   * @param y1 upper boundary on the Y axis
   * @param y2 lower boundary on the Y axis
   * @param z1 upper boundary on the Z axis
   * @param z2 lower boundary on the Z axis
   * @param d inter-node distance
   */
  void addGridOfPhysicalNodes(double x1, double x2, double y1, double y2, double z1, double z2, double d);

  /** Adds a "simple" PhysicalNode (with its SON) at a desired location.*/
  std::shared_ptr<physics::PhysicalNode> getPhysicalNodeInstance(const std::array<double, 3>& position);

  // Physical Objects-------------------------------------------
  // PhysicalCylinder and PhysicalSphere are also instances of PhysicalNode.
  // PhysicalNode contains a SpatialOrganizerNode.
  // So: add/remove PhysicalCylinder/PhysicalSphere makes a call to
  // add/remove-PhysicalNode.
  // the later also calls the remove() method of the associated SpatialOrganizationNode.

  virtual void addPhysicalCylinder(const std::shared_ptr<physics::PhysicalCylinder>& cyl);

  virtual void removePhysicalCylinder(const std::shared_ptr<physics::PhysicalCylinder>& cyl);

  virtual void addPhysicalSphere(const std::shared_ptr<physics::PhysicalSphere>& sphere);

  virtual void removePhysicalSphere(const std::shared_ptr<physics::PhysicalSphere>& sphere);

  virtual void addPhysicalNode(const std::shared_ptr<physics::PhysicalNode>& node);

  virtual void removePhysicalNode(const std::shared_ptr<physics::PhysicalNode>& node);

  //fixme implement
//     void addECMChemicalReaction(ECMChemicalReaction chemicalReaction);
//
//     void removeECMChemicalReaction(ECMChemicalReaction chemicalReaction);

  //  Cells

  virtual void addCell(Cell::UPtr cell);

  virtual void removeCell(Cell* cell);

  // Cell Elements--------------------------------------------------
  virtual void addSomaElement(const std::shared_ptr<local_biology::SomaElement>& soma);

  virtual void removeSomaElement(const std::shared_ptr<local_biology::SomaElement>& soma);

  virtual void addNeuriteElement(const std::shared_ptr<local_biology::NeuriteElement>& neurite);

  virtual void removeNeuriteElement(const std::shared_ptr<local_biology::NeuriteElement>& neurite);

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
  void addNewSubstanceTemplate(const std::shared_ptr<physics::Substance>& s);

  /** Define a template for each <code>IntracellularSubstance</code> in the simulation that has
   * non-standard values for diffusion and degradation constant, and outside visibility and volume dependency.*/
  void addNewIntracellularSubstanceTemplate(const std::shared_ptr<physics::IntracellularSubstance>& s);

  /** Returns an instance of <code>Substance</code>. If a similar substance (with the same id)
   * has already been declared as a template Substance, a copy of it is made (with
   * same id, degradation and diffusion constant, but concentration and quantity 0).
   * If it is the first time that this id is requested, a new template Substance is made
   * (with by-default values) and stored, and a copy will be returned.
   * @param id
   * @return new Substance instance
   */
  virtual std::shared_ptr<physics::Substance> substanceInstance(const std::string& id);

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
  virtual std::shared_ptr<physics::IntracellularSubstance> intracellularSubstanceInstance(const std::string& id);

  // *********************************************************************
  // *** Pre-defined cellType colors
  // *********************************************************************
  void addNewCellTypeColor(const std::string& cell_type, Color color);

  virtual Color cellTypeColor(const std::string& cell_type);

  // *********************************************************************
  // *** Artificial concentration of chemicals
  // *********************************************************************

  /** Returns true if some artificial gradient (of any sorts) have been defined.*/
  virtual bool thereAreArtificialGradients() const;

  /**
   * Defines a bell-shaped artificial concentration in ECM, along the Z axis (ie uniform along X,Y axis).
   * It is a continuous value, and not instances of the class Substance!.
   *
   * @param substance
   * @param max_concentration the value of the concentration at its peak
   * @param z_coord the location of the peak
   * @param sigma the thickness of the layer (= the variance)
   */
  void addArtificialGaussianConcentrationZ(const std::shared_ptr<physics::Substance>& substance,
                                           double max_concentration, double z_coord, double sigma);

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
  void addArtificialLinearConcentrationZ(const std::shared_ptr<physics::Substance>& substance,
                                         double max_concentatration, double z_coord_max, double z_coord_min);

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
  void addArtificialGaussianConcentrationX(const std::shared_ptr<physics::Substance>& substance,
                                           double max_concentatration, double x_coord, double sigma);

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
  void addArtificialLinearConcentrationX(const std::shared_ptr<physics::Substance>& substance,
                                         double max_concentatration, double x_coord_max, double x_coord_min);

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
  virtual double getValueArtificialConcentration(const std::string& nameOfTheChemical,
                                                 const std::array<double, 3>& position) const;

  /**
   * Gets the value of a chemical, at a specific position in space
   * @param nameOfTheChemical
   * @param position the location [x,y,z]
   * @return
   */
  double getValueArtificialConcentration(const std::shared_ptr<physics::Substance>& substance,
                                         const std::array<double, 3>& position) const;
  ///////// GRADIENT
  /**
   * Gets the gradient of a chemical, at a specific altitude
   * @param substance_name
   * @param position the location [x,y,z]
   * @return the gradient [dc/dx , dc/dy , dc/dz]
   */
  virtual std::array<double, 3> getGradientArtificialConcentration(const std::string& substance_name,
                                                                   const std::array<double, 3>& position) const;

  double getGradientArtificialConcentration(const std::shared_ptr<physics::Substance>& s,
                                            const std::array<double, 3>& position) const;

  std::vector<std::shared_ptr<physics::PhysicalNode>> getPhysicalNodeList() const;

  std::vector<std::shared_ptr<physics::PhysicalSphere>> getPhysicalSphereList() const;

  std::vector<std::shared_ptr<physics::PhysicalCylinder>> getPhysicalCylinderList() const;

  virtual std::list<std::shared_ptr<local_biology::NeuriteElement>> getNeuriteElementList() const;

  std::vector<std::shared_ptr<local_biology::SomaElement>> getSomaElementList() const;

  bool isAnyArtificialGradientDefined() const;

  std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> getGaussianArtificialConcentrationZ() const;

  std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> getLinearArtificialConcentrationZ() const;

  std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> getGaussianArtificialConcentrationX() const;

  std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> getLinearArtificialConcentrationX() const;

  virtual double getECMtime() const;

  void setECMtime(double ECMtime);

  virtual void increaseECMtime(double deltaT);

  std::unordered_map<std::string, std::shared_ptr<physics::IntracellularSubstance>> getIntracelularSubstanceTemplates() const;

  std::unordered_map<std::string, std::shared_ptr<physics::Substance>> getSubstanceTemplates() const;

  std::array<double, 3> getMinBounds() const;

  std::array<double, 3> getMaxBounds() const;

  //
  //
  //

  virtual std::shared_ptr<physics::PhysicalCylinder> getPhysicalCylinder(int i) const {
    return physical_cylinders_[i];
  }

  virtual std::shared_ptr<local_biology::NeuriteElement> getNeuriteElement(int i) const {
    return neurite_elements_[i];
  }

  virtual std::shared_ptr<physics::PhysicalNode> getPhysicalNode(int i) const {
    return physical_nodes_[i];
  }

  virtual std::shared_ptr<physics::PhysicalSphere> getPhysicalSphere(int i) const {
    return physical_spheres_[i];
  }

  virtual std::shared_ptr<local_biology::SomaElement> getSomaElement(int i) const {
    return soma_elements_[i];
  }

  virtual Cell* getCell(int i) const {
    return cells_[i].get();
  }

  virtual int getCellListSize() const {
    return cells_.size();
  }

  virtual int getPhysicalNodeListSize() const {
    return physical_nodes_.size();
  }

  virtual int getPhysicalCylinderListSize() const {
    return physical_cylinders_.size();
  }

  virtual int getPhysicalSphereListSize() const {
    return physical_spheres_.size();
  }

  virtual int getSomaElementListSize() const {
    return soma_elements_.size();
  }

  virtual int getNeuriteElementListSize() const {
    return neurite_elements_.size();
  }

  //
  //
  //

  double getRandomDouble1() const {
    return java_->getRandomDouble1();
  }

  double matrixNextRandomDouble() const {
    return java_->matrixNextRandomDouble();
  }

  double exp(double d) const {
    return java_->exp(d);
  }

  double cbrt(double d) const {
    return java_->cbrt(d);
  }

  double sqrt(double d) const {
    return java_->sqrt(d);
  }

  double cos(double d) const {
    return java_->cos(d);
  }

  double sin(double d) const {
    return java_->sin(d);
  }

  double asin(double d) const {
    return java_->asin(d);
  }

  double acos(double d) const {
    return java_->acos(d);
  }

  double atan2(double d, double d1) const {
    return java_->atan2(d, d1);
  }

  std::shared_ptr<physics::PhysicalCylinder> newPhysicalCylinder() const {
    return java_->newPhysicalCylinder();
  }

  std::array<double, 3> matrixRandomNoise3(double k) const {
    return java_->matrixRandomNoise3(k);
  }

  std::shared_ptr<physics::PhysicalSphere> newPhysicalSphere() const {
    return java_->newPhysicalSphere();
  }

  std::shared_ptr<local_biology::NeuriteElement> newNeuriteElement() const {
    return java_->newNeuriteElement();
  }

  std::shared_ptr<local_biology::SomaElement> newSomaElement() const {
    return java_->newSomaElement();
  }

  std::shared_ptr<cx3d::synapse::PhysicalSpine> newPhysicalSpine(const std::shared_ptr<physics::PhysicalObject>& po,
                                                                 const std::array<double, 2>& origin,
                                                                 double length) const {
    return java_->newPhysicalSpine(po, origin, length);
  }

  std::shared_ptr<synapse::PhysicalBouton> newPhysicalBouton(const std::shared_ptr<physics::PhysicalObject>& po,
                                                             const std::array<double, 2>& origin, double length) const {
    return java_->newPhysicalBouton(po, origin, length);
  }

  std::shared_ptr<physics::PhysicalBond> newPhysicalBond(const std::shared_ptr<physics::PhysicalObject>& a,
                                                         const std::array<double, 2>& position_on_a,
                                                         const std::shared_ptr<physics::PhysicalObject>& b,
                                                         const std::array<double, 2>& position_on_b,
                                                         double resting_length, double spring_constant) const {
    return java_->newPhysicalBond(a, position_on_a, b, position_on_b, resting_length, spring_constant);
  }

  double getGaussianDouble(double mean, double standard_deviation) const {
    return java_->getGaussianDouble(mean, standard_deviation);
  }

 private:
  static std::shared_ptr<JavaUtil2> java_;

  // List of all the CX3DRunnable objects in the simulation ............................

  /** List of all the PhysicalNode instances. */
  std::vector<std::shared_ptr<physics::PhysicalNode>> physical_nodes_;
  /** List of all the PhysicalSphere instances. */
  std::vector<std::shared_ptr<physics::PhysicalSphere>> physical_spheres_;
  /** List of all the PhysicalCylinder instances. */
  std::vector<std::shared_ptr<physics::PhysicalCylinder>> physical_cylinders_;
  /** List of all the SomaElement instances. */
  std::vector<std::shared_ptr<local_biology::SomaElement>> soma_elements_;
  /** List of all the NeuriteElement instances. */
  std::vector<std::shared_ptr<local_biology::NeuriteElement>> neurite_elements_;
  /** List of all the Cell instances. */
  std::vector<Cell::UPtr> cells_;
  /** List of all the Chemical reactions instances. */
  //fixme implement std::vector<std::shared_ptr<physics::ECMChemicalReaction>> ecmChemicalReactionList;
  /* Reference time throughout the simulation (in hours) */
  double time_ = 0;

  /* An SON used to get new SON instances from*/
  std::shared_ptr<spatial_organization::SpaceNode<physics::PhysicalNode>> initial_node_;

  /* In here we keep a template for each (extra-cellular) Substance in the simulation that have
   * non-standard value for diffusion and degradation constant.*/
  std::unordered_map<std::string, std::shared_ptr<physics::Substance>> substance_lib_;

  /* In here we keep a template for each (intra-cellular) Substance in the simulation that have
   * non-standard value for diffusion and degradation constant.*/
  std::unordered_map<std::string, std::shared_ptr<physics::IntracellularSubstance>> intracellular_substance_lib_;

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
  std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> gaussian_artificial_concentration_z_;

  /* List of all the chemicals with a linear distribution along the Z-axis
   * max value, TOP (z-coord of the max value), DOWN (z-coord of the 0 value). */
  std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> linear_artificial_concentration_z_;

  /* List of all the chemicals with a gaussian distribution along the X-axis
   * max value, mean (x-coord of the max value), sigma2 (thickness). */
  std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> gaussian_artificial_concentration_x_;

  /* List of all the chemicals with a linear distribution along the X-axis
   * max value, TOP (x-coord of the max value), DOWN (x-coord of the 0 value). */
  std::unordered_map<std::shared_ptr<physics::Substance>, std::array<double, 3>, SubstanceHash, SubstanceEqual> linear_artificial_concentration_x_;

  /* to link the one instance of Substance we have used in the definition of the gradient, with the name of
   * the chemical that can be given as argument in the methods to know the concentration/grad.. */
  std::unordered_map<std::string, std::shared_ptr<physics::Substance>> all_artificial_substances_;

//  ECM();

  // *********************************************************************
  // *** Artificial concentration of chemicals
  // *********************************************************************

  /** If as Substance is not already registered, we register it for you. No charges! Order now!*/
  std::shared_ptr<physics::Substance> getRegisteredArtificialSubstance(
      const std::shared_ptr<physics::Substance>& substance);

  /** If as Substance is not already registered, we register it for you. No charges! Order now!*/
  std::shared_ptr<physics::Substance> getRegisteredArtificialSubstance(const std::string& substanceId);
};

}  // namespace simulation
}  // namespace cx3d

#endif  // SIMULATION_ECM_H_
