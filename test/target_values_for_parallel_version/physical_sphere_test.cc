#include "gtest/gtest.h"
#include <vector>

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

TEST(TargetValue_PhysicalSphere, transformCoordinatesGlobalToPolar) {
  PhysicalSphere sphere;
  std::array<double, 3> coord = {1, 2, 3};
  sphere.setMassLocation({9, 8, 7});
  auto result = sphere.transformCoordinatesGlobalToPolar(coord);

  EXPECT_NEAR(10.770329614269007, result[0], 1e-5);
  EXPECT_NEAR(1.9513027039072615, result[1], 1e-5);
  EXPECT_NEAR(-2.4980915447965089, result[2], 1e-5);
}

TEST(TargetValue_PhysicalSphere, divide) {
  PhysicalSphere mother;
  std::array<double, 3> position = {5, 6, 7};
  mother.setMassLocation({5, 6, 7});
  // mother.setTractorForce({0, 0, 0});
  mother.setDiameter(10);
  mother.setAdherence(1.1);
  mother.setMass(5);
  mother.setXAxis({1, 2, 3});
  mother.setYAxis({4, 5, 6});
  mother.setZAxis({7, 8, 9});

  auto sn = SpaceNode<PhysicalNode>::UPtr(new SpaceNodeMock<PhysicalNode>(
      position, &mother, std::vector<PhysicalNode*>{}));
  mother.setSoNode(std::move(sn));

  auto daughter = mother.divide(0.75, 0.12, 0.34);

  // verify mother data members
  EXPECT_NEAR(0.16369893089539111, mother.getSoNode()->getPosition()[0], 1e-5);
  EXPECT_NEAR(0.39656253332927616, mother.getSoNode()->getPosition()[1], 1e-5);
  EXPECT_NEAR(0.6294261357631612, mother.getSoNode()->getPosition()[2], 1e-5);

  EXPECT_NEAR(0.16369893089539111, mother.getMassLocation()[0], 1e-5);
  EXPECT_NEAR(0.39656253332927616, mother.getMassLocation()[1], 1e-5);
  EXPECT_NEAR(0.6294261357631612, mother.getMassLocation()[2], 1e-5);

  // EXPECT_NEAR(0, mother.getTractorForce()[0], 1e-5);
  // EXPECT_NEAR(0, mother.getTractorForce()[1], 1e-5);
  // EXPECT_NEAR(0, mother.getTractorForce()[2], 1e-5);

  EXPECT_NEAR(8.2982653336624335, mother.getDiameter(), 1e-5);
  EXPECT_NEAR(299.19930057142852, mother.getVolume(), 1e-5);
  EXPECT_NEAR(1.1, mother.getAdherence(), 1e-5);
  EXPECT_NEAR(2.8571428571428563, mother.getMass(), 1e-5);  // FIXME bug??

  EXPECT_NEAR(1, mother.getXAxis()[0], 1e-5);
  EXPECT_NEAR(2, mother.getXAxis()[1], 1e-5);
  EXPECT_NEAR(3, mother.getXAxis()[2], 1e-5);

  EXPECT_NEAR(4, mother.getYAxis()[0], 1e-5);
  EXPECT_NEAR(5, mother.getYAxis()[1], 1e-5);
  EXPECT_NEAR(6, mother.getYAxis()[2], 1e-5);

  EXPECT_NEAR(7, mother.getZAxis()[0], 1e-5);
  EXPECT_NEAR(8, mother.getZAxis()[1], 1e-5);
  EXPECT_NEAR(9, mother.getZAxis()[2], 1e-5);

  // verify daughter data members
  EXPECT_NEAR(11.448401425472813, daughter->getSoNode()->getPosition()[0],
              1e-5);
  EXPECT_NEAR(13.471249955560966, daughter->getSoNode()->getPosition()[1],
              1e-5);
  EXPECT_NEAR(15.494098485649118, daughter->getSoNode()->getPosition()[2],
              1e-5);

  EXPECT_NEAR(11.448401425472813, daughter->getMassLocation()[0], 1e-5);
  EXPECT_NEAR(13.471249955560966, daughter->getMassLocation()[1], 1e-5);
  EXPECT_NEAR(15.494098485649118, daughter->getMassLocation()[2], 1e-5);

  // EXPECT_NEAR(0, mother.getTractorForce()[0], 1e-5);
  // EXPECT_NEAR(0, mother.getTractorForce()[1], 1e-5);
  // EXPECT_NEAR(0, mother.getTractorForce()[2], 1e-5);

  EXPECT_NEAR(7.5394744112915388, daughter->getDiameter(), 1e-5);
  EXPECT_NEAR(224.39947542857155, daughter->getVolume(), 1e-5);
  EXPECT_NEAR(1.1, daughter->getAdherence(), 1e-5);
  EXPECT_NEAR(2.1428571428571437, daughter->getMass(), 1e-5);  // FIXME bug??

  EXPECT_NEAR(1, daughter->getXAxis()[0], 1e-5);
  EXPECT_NEAR(2, daughter->getXAxis()[1], 1e-5);
  EXPECT_NEAR(3, daughter->getXAxis()[2], 1e-5);

  EXPECT_NEAR(4, daughter->getYAxis()[0], 1e-5);
  EXPECT_NEAR(2, daughter->getXAxis()[1], 1e-5);
  EXPECT_NEAR(3, daughter->getXAxis()[2], 1e-5);

  EXPECT_NEAR(7, daughter->getZAxis()[0], 1e-5);
  EXPECT_NEAR(8, daughter->getZAxis()[1], 1e-5);
  EXPECT_NEAR(9, daughter->getZAxis()[2], 1e-5);

  // additional check
  EXPECT_NEAR(5, mother.getMass() + daughter->getMass(), 1e-5);
}

}  // namespace bdm
