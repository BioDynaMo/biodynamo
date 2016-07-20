//
// Created by bogdan on 7/13/16.
//

#ifndef BIODYNAMO_GUI_H
#define BIODYNAMO_GUI_H

#include <TGeoManager.h>
#include "simulation/ecm.h"
#include "color.h"

namespace visualization {

using bdm::simulation::ECM;
using bdm::Color;
using bdm::physics::PhysicalCylinder;
using bdm::physics::PhysicalSphere;

/**
 * Singleton class, which draws graphical user interface
 */
class GUI {
private: // private members
  TGeoManager *geom;
  TGeoVolume *top;

  TGeoMaterial *matEmptySpace;
  TGeoMaterial *matSolid;
  TGeoMedium *medEmptySpace;
  TGeoMedium *medSolid;

  ECM *ecm;

  bool init;

private: // private functions
  TGeoCombiTrans *cylinderTransformation(const PhysicalCylinder *cylinder);
  EColor translateColor(Color color);
  void addBranch(TGeoVolume *container, PhysicalSphere *sphere);
  void preOrderTraversalCylinder(TGeoVolume *container,
                                 PhysicalCylinder *cylinder);

  void addSphereToVolume(TGeoVolume *container, PhysicalSphere *sphere);
  void addCylinderToVolume(TGeoVolume *container, PhysicalCylinder *cylinder);

public: // public interface
  void Init();
  void Update();

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
#endif // BIODYNAMO_GUI_H
