#ifndef DEFAULT_FORCE_H_
#define DEFAULT_FORCE_H_

#include <algorithm>
#include <cmath>
#include "Vc/Vc"

#include "backend.h"
#include "random.h"

namespace bdm {

class DefaultForce {
 public:

  DefaultForce() {}
  ~DefaultForce() {}
  DefaultForce(const DefaultForce&) = delete;
  DefaultForce& operator=(const DefaultForce&) = delete;

  void forceBetweenSpheres(const std::array<double, 3>& ref_mass_location,
                           double ref_diameter, double ref_iof_coefficient,
                           const std::array<double, 3>& nb_mass_location,
                           double nb_diameter, double nb_iof_coefficient,
                           std::array<double, 3>* result) {
    auto c1 = ref_mass_location;
    double r1 = 0.5 * ref_diameter;
    auto c2 = nb_mass_location;
    double r2 = 0.5 * nb_diameter;
    // We take virtual bigger radii to have a distant interaction, to get a
    // desired density.
    double additionalRadius =
        10.0 * std::min(ref_iof_coefficient, nb_iof_coefficient);
    r1 += additionalRadius;
    r2 += additionalRadius;
    // the 3 components of the vector c2 -> c1
    double comp1 = c1[0] - c2[0];
    double comp2 = c1[1] - c2[1];
    double comp3 = c1[2] - c2[2];
    double distanceBetweenCenters =
        std::sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);
    // the overlap distance (how much one penetrates in the other)
    double delta = r1 + r2 - distanceBetweenCenters;
    // if no overlap : no force
    if (delta < 0) {
      *result = {0.0, 0.0, 0.0};
      return;
    }
    // to avoid a division by 0 if the centers are (almost) at the same
    //  location
    if (distanceBetweenCenters < 0.00000001) {
      auto force2on1 = random_.nextNoise(3.0);
      *result = force2on1;
      return;
    }
    // the force itself
    double R = (r1 * r2) / (r1 + r2);
    double gamma = 1;  // attraction coeff
    double k = 2;      // repulsion coeff
    double F = k * delta - gamma * std::sqrt(R * delta);

    double module = F / distanceBetweenCenters;
    std::array<double, 3> force2on1(
        {module * comp1, module * comp2, module * comp3});
    *result = force2on1;
  }

 private:
  Random random_;
};

}  // namespace bdm

#endif  // DEFAULT_FORCE_H_
