//
// Created by bogdan on 7/13/16.
//

#include <TView.h>
#include <TEveManager.h>
#include <TEveGeoNode.h>
#include "visualization/gui.h"

using visualization::GUI;

GUI::GUI() : geom(nullptr), top(nullptr), init(false) {}

void GUI::Update() {
  geom->ResetState();


  TGeoVolume *volume;

  // add spheres to the world
  for (auto sphere : ecm->getPhysicalSphereList()) {
    int id = sphere->getID();
    char name[128];
    sprintf(name, "S%d", id);

    double radius = sphere->getDiameter() / 2;

    // name, medSolid, radius of inner sphere, radius of outer sphere
    volume = geom->MakeSphere(name, medSolid, 0., radius);
    // fixme Color_t is short, sphere color is unsigned int
    volume->SetLineColor((Color_t)sphere->getColor().getValue());

    std::array<double, 3> massLocation = sphere->getMassLocation();
    TGeoTranslation *position =
        new TGeoTranslation(massLocation[0], massLocation[1], massLocation[2]);

    top->AddNode(volume, sphere->getID(), position);
  }

  // add cylinders to the world
  for (auto cylinder : ecm->getPhysicalCylinderList()) {
    char name[128];
    sprintf(name, "C%d", cylinder->getID());

    double length = cylinder->getLength();
    double diameter = cylinder->getDiameter();
    std::array<double, 3> massLocation = cylinder->getMassLocation();

    volume = geom->MakeTube(name, medSolid, 0., diameter, length);
    // fixme Color_t is short, cylinder color is unsigned int
    volume->SetLineColor((Color_t)cylinder->getColor().getValue());

    TGeoTranslation *position =
        new TGeoTranslation(massLocation[0], massLocation[1], massLocation[2]);
    top->AddNode(volume, cylinder->getID(), position);
  }

  gEve->Redraw3D(kTRUE);
}

void GUI::Init() {
  this->ecm = ECM::getInstance();

  TEveManager::Create();

  geom = new TGeoManager("Visualization", "Biodynamo");

  // Set number of segments for approximating circles in drawing.
  // Keep it low for better performance.
  geom->SetNsegments(4);

  matEmptySpace = new TGeoMaterial("EmptySpace", 0, 0, 0);
  matSolid = new TGeoMaterial("Solid", .938, 1., 10000.);
  medEmptySpace = new TGeoMedium("Empty", 1, matEmptySpace);
  medSolid = new TGeoMedium("Solid", 1, matSolid);

  // we don't know how to calculate world radius yet
  double worldRadius = 1000.0;
  top = geom->MakeBox("World", medEmptySpace, worldRadius, worldRadius,
                      worldRadius);
  geom->SetTopVolume(top);

  TGeoNode* node = geom->GetTopNode();
  TEveGeoTopNode* en = new TEveGeoTopNode(geom, node);
  gEve->AddGlobalElement(en);


  init = true;
}
