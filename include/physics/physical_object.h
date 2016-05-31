#ifndef PHYSICS_PHYSICAL_OBJECT_H_
#define PHYSICS_PHYSICAL_OBJECT_H_

#include <string>
#include <list>
#include <array>
#include <vector>
#include <memory>
#include <exception>
#include <unordered_map>

#include "param.h"
#include "color.h"
#include "physics/physical_node.h"
#include "physics/inter_object_force.h"
#include "physics/intracellular_substance.h"
#include "synapse/excrescence.h"
#include "inter_object_force.h"

namespace cx3d {

namespace local_biology {
class CellElement;
}  // namespace local_biology

namespace physics {

class PhysicalBond;
class PhysicalSphere;
class PhysicalCylinder;
class IntracellularSubstance;

using local_biology::CellElement;
using physics::InterObjectForce;
using synapse::Excrescence;

class PhysicalObject : public PhysicalNode {
 public:
  using UPtr = std::unique_ptr<PhysicalObject>;

  /** The class computing the inter object force.*/
  static InterObjectForce* getInterObjectForce();

  /** The class computing the inter object force.*/
  static void setInterObjectForce(InterObjectForce::UPtr force);

  PhysicalObject();

  virtual ~PhysicalObject() {
  }

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  /** Returns true because this object is a PhysicalObject.*/
  virtual bool isAPhysicalObject() const override;

  /**
   * Returns true if this <code>PhysicalObject</code> and the <code>PhysicalObject</code> given as
   * argument have a mother-daughter or sister-sister (e.g. two daughters of a same mother) relation.
   */
  virtual bool isRelative(PhysicalObject* po) const = 0;

  /**
   * Returns the absolute coordinates of the location where a <code>PhysicalObject</code> is attached
   * to this <code>PhysicalObject</code>. Does not necessarily contain a check of the identity of the
   *  element that makes the request.
   *
   * @param daughterWhoAsks the PhysicalObject attached to us.
   * @return the coord
   */
  virtual std::array<double, 3> originOf(PhysicalObject* daughterWhoAsks) = 0;

  /**
   * Removal of a <code>PhysicalObject</code> from the list of our daughters.
   * (Mainly in case of complete retraction of the daughter.*/
  virtual void removeDaughter(PhysicalObject* daughterToRemove) = 0;

  /**
   * Convenient way to change family links in the neuron tree structure
   * (mother, daughter branch). This method is useful during elongation and
   * retraction for intercalation/removal of elements.
   * of elements.
   */
  virtual void updateRelative(PhysicalObject* oldRelative, PhysicalObject* newRelative) = 0;

  /** Returns the <code>CellElement</code>linked to this <code>PhysicalObject</code>.*/
  virtual CellElement* getCellElement() const = 0;

  /** Adds an <code>Excrescence</code> instance to the Excrescence list of this
   * <code>PhysicalObject</code>.*/
  virtual void addExcrescence(Excrescence::UPtr ex);

  /** Removes an <code>Excrescence</code> instance to the Excrescence list of this
   * <code>PhysicalObject</code>.*/
  virtual void removeExcrescence(Excrescence* ex);

  /**
   * Active displacement of the point mass of this <code>PhysicalObject</code>. ("active" means
   * "triggered by a biological process" like in growth or migration). This method MUST NOT be used
   * for displacement by purely passive (physical) force.
   * @param speed in microns/hour
   * @param direction a vector indicating the direction of movement
   */
  virtual void movePointMass(double speed, const std::array<double, 3>& direction) = 0;

  // *************************************************************************************
  // *      METHODS FOR PHYSICS (MECHANICS) COMPUTATION                                              *
  // *************************************************************************************

  /** Compute physical forces, and move accordingly to one simulation time step.*/
  virtual void runPhysics() = 0;

  /**
   * Resets some computational and physical properties (like the tension, volume) after
   * a displacement
   */
  virtual void updateDependentPhysicalVariables() = 0;

  /**
   * Returns the inter-object force that the <code>PhysicalObject</code> in which the method is called applies
   * onto the <code>PhysicalCylinder</code> given as argument.
   * @param c
   * @return
   */
  virtual std::array<double, 4> getForceOn(PhysicalCylinder* c) = 0;

  /**
   * Returns the inter-object force that the <code>PhysicalObject</code> in which the method is called applies
   * onto the <code>PhysicalSphere</code> given as argument.
   * @param s
   * @return
   */
  virtual std::array<double, 3> getForceOn(PhysicalSphere* s) = 0;

