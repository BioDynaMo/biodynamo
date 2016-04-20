#ifndef CELLS_CELL_FACTORY_H_
#define CELLS_CELL_FACTORY_H_

#include <array>
#include <vector>
#include <memory>

#include "cells/cell.h"

namespace cx3d {
namespace cells {

using std::vector;

/**
 * <code>CellFacory</code> generates a new  <code>Cell</code>, <code>SomaElement</code>,
 * <code>PhysicalSphere</code> and <code>SpatialOrganizationNode</code>.  We set than the
 * massLocation and cell color.
 * We can generate a single <code>Cell</code> or a list of <code>Cell</code> distributed
 * uniformly.
 */
class CellFactory {
 public:
  /**
   * Generates a single cell at the specified position.
   * @param cell_origin
   * @return
   */
  //todo remove ecm parameter once porting has been finished
  static std::shared_ptr<Cell> getCellInstance(const std::array<double, 3>& cell_origin,
                                               const std::shared_ptr<physics::ECM>& ecm);

  /**
   * Generates a 2D grid of cells according according to the desired number of cells along
   * the y and x axes. Cell position can be randomized by increasing the standard deviation of
   * the Gaussian noise distribution.
   * @param x_min
   * @param x_max
   * @param y_min
   * @param y_max
   * @param n_x: Number of cells along the x axis
   * @param n_y: Number of cells along the y axis
   * @param noise_std: Gaussian noise standard deviation
   * @return cellList
   */
  //todo remove ecm parameter once porting has been finished
  static vector<std::shared_ptr<Cell>> get2DCellGrid(double x_min, double x_max, double y_min, double y_max,
                                                     double z_pos, int n_x, int n_y, double noise_std,
                                                     const std::shared_ptr<physics::ECM>& ecm);

  /**
   * Generates a 3D grid of cells according to the desired number of cells along
   * the y, x and z axes. Cell position can be randomized by increasing the standard deviation of
   * the Gaussian noise distribution.
   * @param x_min
   * @param x_max
   * @param y_min
   * @param y_max
   * @param z_min
   * @param z_max
   * @param n_x: Number of cells along the x axis
   * @param n_y: Number of cells along the y axis
   * @param n_z: Number of cells along the z axis
   * @param noise_xy_std: Gaussian noise standard deviation for the xy axis
   * @param noise_z_std: Gaussian noise standard deviation for the z direction
   * @return cellList
   */
  //todo remove ecm parameter once porting has been finished
  static vector<std::shared_ptr<Cell>> get3DCellGrid(double x_min, double x_max, double y_min, double y_max,
                                                     double z_min, double z_max, int n_x, int n_y, int n_z,
                                                     double noise_xy_std, double noise_z_std,
                                                     const std::shared_ptr<physics::ECM>& ecm);
 private:
  CellFactory() = delete;
  CellFactory(const CellFactory&) = delete;
  CellFactory& operator=(const CellFactory&) = delete;
};

}  // namespace cells
}  // namespace cx3d

#endif  // CELLS_CELL_FACTORY_H_
