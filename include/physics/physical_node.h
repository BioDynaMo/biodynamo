#ifndef PHYSICS_PHYSICAL_NODE_H_
#define PHYSICS_PHYSICAL_NODE_H_

#include <array>
#include <list>
#include <string>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "sim_state_serializable.h"
#include "physics/substance.h"
#include "spatial_organization/space_node.h"

namespace cx3d {

namespace spatial_organization {
template<class T> class SpatialOrganizationEdge;
}  // namespace spatial_organization

namespace simulation {
class ECM;
// todo replace with include once porting has been finished and remove include in cc file
}// namespace simulation

namespace physics {

using simulation::ECM;
using spatial_organization::SpaceNode;
using spatial_organization::SpatialOrganizationEdge;

/**
 * PhysicalNode represents a piece of the simulation space, whether it contains a physical object of not.
 * As such, it contains a list of all the chemicals (<code>Substance</code>) that are present at this place,
 * as well as the methods for the diffusion. In order to be able to diffuse chemicals, it contains a node
 * that is part of the neighboring system (eg triangulation). A <code>PhysicalNode</code> can only diffuse
 * to and receive from the neighbouring <code>PhysicalNode</code>s.
 * <p>
 * The <code>PhysiacalObject</code> sub-classes (<code>PhysicalSphere</code>, <code>PhysicalCylinder</code>)
 * inherit from this class. This emphasize the fact that they take part in the definition of space and
 * that diffusion of chemical occurs across them.
 * <p>
 * As all the CX3D runnable objects, the PhysicalNode updates its state (i.e. diffuses and degradates) only
 * if needed. The private field <code>onTheSchedulerListForPhysicalNodes</code> is set to <code>true</code>
 * in this case. (For degradation, there is an update mechanism, catching up from the last time it was performed).
 */
class PhysicalNode : public SimStateSerializable, public std::enable_shared_from_this<PhysicalNode> {
 public:
  using UPtr = std::unique_ptr<PhysicalNode>;
  // *************************************************************************************
  // *      METHODS FOR INTERPOLATION (used only by PhysicalNodeMovementListener)        *
  // *************************************************************************************

  static void reset();

  /**
   * Finding the barycentric coordinates of a point Q with respect to the the four points P
   * (based on http://www.devmaster.net/wiki/Barycentric_coordinates).
   * @param Q
   * @param P1
   * @param P2
   * @param P3
   * @param P4
   * @return array with the coordinate of point Q with respect Pi
   */
  static std::array<double, 4> getBarycentricCoordinates(const std::array<double, 3>& Q,
                                                         const std::array<double, 3>& P1,
                                                         const std::array<double, 3>& P2,
                                                         const std::array<double, 3>& P3,
                                                         const std::array<double, 3>& P4);

  /**
   * Finding the barycentric coordinates of a point Q with respect to the the four vertices.
   * @param Q
   * @param vertices
   * @return
   */
  static std::array<double, 4> getBarycentricCoordinates(const std::array<double, 3>& Q,
                                                         const std::array<PhysicalNode*, 4>& vertices);

  PhysicalNode();

  virtual ~PhysicalNode();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  virtual std::string toString() const;

  /** Returns true if this PhysicalNode is a PhysicalObject.*/
  virtual bool isAPhysicalObject() const;

  /** Returns true if this PhysicalNode is a PhysicalCylinder.*/
  virtual bool isAPhysicalCylinder() const;

  /** Returns true if this PhysicalNode is a PhysicalSphere.*/
  virtual bool isAPhysicalSphere() const;

  // *************************************************************************************
  // *        INTERACTION WITH PHYSICAL_OBJECTS (secretion, reading concentration etc.)  *
  // *************************************************************************************

  /**
   * Returns the concentration of an extra-cellular Substance at this PhysicalNode.
   * @param id the name of the substance
   * @return the concentration
   */
  virtual double getExtracellularConcentration(const std::string& id);

  /**
   * Returns the average concentration of for a PhysicalNode and all its neighbors,
   * weighted by physical nodes volumes. Diminishes local fluctuations due to irregular
   * triangulation, but is more expensive to compute.
   * @param id the name of the substance
   * @return the concentration
   */
  virtual double getConvolvedConcentration(const std::string& id);

  /**
   * Returns the concentration of an extra-cellular Substance outside this PhysicalNode.
   * @param id the name of the substance
   * @param location the place where concentration is probed
   * @return the concentration
   */
  virtual double getExtracellularConcentration(const std::string& id, const std::array<double, 3>& location);

  /**
   * Returns the gradient at the space node location for a given substance.
   * The way this method is implemented was suggested by Andreas Steimer.
   * @param id the name of the Substance we have to compute the gradient of.
   * @return [dc/dx, dc/dy, dc/dz]
   */
  virtual std::array<double, 3> getExtracellularGradient(const std::string& id);

  /** Modifies the quantity (increases or decreases) of an extra-cellular Substance.
   * If this <code>PhysicalNode</code> already has an <code>Substance</code> instance
   * corresponding to the type given as argument (with the same id), the fields
   * quantity and concentration in it will be modified, based on a computation depending
   * on the simulation time step and the space volume. If there is no such Substance
   * instance already, a new instance is requested from ECM.
   * <p>
   * This method is not used for diffusion, but only by biological classes...
   * @param id the name of the Substance to change.
   * @param quantityPerTime the rate of quantity production
   */
  virtual void modifyExtracellularQuantity(const std::string& id, double quantity_per_time);