  /**
   * Returns true if this <code>PhysicalObject</code> is in contact, i.e. if it is
   * close enough to the <code>PhysicalObject</code> given as argument.
   * @param o
   * @return
   */
  virtual bool isInContact(PhysicalObject* o);

  /**
   * Returns all the neighboring objects considered as being in contact with this PhysicalObject.
   * @return
   */
  virtual std::list<PhysicalObject*> getPhysicalObjectsInContact();  //todo change to vector

  /**
   * Returns the position in the local coordinate system (xAxis, yXis, zAxis)
   * of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
   * @param positionInGlobalCoord
   * @return
   */
  virtual std::array<double, 3> transformCoordinatesGlobalToLocal(
      const std::array<double, 3>& positionInGlobalCoord) const;

  /**
   * Returns the position in in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1])
   * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
   * @param positionInLocalCoord
   * @return
   */
  virtual std::array<double, 3> transformCoordinatesLocalToGlobal(
      const std::array<double, 3>& positionInLocalCoord) const;

  /**
   * Returns the position in the global coordinate system (cartesian coordinates)
   * of a point expressed in polar coordinates (cylindrical or spherical).
   * @param positionInPolarCoordinates a point defined in polar coordinate system of a PhysicalObject
   * @return [x,y,z] the absolute value in space
   */
  virtual std::array<double, 3> transformCoordinatesPolarToGlobal(const std::array<double, 2>& coord) const = 0;

  /**
   * Returns the position in the polar coordinate system (cylindrical or spherical)
   * of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
   * @param positionInAbsoluteCoordinates the [x,y,z] cartesian values
   * @return the position in local coord.
   */
  virtual std::array<double, 3> transformCoordinatesGlobalToPolar(const std::array<double, 3>& coord) const = 0;

  /** Simply adds the argument to the vector containing all the PhysicalBonds of this
   * PhysicalObject.*/
  virtual void addPhysicalBond(const std::shared_ptr<PhysicalBond>& bond);

  /** Simply removes the argument from the vector containing all the PhysicalBonds of this
   * PhysicalObject. */
  virtual void removePhysicalBond(const std::shared_ptr<PhysicalBond>& bond);

  /** Returns true if there is a PhysicalBond that fixes me to this other PhysicalObject.*/
  virtual bool getHasAPhysicalBondWith(PhysicalObject* po);

  /**
   * Creates a new PhysicalBond between this PhysicalObject and the one given as argument.
   * The newly created PhysicalBond is inserted into the physical bon's list of both objects.
   * @param po
   * @return
   */
  virtual std::shared_ptr<PhysicalBond> makePhysicalBondWith(PhysicalObject* po);

  /**
   * If there is a PhysicalBond between this PhysicalObject and po,
   * it is removed (in both objects).
   * @param po the other PhysicalObject we want to test with
   * @param removeThemAll if true, makes multiple removals (if multiple bonds)
   * @return true if at least one PhysicalBond was removed
   */
  virtual bool removePhysicalBondWith(PhysicalObject* po, bool removeThemAll);

  // *************************************************************************************
  // *      METHODS FOR DIFFUSION (INTRA-CELLULAR & MEMBRANE-BOUNDED SUBSTANCES)         *
  // *************************************************************************************

  /**
   * Compute diffusion of <code>IntracellularSubstances</code> with relatives in the neuron
   * tree structure, and perform diffusion processes according to one simulation time step.
   */
  virtual void runIntracellularDiffusion() = 0;

  /**
   * Returns the concentration of an <code>IntracellularSubstance</code> in this
   * PhysicalObject. If not present at all, zhe value 0 is returned.
   * @param substanceId
   * @return
   */
  virtual double getIntracellularConcentration(const std::string& substanceId);

  /** Modifies the quantity (increases or decreases) of an IntracellularSubstance.
   * If this <code>PhysicalNode</code> already has an <code>IntracellularSubstance</code>
   * instance corresponding to the type given as argument (with the same id), the fields
   * quantity and concentration in it will be modified, based on a computation depending
   * on the simulation time step and the space volume (for the latter : only if volumeDependant
   * is true in the IntracellularSubstance). If there is no such IntracellularSubstance
   * instance already, a new instance is requested from ECM.
   * <p>
   * This method is not used for diffusion, but only by biological classes...
   * @param id the name of the Substance to change.
   * @param quantityPerTime the rate of quantity production
   */
  virtual void modifyIntracellularQuantity(const std::string& id, double quantityPerTime);

