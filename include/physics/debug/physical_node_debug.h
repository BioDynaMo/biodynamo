#ifndef PHYSICS_DEBUG_PHYSICAL_NODE_DEBUG_H_
#define PHYSICS_DEBUG_PHYSICAL_NODE_DEBUG_H_

#include "string_util.h"
#include "physics/physical_node.h"
#include "physics/substance.h"
#include "spatial_organization/space_node.h"
#include "spatial_organization/spatial_organization_edge.h"

namespace bdm {
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

  virtual bool isAPhysicalObject() const override {
    logCallParameterless();
    auto ret = PhysicalNode::isAPhysicalObject();
    logReturn(ret);
    return ret;
  }

  virtual bool isAPhysicalCylinder() const override {
    logCallParameterless();
    auto ret = PhysicalNode::isAPhysicalCylinder();
    logReturn(ret);
    return ret;
  }

  virtual bool isAPhysicalSphere() const override {
    logCallParameterless();
    auto ret = PhysicalNode::isAPhysicalSphere();
    logReturn(ret);
    return ret;
  }

  virtual double getExtracellularConcentration(const std::string& id) override {
    logCall(id);
    auto ret = PhysicalNode::getExtracellularConcentration(id);
    logReturn(ret);
    return ret;
  }

  virtual double getConvolvedConcentration(const std::string& id) override {
    logCall(id);
    auto ret = PhysicalNode::getConvolvedConcentration(id);
    logReturn(ret);
    return ret;
  }

  virtual double getExtracellularConcentration(const std::string& id, const std::array<double, 3>& location) override {
    logCall(id, location);
    auto ret = PhysicalNode::getExtracellularConcentration(id, location);
    logReturn(ret);
    return ret;
  }

  virtual std::array<double, 3> getExtracellularGradient(const std::string& id) override {
    logCall(id);
    auto ret = PhysicalNode::getExtracellularGradient(id);
    logReturn(ret);
    return ret;
  }

  virtual void modifyExtracellularQuantity(const std::string& id, double quantity_per_time) override {
    logCall(id, quantity_per_time)
    PhysicalNode::modifyExtracellularQuantity(id, quantity_per_time);
    logReturnVoid();
  }

  virtual void runExtracellularDiffusion() override {
    logCallParameterless();
    PhysicalNode::runExtracellularDiffusion();
    logReturnVoid();
  }

  virtual Substance* getSubstanceInstance(Substance* template_s) override {
    logCall(template_s);
    auto ret = PhysicalNode::getSubstanceInstance(template_s);
    logReturn(ret);
    return ret;
  }

  virtual double computeConcentrationAtDistanceBasedOnGradient(Substance* s, const std::array<double, 3>& dX) override {
    logCall(s, dX);
    auto ret = PhysicalNode::computeConcentrationAtDistanceBasedOnGradient(s, dX);
    logReturn(ret);
    return ret;
  }

  virtual std::array<double, 3> soNodePosition() const override {
    logCallParameterless();
    auto ret = PhysicalNode::soNodePosition();
    logReturn(ret);
    return ret;
  }

  virtual SpaceNode<PhysicalNode>* getSoNode() const override {
    logCallParameterless();
    auto ret = PhysicalNode::getSoNode();
    logReturn(ret);
    return ret;
  }

  virtual void setSoNode(typename SpaceNode<PhysicalNode>::UPtr son) override {
    logCall(son.get());
    PhysicalNode::setSoNode(std::move(son));
    logReturnVoid();
  }

  virtual bool isOnTheSchedulerListForPhysicalNodes() const override {
    logCallParameterless();
    auto ret = PhysicalNode::isOnTheSchedulerListForPhysicalNodes();
    logReturn(ret);
    return ret;
  }

  virtual void setOnTheSchedulerListForPhysicalNodes(bool on_the_scheduler_list_for_physical_nodes) override {
    logCall(on_the_scheduler_list_for_physical_nodes);
    PhysicalNode::setOnTheSchedulerListForPhysicalNodes(on_the_scheduler_list_for_physical_nodes);
    logReturnVoid();
  }

  virtual int getMovementConcentratioUpdateProcedure() const override {
    logCallParameterless();
    auto ret = PhysicalNode::getMovementConcentratioUpdateProcedure();
    logReturn(ret);
    return ret;
  }

  virtual void setMovementConcentratioUpdateProcedure(int movement_concentration_update_procedure) override {
    logCall(movement_concentration_update_procedure);
    PhysicalNode::setMovementConcentratioUpdateProcedure(movement_concentration_update_procedure);
    logReturnVoid();
  }

  virtual void addExtracellularSubstance(Substance::UPtr is) override {
    logCall(is.get());
    PhysicalNode::addExtracellularSubstance(std::move(is));
    logReturnVoid();
  }

  virtual void removeExtracellularSubstance(Substance* is) override {
    logCall(is);
    PhysicalNode::removeExtracellularSubstance(is);
    logReturnVoid();
  }

  virtual std::list<Substance*> getExtracellularSubstances() const override {
    logCallParameterless();
    auto ret = PhysicalNode::getExtracellularSubstances();
    logReturn(ret);
    return ret;
  }

  virtual Substance* getExtracellularSubstance(const std::string& key) override {
    logCall(key);
    auto ret = PhysicalNode::getExtracellularSubstance(key);
    logReturn(ret);
    return ret;
  }

  virtual int getID() const override {
    logCallParameterless();
    auto ret = PhysicalNode::getID();
    logReturn(ret);
    return ret;
  }

  virtual void degradate(double currentEcmTime) override {
    logCall(currentEcmTime);
    PhysicalNode::degradate(currentEcmTime);
    logReturnVoid();
  }

//  void diffuseEdgeAnalytically(SpatialOrganizationEdge<PhysicalNode>* e, double currentEcmTime) override  {
//    PhysicalNode::diffuseEdgeAnalytically(e, currentEcmTime);
//    logReturnVoid();
//  }

 private:
  PhysicalNodeDebug(const PhysicalNodeDebug&) = delete;
  PhysicalNodeDebug& operator=(const PhysicalNodeDebug&) = delete;
};

}  // physics
}  // bdm

#endif // PHYSICS_DEBUG_PHYSICAL_NODE_DEBUG_H_
