#ifndef BIODYNAMO_GUI_H
#define BIODYNAMO_GUI_H

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
class GUI {
  // members related to GUI
private:
  /**
   * Geometry manager
   */
  TGeoManager *geom;

  /**
   * Top volumes for TGeo and TEve (world)
   */
  TGeoVolume *top;
  TEveGeoTopNode *eveTop;

  /**
   * Eve materials and medium
   */
  TGeoMaterial *matEmptySpace;
  TGeoMaterial *matSolid;
  TGeoMedium *medEmptySpace;
  TGeoMedium *medSolid;

  /**
   * Reference to the ECM.
   */
  ECM *ecm;

  // just to ensure that methods were called in correct order
  bool init;       // true if init was called
  bool update;     // true if update was called
  bool geomClosed; // true if geometry is already closed

  int lastID; // last visualized ID

  /**
   * Max visualized shapes per volume
   */
  int maxVizNodes;

  // private util functions related to visualization
private:
  /**
   * Calculates cylinder transformations: position (based on massLocation) and
   * rotation (based on springAxis)
   */
  TGeoCombiTrans *cylinderTransformation(const PhysicalCylinder *cylinder);

  /**
   * Returns ROOT's EColor from bdm::Color
   */
  EColor translateColor(Color color);

  /**
   * Recursively adds sphere and its daughters to the container.
   */
  void addBranch(PhysicalSphere *sphere, TGeoVolume *container);

  /**
   * Util function for recursive pre-order traversal of cylinders in one
   * sphere's daughter
   */
  void preOrderTraversalCylinder(PhysicalCylinder *cylinder,
                                 TGeoVolume *container);

  /**
   * Adds sphere to the volume, its name will be: "S%d", sphereID
   */
  void addSphereToVolume(PhysicalSphere *sphere, TGeoVolume *container);

  /**
   * Adds cylinder to the volume, its name will be: "C%d", cylinderID
   */
  void addCylinderToVolume(PhysicalCylinder *cylinder, TGeoVolume *container);

  // public interface
public:
  /**
   * Creates TEveManager window, initializes members
   */
  void Init();

  /**
   * Updates GLViewer of TEveManager according to current state of ECM.
   * @param resetCamera - if true, camera will move to observe all scene
   */
  void Update(bool resetCamera = true);

  /**
   * Setter for this->maxVizNodes
   * @param number
   */
  void setMaxVizNodes(int number);

  /**
   * After building the full geometry tree, geometry must be closed.
   * Closing geometry implies checking the geometry validity, fixing shapes with
   * negative parameters (run-time shapes)building the cache manager, voxelizing
   * all volumes, counting the total number of physical nodes and registering
   * the manager class to the browser.
   */
  void CloseGeometry();

private: // singleton properties
  GUI();

public: // singleton interface
  static GUI &getInstance() {
    static GUI instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }

  // C++ 11
  GUI(GUI const &) = delete;
  GUI &operator=(GUI const &) = delete;

  // Note: Scott Meyers mentions in his Effective Modern
  //       C++ book, that deleted functions should generally
  //       be public as it results in better error messages
  //       due to the compilers behavior to check accessibility
  //       before deleted status
};
} // namespace visualization
} // namespace bdm
#endif // BIODYNAMO_GUI_H