#include "cells/cell_factory.h"

#include "physics/physical_node.h"
#include "physics/physical_sphere.h"
#include "local_biology/soma_element.h"
#include "spatial_organization/space_node.h"

namespace cx3d {
namespace cells {

using std::vector;

Cell* CellFactory::getCellInstance(const std::array<double, 3>& cell_origin,
                                   const std::shared_ptr<simulation::ECM>& ecm) {
  // Create new cell
  auto cell = new Cell();
  auto soma = SomaElement::UPtr(new SomaElement());
  auto ps = PhysicalSphere::UPtr(new PhysicalSphere());
  auto son = ecm->getSpatialOrganizationNodeInstance(cell_origin, ps.get());
  ps->setSoNode(son);

  // Add cell to ECM instance
  ecm->addPhysicalSphere(ps.get());  //fixme critical

  // Set cell properties
  ps->setMassLocation(cell_origin);
  ps->setColor(ecm->cellTypeColor(cell->getType()));

  soma->setPhysical(std::move(ps));
  cell->setSomaElement(std::move(soma));
  return cell;
}

vector<Cell*> CellFactory::get2DCellGrid(double x_min, double x_max, double y_min, double y_max,
                                                         double z_pos, int n_x, int n_y, double noise_std,
                                                         const std::shared_ptr<simulation::ECM>& ecm) {

  // Insert all generated cells in a vector
  vector<Cell* > cell_list;
  double dx = (x_max - x_min) / (1 + n_x);
  double dy = (y_max - y_min) / (1 + n_y);

  // Generate cells
  for (int i = 1; i < n_x + 1; i++) {
    for (int j = 1; j < n_y + 1; j++) {
      std::array<double, 3> new_location { x_min + i * dx + ecm->getGaussianDouble(0, noise_std), y_min + j * dy
          + ecm->getGaussianDouble(0, noise_std), z_pos + ecm->getGaussianDouble(0, noise_std) };
      auto cell = getCellInstance(new_location, ecm);
      cell_list.push_back(cell);
    }
  }
  return cell_list;
}

vector<Cell*> CellFactory::get3DCellGrid(double x_min, double x_max, double y_min, double y_max,
                                                         double z_min, double z_max, int n_x, int n_y, int n_z,
                                                         double noise_xy_std, double noise_z_std,
                                                         const std::shared_ptr<simulation::ECM>& ecm) {

  // Insert all generated cells in a vector
  vector<Cell*> cell_list;
  double dx = (x_max - x_min) / (1 + n_x);
  double dy = (y_max - y_min) / (1 + n_y);
  double dz = (z_max - z_min) / (1 + n_z);

  // Generate cells
  for (int i = 1; i < n_x + 1; i++) {
    for (int j = 1; j < n_y + 1; j++) {
      for (int k = 1; k < n_z + 1; k++) {
        std::array<double, 3> new_location { x_min + i * dx + ecm->getGaussianDouble(0, noise_xy_std), y_min + j * dy
            + ecm->getGaussianDouble(0, noise_xy_std), z_min + k * dz + ecm->getGaussianDouble(0, noise_z_std) };
        auto cell = getCellInstance(new_location, ecm);
        cell_list.push_back(cell);
      }
    }
  }
  return cell_list;
}

}  // namespace cells
}  // namespace cx3d
