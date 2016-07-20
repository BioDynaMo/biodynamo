//
// Created by bogdan on 7/13/16.
//

#include <TMath.h>
#include <TView.h>
#include <TEveManager.h>
#include <TEveGeoNode.h>

#include "visualization/gui.h"

using visualization::GUI;

GUI::GUI() : geom(nullptr), top(nullptr), init(false) {}

void GUI::Update() {
  for (auto sphere : ecm->getPhysicalSphereList()) {
    auto sphereId = sphere->getID();

    char name[12];
    sprintf(name, "A%d", sphereId);

    auto container = new TGeoVolumeAssembly(name);
    addBranch(container, sphere);
    top->AddNode(container, sphereId);
  }

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

  // geom->SetMaxVisNodes(1000000);

  init = true;
}

TGeoCombiTrans *GUI::cylinderTransformation(const PhysicalCylinder *cylinder) {
  auto length = cylinder->getLength();
  auto springAxis = cylinder->getSpringAxis();
  auto massLocation = cylinder->getMassLocation();

  auto x1 = massLocation[0];
  auto y1 = massLocation[1];
  auto z1 = massLocation[2];

  auto dx = springAxis[0];
  auto dy = springAxis[1];
  auto dz = springAxis[2];

  auto position = new TGeoTranslation(x1 - dx / 2, y1 - dy / 2, z1 - dz / 2);

  auto phiX = 0.0, thetaY = acos(dz / length) * 180. / M_PI, psiZ = 0.0;

  if ((dx < 0 && dy > 0 && dz > 0) || (dx > 0 && dy < 0 && dz < 0)) {
    phiX = 180. - atan2(dx, dy) * 180. / M_PI;
  } else {
    phiX = atan2(dy, dx) * 180. / M_PI + 90.;
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

void GUI::addBranch(TGeoVolume *container, PhysicalSphere *sphere) {
  addSphereToVolume(container, sphere);

  for (auto cylinder : sphere->getDaughters()) {
    addCylinderToVolume(container, cylinder);
    preOrderTraversalCylinder(container, cylinder);
  }
}

void GUI::preOrderTraversalCylinder(TGeoVolume *container,
                                    PhysicalCylinder *cylinder) {
  auto left = cylinder->getDaughterLeft();
  auto right = cylinder->getDaughterRight();

  if (left != nullptr && right != nullptr) {
    // current cylinder is bifurcation
    char name[13];
    sprintf(name, "AB%d", cylinder->getID());

    auto newContainer = new TGeoVolumeAssembly(name);
    addCylinderToVolume(newContainer, left);
    addCylinderToVolume(newContainer, right);

    preOrderTraversalCylinder(newContainer, left);
    preOrderTraversalCylinder(newContainer, right);

    container->AddNode(newContainer, cylinder->getID());

    return;
  }

  if (left != nullptr) {
    addCylinderToVolume(container, left);
    preOrderTraversalCylinder(container, left);
  }

  if (right != nullptr) {
    addCylinderToVolume(container, right);
    preOrderTraversalCylinder(container, right);
  }
}

void GUI::addCylinderToVolume(TGeoVolume *container,
                              PhysicalCylinder *cylinder) {
  int id = cylinder->getID();

  char name[12];
  sprintf(name, "C%d", id);

  auto length = cylinder->getLength();
  auto radius = cylinder->getDiameter() / 2;
  auto trans = this->cylinderTransformation(cylinder);

  auto volume = geom->MakeTube(name, medSolid, 0., radius, length / 2);
  volume->SetLineColor(this->translateColor(cylinder->getColor()));

  container->AddNode(volume, id, trans);
}

void GUI::addSphereToVolume(TGeoVolume *container, PhysicalSphere *sphere) {
  int id = sphere->getID();

  char name[12];
  sprintf(name, "S%d", id);

  auto radius = sphere->getDiameter() / 2;
  auto massLocation = sphere->getMassLocation();
  auto x = massLocation[0];
  auto y = massLocation[1];
  auto z = massLocation[2];
  auto position = new TGeoTranslation(x, y, z);

  auto volume = geom->MakeSphere(name, medSolid, 0., radius);
  volume->SetLineColor(this->translateColor(sphere->getColor()));

  container->AddNode(volume, id, position);
}
