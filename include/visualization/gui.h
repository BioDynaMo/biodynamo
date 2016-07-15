//
// Created by bogdan on 7/13/16.
//

#ifndef BIODYNAMO_GUI_H
#define BIODYNAMO_GUI_H

#include <memory>
#include <TGeoManager.h>
#include "simulation/ecm.h"


namespace visualization {

using bdm::simulation::ECM;

/**
 * Singleton class, which draws graphical user interface
 */
class GUI {
// members
 private:
  TGeoManager *geom;
  TGeoVolume  *top;

  TGeoMaterial *matEmptySpace;
  TGeoMaterial *matSolid;
  TGeoMedium *medEmptySpace;
  TGeoMedium *medSolid;

  ECM *ecm;

  bool init;

 public:
  void Init();
  void Update();

// Singleton properties:
private:
  GUI();

public: // Singleton methods
  static GUI &getInstance() {
    static GUI instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }

  // C++ 11
  GUI(GUI const &) = delete;
  void operator=(GUI const &) = delete;

  // Note: Scott Meyers mentions in his Effective Modern
  //       C++ book, that deleted functions should generally
  //       be public as it results in better error messages
  //       due to the compilers behavior to check accessibility
  //       before deleted status
};
} // namespace visualization
#endif // BIODYNAMO_GUI_H
