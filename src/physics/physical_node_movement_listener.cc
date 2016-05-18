#include "physics/physical_node_movement_listener.h"

#include "matrix.h"
#include "physics/substance.h"
#include "physics/physical_node.h"

#include "spatial_organization/space_node.h"

namespace cx3d {
namespace physics {

using std::size_t;

int PhysicalNodeMovementListener::movement_operation_id_ = 0;

PhysicalNodeMovementListener::UPtr PhysicalNodeMovementListener::create() {
  return UPtr(new PhysicalNodeMovementListener());
}

SpatialOrganizationNodeMovementListener<PhysicalNode>::UPtr PhysicalNodeMovementListener::getCopy() const {
  return SpatialOrganizationNodeMovementListener<PhysicalNode>::UPtr(new PhysicalNodeMovementListener());
}

void PhysicalNodeMovementListener::nodeAboutToMove(const SpaceNode<PhysicalNode>* node,
                                                   const std::array<double, 3>& planned_movement) {
  auto pn = node->getUserObject();
  neighbors_before_ = node->getPermanentListOfNeighbors();
  auto extracellular_substances_in_pn = pn->getExtracellularSubstances();
  substances_in_n_ = std::vector<Substance*>(extracellular_substances_in_pn.size());
  q_ = std::vector<double>(substances_in_n_.size());
  movement_operation_id_ = (movement_operation_id_ + 1) % 1000000;
  // 1) Quantities summation in pn & pn's neighbors before the move:
  // (for all extracellularSubstances in the moving node)
  size_t i = 0;
  for (auto s : extracellular_substances_in_pn) {
    q_[i] = s->getQuantity();
    substances_in_n_[i] = s;
    auto s_id = s->getId();
    for (auto nn : neighbors_before_) {
      nn->setMovementConcentratioUpdateProcedure(movement_operation_id_);
      auto ss = nn->getExtracellularSubstance(s_id);
      if (ss != nullptr) {
        q_[i] += ss->getQuantity();
      }
    }
    i++;
  }
  // 2) computing the concentration at the future new location of pn (for every substance)

  // if possible : find the tetrahedron the new location belongs to :
  auto future_position = Matrix::add(pn->getSoNode()->getPosition(), planned_movement);
  std::array<int, 1> returned_null;
  auto vertices = pn->getSoNode()->getVerticesOfTheTetrahedronContaining(future_position, returned_null);
  // if there is one : find by interpolation the new concentration :
  // we weight the concentration of each vertex by the barycentric coord of the new point
  if (returned_null[0] != 1) {
    auto barycentric_coord = PhysicalNode::getBarycentricCoordinates(future_position, vertices);
    for (i = 0; i < substances_in_n_.size(); i++) {
      auto name = substances_in_n_[i]->getId();
      double new_concentration = 0;
      for (size_t j = 0; j < 4; j++) {
        new_concentration += vertices[j]->getExtracellularConcentration(name) * barycentric_coord[j];
      }
      substances_in_n_[i]->setConcentration(new_concentration);
    }
  } else {
    // if we can't find a tetra, we compute the gradient and multiply this by the displacement
    for (i = 0; i < substances_in_n_.size(); i++) {
      double new_concentration = pn->computeConcentrationAtDistanceBasedOnGradient(substances_in_n_[i],
                                                                                   planned_movement);
      substances_in_n_[i]->setConcentration(new_concentration);
    }
  }
}

void PhysicalNodeMovementListener::nodeMoved(const SpaceNode<PhysicalNode>* node) {
  auto pn = node->getUserObject();
  auto neighbors_after = node->getNeighbors();
  std::vector<PhysicalNode*> new_neighbors;

  // 3) identifying the really new neighbors of n
  // (i.e. the ones that were not neighbors before the movement)
  for (auto nn : neighbors_after) {
    if (nn->getMovementConcentratioUpdateProcedure() != movement_operation_id_) {
      new_neighbors.push_back(nn);
    }
  }

  // 4) adding all the new neighbors contribution to the total quantity before the move:
  for (int i = 0; i < substances_in_n_.size(); i++) {
    for (auto nn : new_neighbors) {
      auto ss = nn->getExtracellularSubstance(substances_in_n_[i]->getId());
      if (ss != nullptr) {
        q_[i] += ss->getQuantity();
      }
    }
  }

  // For all extracellularSubstances:
  for (int i = 0; i < substances_in_n_.size(); i++) {

    // 5) Update the quantities in every cell that has been affected, and sum it
    // 5.a) in pn itself
    auto s = substances_in_n_[i];
    s->updateQuantityBasedOnConcentration(pn->getSoNode()->getVolume());
    double quantity_after = s->getQuantity();
    // 5.b) in the old neighbors
    for (auto nn : neighbors_before_) {
      auto ss = nn->getExtracellularSubstance(s->getId());
      if (ss != nullptr) {
        ss->updateQuantityBasedOnConcentration(nn->getSoNode()->getVolume());
        quantity_after += ss->getQuantity();
      }
    }
    // 5.c) in the NEW neighbors
    for (auto nn : new_neighbors) {
      auto ss = nn->getExtracellularSubstance(s->getId());
      if (ss != nullptr) {
        ss->updateQuantityBasedOnConcentration(nn->getSoNode()->getVolume());
        quantity_after += ss->getQuantity();
      }
    }

    // 6) defining a ratio of quantity change (quantity before / quantity after) for the i-th substance
    if (quantity_after < 1.0E-14) {
      q_[i] = 0;     // (avoid division by 0)
    } else {
      q_[i] /= quantity_after;
      //        q[i] = 1;       // de-comment this for DEACTIVATION !!!!!!!!!!!!!!!!!!!!!
    }

    // 7) changing the concentration of the i-th substance by its ratio
    // 7.a) in pn
    s->multiplyQuantityAndConcentrationBy(q_[i]);
    // 7.b) in the old neighbors
    for (auto nn : neighbors_before_) {
      auto ss = nn->getExtracellularSubstance(s->getId());
      // Note : should not use PhysicalNode.giveYourSubstanceInstance(), because never returns null
      if (ss != nullptr) {
        ss->multiplyQuantityAndConcentrationBy(q_[i]);
      }
    }
    // 7.c) in the NEW neighbors
    for (auto nn : new_neighbors) {
      auto ss = nn->getExtracellularSubstance(s->getId());
      if (ss != nullptr) {
        ss->multiplyQuantityAndConcentrationBy(q_[i]);
      }
    }
  }
}

void PhysicalNodeMovementListener::nodeAboutToBeRemoved(const SpaceNode<PhysicalNode>* node) {
  auto pn = node->getUserObject();
  neighbors_before_ = node->getPermanentListOfNeighbors();
  auto pn_extracellular_substances = pn->getExtracellularSubstances();
  substances_in_n_ = std::vector<Substance*>(pn_extracellular_substances.size());
  q_ = std::vector<double>(substances_in_n_.size());
  // 1) Quantities summation in pn & pn's neighbors (before pn is removed):
  // (for all extracellularSubstances in the moving node)
  int i = 0;
  for (auto s : pn_extracellular_substances) {
    q_[i] = s->getQuantity();
    substances_in_n_[i] = s;
    for (auto nn : neighbors_before_) {
      auto ss = nn->getExtracellularSubstance(s->getId());
      if (ss != nullptr) {
        q_[i] += ss->getQuantity();
      }
    }
    i++;
  }
}

void PhysicalNodeMovementListener::nodeRemoved(const SpaceNode<PhysicalNode>* node) {
  // For all extracellularSubstances:
  for (int i = 0; i < substances_in_n_.size(); i++) {

    // 2) Update the quantities in the old neighbors, and sum it
    auto s = substances_in_n_[i];
    double quantity_after = 0;
    for (auto nn : neighbors_before_) {
      auto ss = nn->getExtracellularSubstance(s->getId());
      if (ss != nullptr) {
        ss->updateQuantityBasedOnConcentration(nn->getSoNode()->getVolume());
        quantity_after += ss->getQuantity();
      }
    }

    // 3) defining a ratio of quantity change (quantity before / quantity after) for the i-th substance
    if (quantity_after < 1.0E-14) {
      q_[i] = 0;  //(avoid division by 0)
    } else {
      q_[i] /= quantity_after;
    }

    // 4) changing the concentration of the i-th substance by its ratio, in the old neighbors
    for (auto nn : neighbors_before_) {
      auto ss = nn->getExtracellularSubstance(s->getId());
      if (ss != nullptr) {
        ss->multiplyQuantityAndConcentrationBy(q_[i]);
      }
    }
  }
}

void PhysicalNodeMovementListener::nodeAboutToBeAdded(const SpaceNode<PhysicalNode>* node,
                                                      const std::array<double, 3>& planned_position,
                                                      const std::array<PhysicalNode*, 4>& vertices) {
  auto pn = node->getUserObject();
  if (vertices[0] != nullptr) {  // fixme hack:  && vertices[0] != null
    auto pnn = vertices[0];  // a future neighbor of the PhysicalNode about to be inserted
    // (we have to rely on it to know the chemicals present )
    auto barycentric_coord = PhysicalNode::getBarycentricCoordinates(planned_position, vertices);

    for (auto s : pnn->getExtracellularSubstances()) {
      auto name = s->getId();
      double new_concentration = 0;
      for (size_t j = 0; j < 4; j++) {
        new_concentration += vertices[j]->getExtracellularConcentration(name) * barycentric_coord[j];
      }
      auto new_substance = Substance::UPtr(new Substance(*s));
      new_substance->setConcentration(new_concentration);
      //pn.getExtracellularSubstances().put(name, newSubstance); //fixme this looks like a bug - hashmap is cloned -> inserting a value won't have an effect
    }
  }
}

void PhysicalNodeMovementListener::nodeAdded(const SpaceNode<PhysicalNode>* node) {
  // 2) sum the quantity before update
  auto pn = node->getUserObject();
  // since there might be no substances yet in the point
  // pn, we take a neighbor as furnishing the templates
  auto neighbors = node->getPermanentListOfNeighbors();
  auto pnn = neighbors.front();
  substances_in_n_ = std::vector<Substance*>(pnn->getExtracellularSubstances().size());  //todo more efficient if # of needed elements are already reserved here
  q_ = std::vector<double>(substances_in_n_.size());
  int i = 0;
  for (auto s : pnn->getExtracellularSubstances()) {
    q_[i] = 0;
    substances_in_n_[i] = s;
    for (auto nn : neighbors) {  //todo critical
      auto ss = nn->getExtracellularSubstance(s->getId());
      if (ss != nullptr) {
        q_[i] += ss->getQuantity();
      }
    }
    i++;
  }

  // 3) update quantities in all nodes, and sum it :
  for (i = 0; i < substances_in_n_.size(); i++) {
    double quantity_after = 0;
    // 3.a) in pn itself
    auto s = substances_in_n_[i];
    auto ss = pn->getExtracellularSubstance(s->getId());
    if (ss != nullptr) {
      ss->updateQuantityBasedOnConcentration(node->getVolume());
      quantity_after += ss->getQuantity();
    }

    // 3.b) in the  neighbors
    for (auto nn : node->getNeighbors()) {
      auto pn_1 = nn;
      ss = pn_1->getExtracellularSubstance(s->getId());
      if (ss != nullptr) {
        ss->updateQuantityBasedOnConcentration(pn_1->getSoNode()->getVolume());
        quantity_after += ss->getQuantity();
      }
    }

    // 4) defining a ratio of quantity change (quantity before / quantity after) for the i-th substance
    if (quantity_after < 1.0E-14) {
      q_[i] = 0;  //(avoid division by 0)
    } else {
      q_[i] /= quantity_after;
      //        q[i] = 1;     // De-comment this for DEACTIVATION !!!!!!!!!!!!!!!!!!!!!
    }

    // 5) changing the concentration of the i-th substance by its ratio
    // 5.a) in pn
    ss = pn->getExtracellularSubstance(s->getId());
    if (ss != nullptr) {
      ss->multiplyQuantityAndConcentrationBy(q_[i]);
    }
    // 5.b) in the neighbors
    for (auto nn : node->getNeighbors()) {
      ss = nn->getExtracellularSubstance(s->getId());
      if (ss != nullptr) {
        ss->multiplyQuantityAndConcentrationBy(q_[i]);
      }
    }
  }
}

std::string PhysicalNodeMovementListener::toString() const {
  return "PhyscicalNodeMovementListener";
}

}  // namespace cx3d
}  // namespace physics

