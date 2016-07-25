//
// Created by bogdan on 7/13/16.
//

#ifndef BIODYNAMO_GUI_H
#define BIODYNAMO_GUI_H

#include <thread>
#include <mutex>

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
public:
  std::mutex simulation;
  std::mutex visualization;

  void nextStep(); // for debug purposes only

  // members related to visualization
private:
  TGeoManager *geom;
  TGeoVolume *top;
  TEveGeoTopNode *eveTop;

  TGeoMaterial *matEmptySpace;
  TGeoMaterial *matSolid;
  TGeoMedium *medEmptySpace;
  TGeoMedium *medSolid;

  ECM *ecm;

  // just to ensure that methods were called in correct order
  bool init;
  bool update;
  bool animation;

  // private util functions
private:
  /**
   * Calculates cylinder transformations: position (based on massLocation) and
   * rotation (based on springAxis)
   */
  TGeoCombiTrans *cylinderTransformation(const PhysicalCylinder *cylinder);
  /**
   * Gets ROOT's EColor from bdm::Color
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
   * Creates TEveManager, initializes members
   */
  void Init();
  /**
   * Update objects in the scene
   */
  void Update();

  /**
   * Show animation tab in the Eve browser
   */
  void ShowAnimationTab();

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