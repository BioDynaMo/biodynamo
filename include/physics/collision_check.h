#ifndef PHYSICS_COLLISION_CHECK_H_
#define PHYSICS_COLLISION_CHECK_H_

#include <array>
#include <memory>

namespace bdm {
namespace physics {

class PhysicalCylinder;

class CollisionCheck {
 public:
  static double howMuchCanWeMove(const std::array<double, 3>& A, const std::array<double, 3>& B,
                                 const std::array<double, 3>& C, const std::array<double, 3>& D,
                                 const std::array<double, 3>& E, double dim);

  static void addPhysicalBondIfCrossing(const std::array<double, 3>& A, const std::array<double, 3>& B,
                                        const std::array<double, 3>& C, PhysicalCylinder* moving,
                                        PhysicalCylinder* still);

 private:
  CollisionCheck() = delete;
};

}  // namespace physics
}  // namespace bdm

#endif  // PHYSICS_COLLISION_CHECK_H_
