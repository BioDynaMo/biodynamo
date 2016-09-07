/// \brief this file is used to obtain results of scalar calculations that will
/// be used in automated tests of the new prototype

#include <iostream>
#include <vector>
#include "simulation/ecm.h"

#include "spatial_organization/space_node.h"
#include "physics/default_force.h"
#include "physics/physical_sphere.h"

using bdm::simulation::ECM;
using bdm::physics::PhysicalNode;
using bdm::physics::PhysicalObject;
using bdm::physics::PhysicalSphere;
using bdm::physics::DefaultForce;
using bdm::spatial_organization::SpaceNode;

template <typename T>
class SpaceNodeMock : public SpaceNode<T> {
 public:
  SpaceNodeMock(const std::array<double, 3>& position, T* content,
                const std::vector<T*>& neighbors)
      : SpaceNode<T>(position, content), neighbors_{neighbors} {}
  virtual ~SpaceNodeMock() {}
  std::vector<T*> getNeighbors() const override { return neighbors_; }

 private:
  std::vector<T*> neighbors_;
};

void interObjectForce() {
  std::cout << "------------------------------------------------" << std::endl;
  std::cout << __FUNCTION__ << std::endl;
  PhysicalSphere cell1;
  cell1.setAdherence(0.4);
  cell1.setDiameter(10);
  cell1.setMass(1.1);
  std::array<double, 3> pos1 = {0, 0, 0};
  cell1.setMassLocation(pos1);

  PhysicalSphere cell2;
  cell2.setAdherence(0.4);
  cell2.setDiameter(10);
  cell2.setMass(1.1);
  std::array<double, 3> pos2 = {0, 5, 0};
  cell2.setMassLocation(pos2);

  DefaultForce force;
  auto force1 = force.forceOnASphereFromASphere(&cell1, &cell2);
  auto force2 = force.forceOnASphereFromASphere(&cell2, &cell1);

  std::cout << force1[0] << " - " << force1[1] << " - " << force1[2] << " - "
            << std::endl;
  std::cout << force2[0] << " - " << force2[1] << " - " << force2[2] << " - "
            << std::endl;
}

void physicalSphereDisplacement() {
  std::cout << "------------------------------------------------" << std::endl;
  std::cout << __FUNCTION__ << std::endl;
  PhysicalObject::setInterObjectForce(
      std::unique_ptr<DefaultForce>(new DefaultForce()));
  ECM::getInstance()->setArtificialWallsForSpheres(false);
  PhysicalSphere cell1;
  cell1.setAdherence(0.4);
  cell1.setDiameter(10);
  cell1.setMass(1.1);
  std::array<double, 3> pos1 = {0, 0, 0};
  cell1.setMassLocation(pos1);
  //  cell1.setTractorForce({0.99, 0.98, 0.97});

  PhysicalSphere cell2;
  cell2.setAdherence(0.4);
  cell2.setDiameter(10);
  cell2.setMass(1.1);
  std::array<double, 3> pos2 = {0, 5, 0};
  cell2.setMassLocation(pos2);
  //  cell1.setTractorForce({1.01, 1.02, 1.03});

  auto sn1 = SpaceNode<PhysicalNode>::UPtr(new SpaceNodeMock<PhysicalNode>(
      pos1, &cell1, std::vector<PhysicalNode*>{&cell2}));
  auto sn1_raw = sn1.get();
  cell1.setSoNode(std::move(sn1));

  auto sn2 = SpaceNode<PhysicalNode>::UPtr(new SpaceNodeMock<PhysicalNode>(
      pos2, &cell2, std::vector<PhysicalNode*>{&cell1}));
  auto sn2_raw = sn2.get();
  cell2.setSoNode(std::move(sn2));

  cell1.runPhysics();
  cell2.runPhysics();

  const auto& final_pos_cell1 = sn1_raw->getPosition();
  const auto& final_pos_cell2 = sn2_raw->getPosition();
  std::cout << final_pos_cell1[0] << " - " << final_pos_cell1[1] << " - "
            << final_pos_cell1[2] << " - " << std::endl;
  std::cout << final_pos_cell2[0] << " - " << final_pos_cell2[1] << " - "
            << final_pos_cell2[2] << " - " << std::endl;
}

int main() {
  interObjectForce();
  physicalSphereDisplacement();
}
