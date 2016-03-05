#ifndef PHYSICS_DEBUG_PHYSICAL_NODE_DEBUG_H_
#define PHYSICS_DEBUG_PHYSICAL_NODE_DEBUG_H_

#include "string_util.h"
#include "physics/physical_node.h"
#include "physics/substance.h"
#include "spatial_organization/space_node.h"
#include "spatial_organization/spatial_organization_edge.h"

namespace cx3d {
namespace physics {

/**
 * This class is used to generate debug output for the methods that are visible from
 * outside
 */
class PhysicalNodeDebug : public PhysicalNode {
 public:
  PhysicalNodeDebug()
      : PhysicalNode() {
//    logConstrParameterless("PhysicalNode");
  }

  bool isAPhysicalObject() {
    logCallParameterless();
    auto ret = PhysicalNode::isAPhysicalObject();
    logReturn(ret);
    return ret;
  }

  bool isAPhysicalCylinder() {
    logCallParameterless();
    auto ret = PhysicalNode::isAPhysicalCylinder();
    logReturn(ret);
    return ret;
  }

  bool isAPhysicalSphere() {
    logCallParameterless();
    auto ret = PhysicalNode::isAPhysicalSphere();
    logReturn(ret);
    return ret;
  }

  double getExtracellularConcentration(const std::string& id) {
    logCall(id);
    auto ret = PhysicalNode::getExtracellularConcentration(id);
    logReturn(ret);
    return ret;
  }

  double getConvolvedConcentration(const std::string& id) {
    logCall(id);
    auto ret = PhysicalNode::getConvolvedConcentration(id);
    logReturn(ret);
    return ret;
  }

  double getExtracellularConcentration(const std::string& id, const std::array<double, 3>& location) {
    logCall(id, location);
    auto ret = PhysicalNode::getExtracellularConcentration(id, location);
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> getExtracellularGradient(const std::string& id) {
    logCall(id);
    auto ret = PhysicalNode::getExtracellularGradient(id);
    logReturn(ret);
    return ret;
  }

  void modifyExtracellularQuantity(const std::string& id, double quantity_per_time) {
    logCall(id, quantity_per_time)
    PhysicalNode::modifyExtracellularQuantity(id, quantity_per_time);
    logReturnVoid();
  }

  void runExtracellularDiffusion() {
    logCallParameterless();
    PhysicalNode::runExtracellularDiffusion();
    logReturnVoid();
  }

  std::shared_ptr<Substance> getSubstanceInstance(const std::shared_ptr<Substance>& template_s) {
    logCall(template_s);
    auto ret = PhysicalNode::getSubstanceInstance(template_s);
    logReturn(ret);
    return ret;
  }

  double computeConcentrationAtDistanceBasedOnGradient(const std::shared_ptr<Substance>& s,
      std::array<double, 3>& dX) {
    logCall(s, dX);
    auto ret = PhysicalNode::computeConcentrationAtDistanceBasedOnGradient(s, dX);
    logReturn(ret);
    return ret;
  }

  std::array<double, 3> soNodePosition() {
    logCallParameterless();
    auto ret = PhysicalNode::soNodePosition();
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<spatial_organization::SpaceNode<PhysicalNode>> getSoNode() {
    logCallParameterless();
    auto ret = PhysicalNode::getSoNode();
    logReturn(ret);
    return ret;
  }

  void setSoNode(const std::shared_ptr<spatial_organization::SpaceNode<PhysicalNode>>& son) {
    logCall(son);
    PhysicalNode::setSoNode(son);
    logReturnVoid();
  }

  bool isOnTheSchedulerListForPhysicalNodes() {
    logCallParameterless();
    auto ret = PhysicalNode::isOnTheSchedulerListForPhysicalNodes();
    logReturn(ret);
    return ret;
  }

  void setOnTheSchedulerListForPhysicalNodes(bool on_the_scheduler_list_for_physical_nodes) {
    logCall(on_the_scheduler_list_for_physical_nodes);
    PhysicalNode::setOnTheSchedulerListForPhysicalNodes(on_the_scheduler_list_for_physical_nodes);
    logReturnVoid();
  }

  int getMovementConcentratioUpdateProcedure() {
    logCallParameterless();
    auto ret = PhysicalNode::getMovementConcentratioUpdateProcedure();
    logReturn(ret);
    return ret;
  }

  void setMovementConcentratioUpdateProcedure(int movement_concentration_update_procedure) {
    logCall(movement_concentration_update_procedure);
    PhysicalNode::setMovementConcentratioUpdateProcedure(movement_concentration_update_procedure);
    logReturnVoid();
  }

  void addExtracellularSubstance(const std::shared_ptr<Substance>& is) {
    logCall(is);
    PhysicalNode::addExtracellularSubstance(is);
    logReturnVoid();
  }

  void removeExtracellularSubstance(const std::shared_ptr<Substance>& is) {
    logCall(is);
    PhysicalNode::removeExtracellularSubstance(is);
    logReturnVoid();
  }

  std::list<std::shared_ptr<Substance>> getExtracellularSubstances() {
    logCallParameterless();
    auto ret = PhysicalNode::getExtracellularSubstances();
    logReturn(ret);
    return ret;
  }

  std::shared_ptr<Substance> getExtracellularSubstance(const std::string& key) {
    logCall(key);
    auto ret = PhysicalNode::getExtracellularSubstance(key);
    logReturn(ret);
    return ret;
  }

  int getID() {
    logCallParameterless();
    auto ret = PhysicalNode::getID();
    logReturn(ret);
    return ret;
  }

  void degradate(double currentEcmTime) {
    logCall(currentEcmTime);
    PhysicalNode::degradate(currentEcmTime);
    logReturnVoid();
  }

//  void diffuseEdgeAnalytically(
//      const std::shared_ptr<spatial_organization::SpatialOrganizationEdge<PhysicalNode>>& e, double currentEcmTime) {
//    PhysicalNode::diffuseEdgeAnalytically(e, currentEcmTime);
//    logReturnVoid();
//  }

private:
  PhysicalNodeDebug(const PhysicalNodeDebug&) = delete;
  PhysicalNodeDebug& operator=(const PhysicalNodeDebug&) = delete;
};

}  // physics
}  // cx3d

#endif // PHYSICS_DEBUG_PHYSICAL_NODE_DEBUG_H_
