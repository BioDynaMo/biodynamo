#ifndef VISUALIZATION_GUI_H_
#define VISUALIZATION_GUI_H_

#include <TGeoManager.h>
#include <TEveGeoNode.h>
#include "simulation/ecm.h"
#include "color.h"

namespace bdm {
namespace visualization {

using bdm::simulation::ECM;
using bdm::Color;
using bdm::physics::PhysicalCylinder;
using bdm::physics::PhysicalSphere;

/**
 * Singleton class, which creates graphical user interface for biodynamo
 * simulation
 */
class Gui {
public:
  static Gui &getInstance() {
    static Gui instance;  // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }

  /**
   * Creates TEveManager window, initializes members
   */
  void Init();

  /**
   * Updates GLViewer of TEveManager according to current state of ECM.
   */
  void Update();

  /**
   * Setter for this->MaxVizNodes
   * @param number - number of visualizable nodes per volume
   */
  void SetMaxVizNodes(int number);

  /**
   * After building the full geometry tree, geometry must be closed.
   * Closing geometry implies checking the geometry validity, fixing shapes with
   * negative parameters (run-time shapes) building the cache manager, voxelizing
   * all volumes, counting the total number of physical nodes and registering
   * the manager class to the browser.
   */
  void CloseGeometry();

 private:
  Gui();
  Gui(Gui const &) = delete;
  Gui &operator=(Gui const &) = delete;

  /**
   * Calculates cylinder transformations: position (based on massLocation) and
   * rotation (based on springAxis and massLocation)
   */
  TGeoCombiTrans *CylinderTransformation(const PhysicalCylinder *cylinder);

  /**
   * Returns ROOT's EColor from bdm::Color
   */
  EColor TranslateColor(Color color);

  /**
   * Recursively adds sphere and its daughters to the container.
   */
  void AddBranch(PhysicalSphere *sphere, TGeoVolume *container);

  /**
   * Util function for recursive pre-order traversal of cylinders in one
   * sphere's daughter
   */
  void PreOrderTraversalCylinder(PhysicalCylinder *cylinder,
                                 TGeoVolume *container);

  /**
   * Adds sphere to the volume, its name will be: "S%d", sphereID
   */
  void AddSphereToVolume(PhysicalSphere *sphere, TGeoVolume *container);

  /**
   * Adds cylinder to the volume, its name will be: "C%d", cylinderID
   */
  void AddCylinderToVolume(PhysicalCylinder *cylinder, TGeoVolume *container);

  /**
   * Geometry manager
   */
  TGeoManager *geom_;

  /**
   * Top volumes for TGeo and TEve (world)
   */
  TGeoVolume *top_;
  TEveGeoTopNode *eve_top_;

  /**
   * Eve materials and medium
   */
  TGeoMaterial *mat_empty_space_;
  TGeoMaterial *mat_solid_;
  TGeoMedium *med_empty_space_;
  TGeoMedium *med_solid_;

  /**
   * Reference to the ECM.
   */
  ECM *ecm_;

  // just to ensure that methods were called in correct order
  bool init_;  // true if init_ was called
  bool update_;  // true if update was called
  bool is_geometry_closed_;  // true if geometry is already closed

  /**
   * Last visualized node ID.
   */
  int last_id_;

  /**
   * Max visualized shapes per volume.
   */
  int max_viz_nodes_;
};
}  // namespace visualization
}  // namespace bdm
#endif  // VISUALIZATION_GUI_H_
