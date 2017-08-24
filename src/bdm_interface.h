// The following classes will enable an interface between BioDynaMo and
// Vasileios Vavourakis' cancer modeling
// framework

#ifndef BDM_INTERFACE_H_
#define BDM_INTERFACE_H_

#include <vector>
#include "backend.h"
#include "cell.h"

namespace bdm {

class Vector3DInterface {
 public:
  Vector3DInterface() { coord_[0] = coord_[1] = coord_[2] = 0.0; }
  Vector3DInterface(double x, double y, double z) {
    coord_[0] = x;
    coord_[1] = y;
    coord_[2] = z;
  }
  Vector3DInterface(const Vector3DInterface& v) {
    coord_[0] = v.coord_[0];
    coord_[1] = v.coord_[1];
    coord_[2] = v.coord_[2];
  }
  ~Vector3DInterface() {}

  double coord_[3];
};

class ContinuousInterfaceData {
 public:
  explicit ContinuousInterfaceData(int num_cell_types = 1) {
    const unsigned int num_vertices = 8;
    oxygen_level_.assign(num_vertices, 0.0);
    oxygen_level_gradient_.assign(num_vertices, Vector3DInterface());
    normoxic_cells_.resize(num_vertices);
    hypoxic_cells_.resize(num_vertices);
    for (unsigned int n = 0; n < num_vertices; n++) {
      normoxic_cells_[n].assign(num_cell_types, 0);
      hypoxic_cells_[n].assign(num_cell_types, 0);
    }
  }
  ~ContinuousInterfaceData() {}

  std::vector<double> oxygen_level_;
  std::vector<Vector3DInterface> oxygen_level_gradient_;
  std::vector<std::vector<unsigned int>> normoxic_cells_;
  std::vector<std::vector<unsigned int>> hypoxic_cells_;
};

class DiscontinuousInterfaceData {
 public:
  DiscontinuousInterfaceData() {}
  ~DiscontinuousInterfaceData() {}

  double ecm_density_;
  Vector3DInterface ecm_density_gradient_;
};

/// Enumeration of the 8 vertices of the BDMCubicDomain as in the following
/// schematic. Thus the
/// continuous interface data are distributed over the 8 points of the
/// hexahedron, while the
/// discontinuous interface data are assumed uniform (constant) in the cubic
/// domain.
///
///          7        6
///          o--------o
///         /:       /|
///        / :      / |
///     4 /  :   5 /  |
///      o--------o   |
///      |   o....|...o 2
///      |  .3    |  /
///      | .      | /
///      |.       |/
///      o--------o
///      0        1
template <int TOptional = 1>
class BDMCubicDomain {
 public:
  BDMCubicDomain() : is_init_(false) {}
  ~BDMCubicDomain() {}

  ContinuousInterfaceData cont_fd_;
  DiscontinuousInterfaceData disc_fd_;

  void Init(size_t ncell, std::vector<Vector3DInterface> v) {
    cells_ = std::vector<bdm::Cell>(ncell);
    if (v.size() != 8) {
      throw;
    } else {
      vertex_ = v;
    }
    is_init_ = true;
  }
  inline bool is_init() const { return is_init_; }  // NOLINT

 private:
  bool is_init_;
  std::vector<bdm::Cell> cells_;
  std::vector<Vector3DInterface> vertex_;
};
}  // namespace bdm
#endif  // BDM_INTERFACE_H_
