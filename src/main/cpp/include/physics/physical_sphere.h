#ifndef PHYSICS_PHYSICAL_SPHERE_H_
#define PHYSICS_PHYSICAL_SPHERE_H_

#include <array>
#include <exception>

#include "physics/physical_object.h"

namespace cx3d {
namespace physics {

class PhysicalSphere : public PhysicalObject {
 public:
  virtual ~PhysicalSphere() {
  }
};

}  //namespace physics
}  //namespace cx3d

#endif  // PHYSICS_PHYSICAL_SPHERE_H_