  /** Returns the concentration of a membrane bound IntracellularSubstance on
   * this PhysicalObject. Recall that by definition, the PhysicalObject are
   * considered as expressing a cell type specific protein as well as the universal
   * marker "U".
   * @param id
   * @return
   */
  virtual double getMembraneConcentration(const std::string& id);

  /** Modifies the quantity (increases or decreases) of an membrane-bound chemical.
   *
   * This method is not used for diffusion, but only by biological classes...
   * @param id the name of the Substance to change.
   * @param quantityPerTime the rate of quantity production
   */

  virtual void modifyMembraneQuantity(const std::string& id, double quantityPerTime);

  /* Returns the INSTANCE of IntracellularSubstance in this PhysicalObject with the same id
   * than the IntracellularSubstance given as argument. If there is no such instance, a
   * new one with similar properties is created, inserted into the intracellularSubstances
   * vector and then returned. Only used between subclasses of physicalObject for intracellular
   * diffusion. C.f. very similar method : PhysicalNode.giveYourSubstanceInstance.
   */
  virtual IntracellularSubstance* giveYouIntracellularSubstanceInstance(IntracellularSubstance* templateS);

  /* Diffusion of diffusible IntracellularSubstances between two PhysicalObjects.
   */
  virtual void diffuseWithThisPhysicalObjects(PhysicalObject* po, double distance);

  // *************************************************************************************
  // *      GETTERS & SETTERS                                                            *
  // *************************************************************************************

  /** Returns the <code>java.awt.Color</code> used to draw this PhysicalObject in the GUI. */
  virtual Color getColor() const;

  /** Sets the <code>java.awt.Color</code> used to draw this PhysicalObject in the GUI. */
  virtual void setColor(Color color);

  /** Returns a copy of the masslocation.*/
  virtual std::array<double, 3> getMassLocation() const;

  /**
   * - CAUTION : Never use this method to move a PhysicalObject, because the physics is not updated.
   * <p>
   * - Never?
   * <p>
   *  - I said NEVER !
   * @param massLocation the massLocation to set
   */
  virtual void setMassLocation(const std::array<double, 3>& massLocation);

  /** Returns the "Up" direction for a Sphere, and the axis direction for a Cylinder*/
  virtual std::array<double, 3> getAxis() const = 0;

  /** Returns the first axis of the local coordinate system.*/
  virtual std::array<double, 3> getXAxis() const;

  /** Sets the first axis of the local coordinate system. Should have a norm of 1.*/
  virtual void setXAxis(const std::array<double, 3>& axis);

  /** Returns the second axis of the local coordinate system.*/
  virtual std::array<double, 3> getYAxis() const;

  /** Sets the second axis of the local coordinate system. Should have a norm of 1*/
  virtual void setYAxis(const std::array<double, 3>& axis);

  /** Returns the third axis of the local coordinate system.*/
  virtual std::array<double, 3> getZAxis() const;

  /** Sets the third axis of the local coordinate system. Should have a norm of 1*/
  virtual void setZAxis(const std::array<double, 3>& axis);

  /** Only for GUI display. Total force on this objects point mass, last time it was computed.
   * 3 first components give the x,y,z coord, and last one if movement was applied (<0 means no).*/
  virtual std::array<double, 4> getTotalForceLastTimeStep() const;

  /** Returns true if this object still plays a role in the simulation.
   * For instance a PhysicalObject associated with a neurite that just retracted
   * is not exxisting. */
  virtual bool isStillExisting() const;

  /** The role of the method is to indicate that an object is about to
   * be garbage Collected. Caution: don't use this method! */
  virtual void setStillExisting(bool stillExists);

  /** If true, this PhysicalObject will be run by the Scheduler on the next occasion.*/
  virtual bool isOnTheSchedulerListForPhysicalObjects() const;

  /** If true, this PhysicalObject will be run by the Scheduler on the next occasion.*/
  virtual void setOnTheSchedulerListForPhysicalObjects(bool onTheSchedulerListForPhysicalObjects);

  /**
   * Returns a unit vector, pointing out of the PhysicalObject if origin at location
   * specified in argument.
   * @param positionInPolarCoordinates the origin of the normal vector (local cartesian coord)
   * @return a vector pointing "out", of unitary norm (absolute cartesian coord)
   */
  virtual std::array<double, 3> getUnitNormalVector(const std::array<double, 3>& positionInPolarCoordinates) const = 0;

