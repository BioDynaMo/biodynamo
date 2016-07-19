//
// Created by bogdan on 7/13/16.
//

#include <TMath.h>
#include <TView.h>
#include <TEveManager.h>
#include <TEveGeoNode.h>
#include <TBenchmark.h>

#include "visualization/gui.h"

using visualization::GUI;

GUI::GUI() : geom(nullptr), top(nullptr), init(false) {}

void GUI::Update() {
  TGeoVolume *volume;

  printf("[Info] Adding spheres to the world\n");
  // add spheres to the world
  for (auto sphere : ecm->getPhysicalSphereList()) {
    int id = sphere->getID();
    char name[128];
    sprintf(name, "S%d", id);

    double radius = sphere->getDiameter() / 2;

    // name, medSolid, radius of inner sphere, radius of outer sphere
    volume = geom->MakeSphere(name, medSolid, 0., radius);

    EColor color = this->translateColor(sphere->getColor());
    volume->SetLineColor(color);

    auto massLocation = sphere->getMassLocation();
    TGeoTranslation *position =
        new TGeoTranslation(massLocation[0], massLocation[1], massLocation[2]);

    top->AddNode(volume, sphere->getID(), position);
  }

  printf("[Info] Adding cylinders to the world\n");
  // add cylinders to the world
  for (auto cylinder : ecm->getPhysicalCylinderList()) {
    char name[128];
    sprintf(name, "C%d", cylinder->getID());

    auto length = cylinder->getLength();
    auto radius = cylinder->getDiameter() / 2;

    volume = geom->MakeTube(name, medSolid, 0., radius, length / 2);

    EColor color = this->translateColor(cylinder->getColor());
    volume->SetLineColor(color);

    TGeoCombiTrans *trans = this->cylinderTransformation(cylinder);
    top->AddNode(volume, cylinder->getID(), trans);
  }

  printf("[Info] Finished adding nodes to the world\n");


  geom->CloseGeometry();
  top->Draw("ogl");
}

void GUI::Init() {
  this->ecm = ECM::getInstance();

  // TEveManager::Create();

  geom = new TGeoManager("Visualization", "Biodynamo");

  // Set number of segments for approximating circles in drawing.
  // Keep it low for better performance.
  geom->SetNsegments(4);

  matEmptySpace = new TGeoMaterial("EmptySpace", 0, 0, 0);
  matSolid = new TGeoMaterial("Solid", .938, 1., 10000.);
  medEmptySpace = new TGeoMedium("Empty", 1, matEmptySpace);
  medSolid = new TGeoMedium("Solid", 1, matSolid);

  // we don't know how to calculate world radius yet
  double worldRadius = 10000.0;
  top = geom->MakeBox("World", medEmptySpace, worldRadius, worldRadius,
                      worldRadius);
  geom->SetTopVolume(top);

  // TGeoNode *node = geom->GetTopNode();
  // TEveGeoTopNode *en = new TEveGeoTopNode(geom, node);
  // gEve->AddGlobalElement(en);

  geom->SetMaxVisNodes(1000000);

  init = true;
}

TGeoCombiTrans *GUI::cylinderTransformation(const PhysicalCylinder *cylinder) {
  double phiX = 0.0, thetaY = 0.0, psiZ = 0.0;

  double length = cylinder->getLength();
  auto springAxis = cylinder->getSpringAxis();
  auto massLocation = cylinder->getMassLocation();

  auto x1 = massLocation[0];
  auto y1 = massLocation[1];
  auto z1 = massLocation[2];

  auto dx = springAxis[0];
  auto dy = springAxis[1];
  auto dz = springAxis[2];

  auto position = new TGeoTranslation(x1 - dx / 2, y1 - dy / 2, z1 - dz / 2);

  if ((dx > 0 && dy > 0 && dz > 0) || (dx < 0 && dy < 0 && dz < 0)) {
    phiX = atan2(dy, dx) * 180. / M_PI + 90.;
    thetaY = acos(dz / length) * 180. / M_PI;

  } else if ((dx < 0 && dy > 0 && dz > 0) || (dx > 0 && dy < 0 && dz < 0)) {
    phiX = 180. - atan2(dx, dy) * 180. / M_PI;
    thetaY = acos(dz / length) * 180. / M_PI;

  } else if ((dx < 0 && dy < 0 && dz > 0) || (dx > 0 && dy > 0 && dz < 0)) {
    phiX = atan2(dy, dx) * 180. / M_PI + 90.;
    thetaY = acos(dz / length) * 180. / M_PI;

  } else if ((dx < 0 && dy > 0 && dz < 0) || (dx > 0 && dy < 0 && dz > 0)) {
    phiX = atan2(dy, dx) * 180. / M_PI + 90.;
    thetaY = acos(dz / length) * 180. / M_PI;
  }

  auto rotation = new TGeoRotation("rot", phiX, thetaY, psiZ);

  return new TGeoCombiTrans(*position, *rotation);
}

EColor GUI::translateColor(Color color) {
  if (color == bdm::Param::kYellow) {
    return kYellow;
  } else if (color == bdm::Param::kViolet) {
    return kViolet;
  } else if (color == bdm::Param::kBlue) {
    return kBlue;
  } else if (color == bdm::Param::kRed) {
    return kRed;
  } else if (color == bdm::Param::kGreen) {
    return kGreen;
  } else if (color == bdm::Param::kGray) {
    return kGray;
  } else {
    // ROOT doesn't know this `color`, return kAzure :)
    return kAzure;
  }
}