  // *************************************************************************************
  // *                            RUN (diffusion, degradation)                           *
  // *************************************************************************************

  /**
   * Degradate (according to degrad. constant) and diffuse 8according to diff. constant)
   * all the <code>Substance</code> stored in this <code>PhysicalNode</code>.
   */
  virtual void runExtracellularDiffusion();

  /** Returns the INSTANCE of Substance stored in this node, with the same id
   * than the Substance given as argument. If there is no such Instance, a new one
   * is created, inserted into the list extracellularSubstances and returned.
   * Used for diffusion and for ECMChemicalReaction.*/
  virtual Substance* getSubstanceInstance(Substance* template_s);

  /* Interpolation of the concentration value at a certain distance. Used to update
   * the chemicals in case of a displacement or for the creation of new nodes, in
   * PhysicalNodeMovementListener, in case barycentric interpolation is not possible.
   * @param s the substance
   * @param dX the distance from this nodes's location
   * @return
   */
  virtual double computeConcentrationAtDistanceBasedOnGradient(Substance* s, const std::array<double, 3>& dX);

  // *************************************************************************************
  // *      GETTERS & SETTERS                                                            *
  // *************************************************************************************

  /** Returns the position of the <code>SpatialOrganizationNode</code>.
   * Equivalent to getSoNode().getPosition(). */
  // not a real getter...
  virtual std::array<double, 3> soNodePosition() const;

  /**
   * Sets the SpatialOrganizationNode (vertex in the triangulation neighboring system).
   */
  virtual SpaceNode<PhysicalNode>* getSoNode() const;  //todo change to SpatialOrganizationNode after porting has been finished

  /** Returns the SpatialOrganizationNode (vertex in the triangulation neighboring system).*/
  virtual void setSoNode(typename SpaceNode<PhysicalNode>::UPtr son);  //todo change to SpatialOrganizationNode after porting has been finished

  /** if <code>true</code>, the PhysicalNode will be run by the Scheduler.**/
  virtual bool isOnTheSchedulerListForPhysicalNodes() const;

  /** if <code>true</code>, the PhysicalNode will be run by the Scheduler.**/
  virtual void setOnTheSchedulerListForPhysicalNodes(bool on_the_scheduler_list_for_physical_nodes);

  /** Solely used by the PhysicalNodeMovementListener to update substance concentrations.**/
  virtual int getMovementConcentratioUpdateProcedure() const;

  /** Solely used by the PhysicalNodeMovementListener to update substance concentrations.**/
  virtual void setMovementConcentratioUpdateProcedure(int movement_concentration_update_procedure);

  /** Add an extracellular or membrane-bound chemicals
   *  in this PhysicalNode. */
  virtual void addExtracellularSubstance(Substance::UPtr is);

  /** Remove an extracellular or membrane-bound chemicals that are present
   *  in this PhysicalNode. */
  virtual void removeExtracellularSubstance(Substance* is);

  /** All the (diffusible) chemicals that are present in the space defined by this physicalNode. */
  virtual std::list<Substance*> getExtracellularSubstances() const;  //todo refactor - originally returned the whole map

  virtual Substance* getExtracellularSubstance(const std::string& key);  //todo refactor - added to avoid implementing unorederd_map for swig

  /** Returns a unique ID for this PhysicalNode.*/
  virtual int getID() const;

  /**
   * Runs the degradation of all Substances (only if it is not up-to-date). This method
   * is called before each operation on Substances (
   * @param currentEcmTime the current time of the caller
   * (so that it doesn't require a call to ECM).
   */
  virtual void degradate(double currentEcmTime);

 protected:
  /* Reference to the ECM. */
  static ECM* ecm_;

  /**
   *  My anchor point in the neighboring system
   */
  typename SpaceNode<PhysicalNode>::UPtr so_node_;

 private:
  static std::size_t id_counter_;

  /**
   *  Unique identification for this CellElement instance. Used for marshalling/demarshalling
   */
  std::size_t id_ = 0;

  /**
   *  If true, the PhysicalNode will be run by the Scheduler.
   */
  bool on_scheduler_list_for_physical_nodes_;

  /**
   * Stores the time at which degradation was performed last (important for later catch up).
   */
  double last_ecm_time_degradate_was_run_;

  /**
   * Used by the PhysicalNodeMovementListener to update substance concentrations.
   */
  int movement_concentration_update_procedure_ = -999;

  /**
   * All the (diffusible) chemicals that are present in the space defined by this physicalNode.
   */
  std::unordered_map<std::string, Substance::UPtr> extracellular_substances_;

  /* Analytic solution of the diffusion process along the edge between two PhysicalNodes.
   * dQA/dt = diffCst*(Area/distance)*(QB/VB-QA/VA)
   */
  virtual void diffuseEdgeAnalytically(SpatialOrganizationEdge<PhysicalNode>* e, double current_ecm_time);
};

}  // namespace physics
}  // namespace cx3d

#endif  // PHYSICS_PHYSICAL_NODE_H_