  /** Returns the vector containing all the PhysicalBonds of this PhysicalObject.*/
  // todo change to vector if porting has been finished
  virtual std::list<std::shared_ptr<PhysicalBond>> getPhysicalBonds() const;

  /** Sets the vector containing all the PhysicalBonds of this PhysicalObject.
   * This methof should not be used during the simulation. */
  virtual void setPhysicalBonds(const std::list<std::shared_ptr<PhysicalBond> >& physicalBonds);  //todo change to vector

  /** Returns the vector containing all the Excrescences (PhysicalSpine, PhysicalBouton).*/
  virtual std::vector<Excrescence*> getExcrescences() const;

  /** Returns the adherence to the extracellular matrix, i.e. the static friction
   * (the minimum force amplitude needed for triggering a movement). */
  virtual double getAdherence() const;

  /** Sets the adherence to the extracellular matrix, i.e. the static friction
   * (the minimum force amplitude needed for triggering a movement). */
  virtual void setAdherence(double adherence);

  /** Returns the mass, i.e. the kinetic friction
   * (scales the movement amplitude, therefore is considered as the mass).*/
  virtual double getMass() const;

  /** Sets the mass, i.e. the kinetic friction
   * (scales the movement amplitude, therefore is considered as the mass).*/
  virtual void setMass(double mass);

  virtual double getDiameter() const;

  /**
   * Sets the diameter to a new value, and update the volume accordingly.
   * is equivalent to setDiamater(diameter, true)
   * @param diameter
   */
  virtual void setDiameter(double diameter);

  /**
   * Sets the diameter. The volume is sets accordingly if desired.
   *
   * @param diameter the new diameter
   * @param updateVolume if true, the volume is set to match the new diameter.
   */
  virtual void setDiameter(double diameter, bool updateVolume);

  /** Returns the volume of this PhysicalObject.*/
  virtual double getVolume() const;

  /**
   * Sets the volume, and (optionally) recomputes an new diameter.
   * THIS METHOD SHOULD NOT BE USED TO INITIALIZE A PHYSICAL_OBJECT.
   * DEFINE DIMENSIONS, AND THE VOLUME WILL BE COMPUTED.
   * @param volume the new volume
   * @param updateDiameter if true, the diameter will be updated.
   */
  virtual void setVolume(double volume, bool updateDiameter);

  /**
   * Sets the volume, and recomputes an new diameter.
   * THIS METHOD SHOULD NOT BE USED TO INITIALIZE A PHYSICAL_OBJECT.
   * DEFINE DIMENSIONS, AND THE VOLUME WILL BE COMPUTED.
   * @param volume the new volume
   */
  virtual void setVolume(double volume);

  /** Get an intracellular and membrane-bound chemicals that are present
   *  in this PhysicalNode. */
  virtual IntracellularSubstance* getIntracellularSubstance(const std::string& id);

  /** Add an intracellular or membrane-bound chemicals
   *  in this PhysicalNode. */
  virtual void addIntracellularSubstance(IntracellularSubstance::UPtr is);

  /** Remove an intracellular or membrane-bound chemicals that are present
   *  in this PhysicalNode. */
  virtual void removeIntracellularSubstance(IntracellularSubstance* is);

  /** All the intracellular and membrane-bound chemicals that are present
   *  in this PhysicalNode. */
  virtual std::list<IntracellularSubstance*> getIntracellularSubstances1() const;  //todo return map after porting has been finished

  /** Returns the length of a cylinder, or the diameter of a sphere.*/
  virtual double getLength() const = 0;

  virtual void changeDiameter(double speed) = 0;

  virtual void changeVolume(double speed) = 0;

  virtual double getInterObjectForceCoefficient() const = 0;

  virtual void setInterObjectForceCoefficient(double interObjectForceCoefficient) = 0;

 protected:
  /** adding an IntracellularSubstance instance (CAUTION : should not be used for biologic production,
   * and therefore is not a public method. Instead , this method is used for filling up a new
   * PhysicalObject in case of extension).
   */
  virtual void addNewIntracellularSubstance(IntracellularSubstance::UPtr s);

