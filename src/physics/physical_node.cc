#include "physics/physical_node.h"

#include <sstream>

#include "matrix.h"
#include "param.h"
#include "stl_util.h"
#include "string_util.h"
#include "sim_state_serialization_util.h"

#include "physics/substance.h"

#include "spatial_organization/spatial_organization_node.h"
#include "spatial_organization/edge.h"

#include "simulation/ecm.h"

namespace cx3d {
namespace physics {

std::size_t PhysicalNode::id_counter_ = 0;
ECM* PhysicalNode::ecm_ = ECM::getInstance();

void PhysicalNode::reset() {
  id_counter_ = 0;
}

std::array<double, 4> PhysicalNode::getBarycentricCoordinates(const std::array<double, 3>& Q,
                                                              const std::array<double, 3>& P1,
                                                              const std::array<double, 3>& P2,
                                                              const std::array<double, 3>& P3,
                                                              const std::array<double, 3>& P4) {
  // three linearly independent vectors
  auto B1 = Matrix::subtract(P2, P1);
  auto B2 = Matrix::subtract(P3, P1);
  auto B3 = Matrix::subtract(P4, P1);
  // finding how to express (Q-P1) with these three vectors : gives the 2nd, 3rd and 4th coordinate
  std::array<std::array<double, 3>, 3> A;
  A[0] = {B1[0], B2[0], B3[0]};
  A[1] = {B1[1], B2[1], B3[1]};
  A[2] = {B1[2], B2[2], B3[2]};
  auto barycentric_coord = Matrix::solve(A, Matrix::subtract(Q, P1));
  // to find the first component : the total = 1
  double first_coord = 1 - (barycentric_coord[0] + barycentric_coord[1] + barycentric_coord[2]);

  return std::array<double, 4>( { first_coord, barycentric_coord[0], barycentric_coord[1], barycentric_coord[2] });
}

std::array<double, 4> PhysicalNode::getBarycentricCoordinates(
    const std::array<double, 3>& Q, const std::array<PhysicalNode*, 4>& vertices) {
  auto a = vertices[0]->getSoNode()->getPosition();
  auto b = vertices[1]->getSoNode()->getPosition();
  auto c = vertices[2]->getSoNode()->getPosition();
  auto d = vertices[3]->getSoNode()->getPosition();
  return PhysicalNode::getBarycentricCoordinates(Q, a, b, c, d);
}

PhysicalNode::PhysicalNode()
    : id_ { ++PhysicalNode::id_counter_ },
      on_scheduler_list_for_physical_nodes_ { true },
      last_ecm_time_degradate_was_run_ { 0 },
      movement_concentration_update_procedure_ { -999 } {
  last_ecm_time_degradate_was_run_ = ecm_->getECMtime();
}

PhysicalNode::~PhysicalNode() {
}

StringBuilder& PhysicalNode::simStateToJson(StringBuilder& sb) const {
  sb.append("{");

  SimStateSerializationUtil::keyValue(sb, "ID", id_);
  SimStateSerializationUtil::keyValue(sb, "idCounter", PhysicalNode::id_counter_);
  SimStateSerializationUtil::keyValue(sb, "onTheSchedulerListForPhysicalNodes", on_scheduler_list_for_physical_nodes_);
  SimStateSerializationUtil::keyValue(sb, "lastECMTimeDegradateWasRun", last_ecm_time_degradate_was_run_);
  SimStateSerializationUtil::keyValue(sb, "movementConcentratioUpdateProcedure",
                                      movement_concentration_update_procedure_);

  SimStateSerializationUtil::map(sb, "extracellularSubstances", extracellular_substances_);
  SimStateSerializationUtil::keyValue(sb, "soNode", so_node_.get());

  SimStateSerializationUtil::removeLastChar(sb);
  sb.append("}");
  return sb;
}

std::string PhysicalNode::toString() const {
  std::ostringstream str_stream;
  str_stream << "(";
  str_stream << StringUtil::toStr(id_);
  str_stream << ", ";
  str_stream << StringUtil::toStr(PhysicalNode::id_counter_);
  str_stream << ", ";
  str_stream << StringUtil::toStr(on_scheduler_list_for_physical_nodes_);
  str_stream << ", ";
  str_stream << StringUtil::toStr(last_ecm_time_degradate_was_run_);
  str_stream << ", ";
  str_stream << StringUtil::toStr(movement_concentration_update_procedure_);
  str_stream << ", ";
  str_stream << StringUtil::toStr(extracellular_substances_);
  str_stream << ", ";
  str_stream << StringUtil::toStr(so_node_.get());
  str_stream << ")";
  return str_stream.str();
}

bool PhysicalNode::isAPhysicalObject() const {
  //The function is overwritten in PhysicalObject.
  return false;
}

bool PhysicalNode::isAPhysicalCylinder() const {
  // The function is overwritten in PhysicalSphere.
  return false;
}

bool PhysicalNode::isAPhysicalSphere() const {
  // The function is overwritten in PhysicalSphere
  return false;
}

double PhysicalNode::getExtracellularConcentration(const std::string& id) {
  double c = 0.0;
  if (PhysicalNode::ecm_->thereAreArtificialGradients()) {
    c += PhysicalNode::ecm_->getValueArtificialConcentration(id, so_node_->getPosition());
  }
  Substance* s= nullptr;
  if (!STLUtil::mapContains(extracellular_substances_, id)) {
    return c;
  } else {
    s = extracellular_substances_[id].get();
    degradate(PhysicalNode::ecm_->getECMtime());  // make sure you are up-to-date weight/ degradation
    return c + s->getConcentration();
  }
}

double PhysicalNode::getConvolvedConcentration(const std::string& id) {
  double vol_sum = 0;
  double ex_c = 0;
  double curr_vol = so_node_->getVolume();
  double curr_c = getExtracellularConcentration(id);
  vol_sum = vol_sum + curr_vol;
  ex_c = ex_c + curr_vol * curr_c;
  for (auto pn : so_node_->getNeighbors()) {
    curr_vol = pn->getSoNode()->getVolume();
    curr_c = pn->getExtracellularConcentration(id);
    ex_c = ex_c + curr_vol * curr_c;
    vol_sum = vol_sum + curr_vol;
  }
  return ex_c / vol_sum;
}

double PhysicalNode::getExtracellularConcentration(const std::string& id, const std::array<double, 3>& location) {
  double c = 0.0;
  if (PhysicalNode::ecm_->thereAreArtificialGradients()) {
    c += PhysicalNode::ecm_->getValueArtificialConcentration(id, location);
  }

  bool returned_null = false;
  auto vertices = so_node_->getVerticesOfTheTetrahedronContaining(location, returned_null);
  double concentration_at_location = 0;
  if (!returned_null) {
    auto barycentric_coord = PhysicalNode::getBarycentricCoordinates(location, vertices);
    for (auto j = 0; j < 4; j++) {
      concentration_at_location += vertices[j]->getExtracellularConcentration(id) * barycentric_coord[j];
    }
  }
  return c + concentration_at_location;
}

std::array<double, 3> PhysicalNode::getExtracellularGradient(const std::string& id) {
  // the gradient can be composed of diffusible Substances and artificial substances in ecm
  std::array<double, 3> grad;
  // 1. diffusible substance component
  Substance* s = nullptr;
  if (!STLUtil::mapContains(extracellular_substances_, id)) {
    grad = {0.0, 0.0, 0.0};
  } else {
    s = extracellular_substances_[id].get();
    // distance to three neighbors
    std::array<std::array<double, 3>, 3> vectors_to_neighbors;
    std::array<double, 3> differences_between_neighbors_and_this;
    int index_of_equation = 0;
    // loop through the neighbors until we have selected three of them (indexOfTheEquationWeGet)
    for (auto n : so_node_->getNeighbors()) {
      double substance_concentration_in_neighbor = n->getExtracellularConcentration(id);
      // prepare the linear system to be solved
      vectors_to_neighbors[index_of_equation] = Matrix::subtract(n->soNodePosition(), so_node_->getPosition());
      differences_between_neighbors_and_this[index_of_equation] = (substance_concentration_in_neighbor
          - s->getConcentration());
      index_of_equation++;
      // only three equations;
      if (index_of_equation > 2)
      break;
    }
    grad = Matrix::solve(vectors_to_neighbors, differences_between_neighbors_and_this);
  }
  // 2. ECM's artificial gradient component
  if (PhysicalNode::ecm_->thereAreArtificialGradients()) {
    grad = Matrix::add(grad, PhysicalNode::ecm_->getGradientArtificialConcentration(id, so_node_->getPosition()));
  }
  return grad;
}

void PhysicalNode::modifyExtracellularQuantity(const std::string& id, double quantityPerTime) {
  Substance* ss = nullptr;
  if (STLUtil::mapContains(extracellular_substances_, id)) {
    ss = extracellular_substances_[id].get();
  } else {
    auto ss_uptr = PhysicalNode::ecm_->substanceInstance(id);
    ss = ss_uptr.get();
    extracellular_substances_[id] = std::move(ss_uptr);
  }
  double delta_q = quantityPerTime * Param::kSimulationTimeStep;
  double volume = so_node_->getVolume();
  degradate(PhysicalNode::ecm_->getECMtime());  // make sure you are up-to-date weight/ degradation
  ss->updateQuantityBasedOnConcentration(volume);  // TODO : is this step really necessary ?
  ss->changeQuantityFrom(delta_q);
  ss->updateConcentrationBasedOnQuantity(volume);
  // we will diffuse next time step
  on_scheduler_list_for_physical_nodes_ = true;
}

void PhysicalNode::runExtracellularDiffusion() {
  // 1) now that we are about to diffuse, a new diffusion should only be performed
  // if there is a good reason for that.
  on_scheduler_list_for_physical_nodes_ = false;
  double current_ecm_time = PhysicalNode::ecm_->getECMtime();
  // 2) Degradation according to the degradation constant for each chemical
  degradate(current_ecm_time);
  // 3) Diffusion (along every edge)
  for (auto e : so_node_->getEdges()) {
    diffuseEdgeAnalytically(e, current_ecm_time);
  }
}

Substance* PhysicalNode::getSubstanceInstance(Substance* templateS) {
  Substance* s = nullptr;
  if (!STLUtil::mapContains(extracellular_substances_, templateS->getId())) {
    // if it doesn't exist, you create it (with concentration 0)
    auto s_uptr = Substance::UPtr(new Substance(*templateS));
    s = s_uptr.get();
    s->setConcentration(0);
    s->setQuantity(0);
    extracellular_substances_[s->getId()] = std::move(s_uptr);
  } else {
    s = extracellular_substances_[templateS->getId()].get();
  }
  return s;
}

double PhysicalNode::computeConcentrationAtDistanceBasedOnGradient(Substance* s,
                                                                   const std::array<double, 3>& dX) {
  // if the point that interests us is inside a tetrahedron, we interpolate the
  // value based on the tetrahedron

  // otherwise we compute the gradient, and multiply it with the displacement -> get value;
  auto gradient = getExtracellularGradient(s->getId());
  double new_concentration = s->getConcentration() + Matrix::dot(gradient, dX);
  if (new_concentration < 0) {
    new_concentration = 0;
  }
  return new_concentration;
}

std::array<double, 3> PhysicalNode::soNodePosition() const {
  return so_node_->getPosition();
}

SpatialOrganizationNode<PhysicalNode>* PhysicalNode::getSoNode() const {
  return so_node_.get();
}

void PhysicalNode::setSoNode(typename SpatialOrganizationNode<PhysicalNode>::UPtr son) {
  so_node_ = std::move(son);
}

bool PhysicalNode::isOnTheSchedulerListForPhysicalNodes() const {
  return on_scheduler_list_for_physical_nodes_;
}

void PhysicalNode::setOnTheSchedulerListForPhysicalNodes(bool on_the_scheduler_list_for_physical_nodes) {
  on_scheduler_list_for_physical_nodes_ = on_the_scheduler_list_for_physical_nodes;
}

int PhysicalNode::getMovementConcentratioUpdateProcedure() const {
  return movement_concentration_update_procedure_;
}

void PhysicalNode::setMovementConcentratioUpdateProcedure(int movement_concentration_update_procedure) {
  movement_concentration_update_procedure_ = movement_concentration_update_procedure;
}

void PhysicalNode::addExtracellularSubstance(Substance::UPtr is) {
  extracellular_substances_[is->getId()] = std::move(is);
}

void PhysicalNode::removeExtracellularSubstance(Substance* is) {
  extracellular_substances_.erase(is->getId());
}

std::list<Substance*> PhysicalNode::getExtracellularSubstances() const {  //todo refactor - originally returned the whole map
  std::list<Substance*> list;
  for (auto& i : extracellular_substances_) {
    list.push_front(i.second.get());
  }
  return list;
}

Substance* PhysicalNode::getExtracellularSubstance(const std::string& key) {  //todo refactor - added to avoid implementing unorederd_ma for swig
  if (STLUtil::mapContains(extracellular_substances_, key)) {
    return extracellular_substances_[key].get();
  } else {
    return nullptr;
  }
}

int PhysicalNode::getID() const {
  return id_;
}

void PhysicalNode::degradate(double currentEcmTime) {  //changed to proteceted

  // if we are up-to-date : we stop here.
  if (last_ecm_time_degradate_was_run_ > currentEcmTime) {
    return;
  }
  // Otherwise, degradation according to the degradation constant for each chemical
  double delta_t = currentEcmTime - last_ecm_time_degradate_was_run_;
  for (auto& i : extracellular_substances_) {
    auto s = i.second.get();
    double decay = MathUtil::exp(-s->getDegradationConstant() * delta_t);
    s->multiplyQuantityAndConcentrationBy(decay);
  }
  // We store the current time as the last time we updated degradation
  // (+0.0000001, to be on the safe side of the double comparison)
  last_ecm_time_degradate_was_run_ = currentEcmTime + 0.0000001;
}

void PhysicalNode::diffuseEdgeAnalytically(SpatialOrganizationEdge<PhysicalNode>* e, double current_ecm_time) {
  // the two PhysicalNodes
  auto n_b = e->getOppositeElement(this);

  // make sure the other one is up-to-date with degradation
  n_b->degradate(current_ecm_time);
  // some values about space node distances, contact area and volume
  auto son_a = so_node_.get();
  auto son_b = n_b->getSoNode();
  double dist = Matrix::distance(son_a->getPosition(), son_b->getPosition());
  double v_a = son_a->getVolume();
  double v_b = son_b->getVolume();
  double pre_a = (e->getCrossSection() / dist);
  double pre_m = (e->getCrossSection() / dist) * (1.0 / v_a + 1.0 / v_b);

  // diffusion of all the extracellularSubstances in A :
  for (auto s_a : getExtracellularSubstances()) {
    double s_a_concentration = s_a->getConcentration();
    // stop here if 1) non diffusible substance or 2) concentration very low:
    double diffusion_constant = s_a->getDiffusionConstant();
    if (diffusion_constant < 10E-14 || s_a_concentration < Param::kMinimalConcentrationForExtracellularDiffusion) {
      continue;  // to avoid a division by zero in the n/m if the diff const = 0;
    }
    // find the counterpart in B
    auto s_b = n_b->getSubstanceInstance(s_a);
    double s_b_concentration = s_b->getConcentration();
    // saving time : no diffusion if almost no difference;
    double abs_diff = std::abs(s_a_concentration - s_b_concentration);
    if ((abs_diff < Param::kMinimalDifferenceConcentrationForExtracacellularDiffusion)
        || (abs_diff / s_a_concentration < Param::kMinimalDCOverCForExtracellularDiffusion)) {
      continue;
    }
    // If we reach this point, it means that it is worth performing the diffusion.
    // we thus put ourselves on the list for performing it again next time step.
    n_b->setOnTheSchedulerListForPhysicalNodes(true);
    on_scheduler_list_for_physical_nodes_ = true;

    // Analytical computation of the diffusion between these two nodes
    // (cf document "Diffusion" by F.Zubler for explanation).

    double q_a = s_a->getQuantity();
    double q_b = s_b->getQuantity();
    double tot = q_a + q_b;
    double a = pre_a * diffusion_constant;
    double m = pre_m * diffusion_constant;

    double n = a * tot / v_b;
    double n_over_M = n / m;
    double K = q_a - n_over_M;
    q_a = K * MathUtil::exp(-m * Param::kSimulationTimeStep) + n_over_M;
    q_b = tot - q_a;

    s_a->setQuantity(q_a);
    s_b->setQuantity(q_b);
    // and update their concentration again
    s_a->updateConcentrationBasedOnQuantity(v_a);
    s_b->updateConcentrationBasedOnQuantity(v_b);
  }
}

}  // namespace physics
}  // namespace cx3d

