#include <vector>
#include "gtest/gtest.h"

#include "physics/default_force.h"
#include "physics/physical_sphere.h"
#include "simulation/ecm.h"
#include "spatial_organization/space_node.h"

namespace bdm {

using simulation::ECM;
using physics::DefaultForce;
using physics::PhysicalNode;
using physics::PhysicalObject;
using physics::PhysicalSphere;
using spatial_organization::SpaceNode;

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

TEST(TargetValue_PhysicalSphere, displacement) {
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

  EXPECT_NEAR(0, final_pos_cell1[0], 1e-5);
  EXPECT_NEAR(0, final_pos_cell1[1], 1e-5);
  EXPECT_NEAR(0, final_pos_cell1[2], 1e-5);

  EXPECT_NEAR(0, final_pos_cell2[0], 1e-5);
  EXPECT_NEAR(5, final_pos_cell2[1], 1e-5);
  EXPECT_NEAR(0, final_pos_cell2[2], 1e-5);
}

}  // namespace bdm