  /**
   * Returns the force that a daughter branch transmits to a mother's point
   * mass. It consists of 1) the spring force between the mother and the
   * daughter point masses and 2) the part of the inter-object mechanical
   * interactions of the daughter branch (with other objects of the
   * simulation) that is transmitted to the proximal end of the daughter
   * (= point-mass of the mother).
   *
   * @param motherWhoAsks the PhysicalObject attached to the mass.
   * @return the force in a std::array<double, 3>
   */
  virtual std::array<double, 3> forceTransmittedFromDaugtherToMother(
      PhysicalObject* motherWhoAsks) = 0;

  /**
   * Returns true if this <code>PhysicalObject</code> and the <code>PhysicalSphere</code> given as
   * argument are close enough to be considered as being in contact.
   * @param s
   * @return
   */
  virtual bool isInContactWithSphere(PhysicalSphere* s) = 0;

  /**
   * Returns true if this <code>PhysicalObject</code> and the <code>PhysicalSphere</code> given as
   * argument are close enough to be considered as being in contact.
   * @param c
   * @return
   */
  virtual bool isInContactWithCylinder(PhysicalCylinder* c) = 0;

  /** Recompute volume after diameter has changed.*/
  virtual void updateVolume() = 0;

  /** Recompute diameter, after volume has been change.*/
  virtual void updateDiameter() = 0;

  /**
   * Updates the concentration of substances, based on the volume of the object.
   * Is usually called after change of the volume (and therefore we don't modify it here)
   */
  virtual void updateIntracellularConcentrations() = 0;

  virtual void setVolumeOnly(double v);

  /** The simulation of Force in this simulation.*/
  static InterObjectForce::UPtr inter_object_force_;

  /** The unique point mass of the object*/
  std::array<double, 3> mass_location_ = std::array<double, 3> { 0.0, 0.0, 0.0 };

  /** First axis of the local coordinate system.*/
  std::array<double, 3> x_axis_ = std::array<double, 3> { 1.0, 0.0, 0.0 };
  /** Second axis of the local coordinate system.*/
  std::array<double, 3> y_axis_ = std::array<double, 3> { 0.0, 1.0, 0.0 };
  /** Third axis of the local coordinate system.*/
  std::array<double, 3> z_axis_ = std::array<double, 3> { 0.0, 0.0, 1.0 };

  /** static friction (the minimum force amplitude for triggering a movement). */
  double adherence_ = 0.1;
  /** kinetic friction (scales the movement amplitude, therefore is called "mass")*/
  double mass_ = 1;
  /** diameter of the object (wheter if sphere or cylinder).*/
  double diameter_ = 1;
  /** volume of this PhysicalObject; updated in updatePhysicalProperties() */
  double volume_ = 1;

  /** Color used when displaying the object*/
  Color color_ = Param::kViolet;

  /** Only for display. Total force on this objects point mass, last time it was computed.
   * 3 first components give the x,y,z coord, and last one if movement was applied (<0 means no)*/
  std::array<double, 4> total_force_last_time_step_ = std::array<double, 4> { 0.0, 0.0, 0.0, -1.0 };

  /** All the internal and membrane-bound (diffusible and non-diffusible)
   *  chemicals that are present inside the PhysicalObject.*/
  std::unordered_map<std::string, IntracellularSubstance::UPtr> intracellular_substances_;

  /** List of the Physical bonds that this object can do (for cell adhesion where synapse formation occurs)*/
  std::list<std::shared_ptr<PhysicalBond> > physical_bonds_;  //todo change to vector once porting has been finished

  /** List of the Physical bonds that this object can do (for cell adhesion, to restore proper configuration)*/
  std::vector<Excrescence::UPtr> excrescences_;

  /**
   * Tells if a PhysicalObject is still part of the simulation.
   * If an object is deleted (either by fusion of two segments, or after retraction),
   * the value becomes false. Needed because of the copy vector (in a
   * random order) of the elements vectors in ECM - to avoid that the run
   * methods are called for elements that were just erased.
   */
  bool still_existing_ = true;

 private:
  PhysicalObject(const PhysicalObject& other) = delete;
  PhysicalObject& operator=(const PhysicalObject& other) = delete;

  /** If true, the PhysicalObject will be run by the Scheduler.
   * Caution : not the same than onTheSchedulerListForPhysicalNodes.*/
  bool on_scheduler_list_for_physical_objects_ = true;
};

}  //namespace physics
}  //namespace cx3d

#endif  // PHYSICS_PHYSICAL_OBJECT_H_
