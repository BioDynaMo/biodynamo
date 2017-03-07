//=================================================================================================
#ifndef BDM_INTERFACE_H_
#define BDM_INTERFACE_H_
//=================================================================================================
#include <vector>
#include "backend.h"
#include "cell.h"
#include "daosoa.h"
#include "resource_manager.h"
#include "scheduler.h"
#include "timing.h"

//=================================================================================================

namespace bdm {
// The following classes will enable an interface between BioDynaMo and
// Vasileios Vavourakis' cancer modeling
// framework

class Vector3DInterface {
 public:
  // public member functions: constructor(s) & destructor
  Vector3DInterface() { comp[0] = comp[1] = comp[2] = 0.0; }
  Vector3DInterface(double x, double y, double z) {
    comp[0] = x;
    comp[1] = y;
    comp[2] = z;
  }
  Vector3DInterface(const Vector3DInterface& v) {
    comp[0] = v.comp[0];
    comp[1] = v.comp[1];
    comp[2] = v.comp[2];
  }
  ~Vector3DInterface() {}

 public:
  // public member objects
  double comp[3];
};

//=================================================================================================
class ContinuousInterfaceData {
 public:
  // public member functions: constructor(s) & destructor
  explicit ContinuousInterfaceData(int num_cell_types = 1) {
    const unsigned int num_vertices = 8;
    oxygen_level.assign(num_vertices, 0.0);
    oxygen_level_gradient.assign(num_vertices, Vector3DInterface());
    normoxic_cells.resize(num_vertices);
    hypoxic_cells.resize(num_vertices);
    for (unsigned int n = 0; n < num_vertices; n++) {
      normoxic_cells[n].assign(num_cell_types, 0);
      hypoxic_cells[n].assign(num_cell_types, 0);
    }
  }
  ~ContinuousInterfaceData() {}

 public:
  // public member objects: input/output data
  std::vector<double> oxygen_level;
  std::vector<Vector3DInterface> oxygen_level_gradient;
  std::vector<std::vector<unsigned int> > normoxic_cells;
  std::vector<std::vector<unsigned int> > hypoxic_cells;
};
//-------------------------------------------------------------------------------------------------
class DiscontinuousInterfaceData {
 public:
  // public member functions: constructor(s) & destructor
  DiscontinuousInterfaceData() {}
  ~DiscontinuousInterfaceData() {}

 public:
  // public member objects: input/output data
  double ecm_density;
  Vector3DInterface ecm_density_gradient;
};

//=================================================================================================
// Enumeration of the 8 vertices of the BDMCubicDomain as in the following
// schematic. Thus the
// continuous interface data are distributed over the 8 points of the
// hexahedron, while the
// discontinuous interface data are assumed uniform (constant) in the cubic
// domain.
//
//        7        6
//        o--------o
//       /:       /|
//      / :      / |
//   4 /  :   5 /  |
//    o--------o   |
//    |   o....|...o 2
//    |  .3    |  /
//    | .      | /
//    |.       |/
//    o--------o
//    0        1
//
//=================================================================================================
class BDMCubicDomain {
 public:
  // public member functions: constructor(s) & destructor
  BDMCubicDomain() : _is_init(false) {}
  ~BDMCubicDomain() {}
  // public member functions:
  void init(size_t ncell, std::vector<Vector3DInterface> v) {
    _cells = bdm::daosoa<bdm::Cell>(ncell);
    if (v.size() != 8)
      throw;
    else
      _vertex = v;
    _is_init = true;
  }
  inline bool is_init() const { return _is_init; }

 private:
  // private member objects
  bool _is_init;
  bdm::daosoa<bdm::Cell> _cells;
  std::vector<Vector3DInterface> _vertex;

 public:
  // public member objects
  ContinuousInterfaceData cont_fd;
  DiscontinuousInterfaceData disc_fd;
};
}   // namespace bdm
//=================================================================================================
#endif   // BDM_INTERFACE_H_
//=================================================================================================
