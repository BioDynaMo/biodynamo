//
// Created by bogdan on 7/13/16.
//
#include <thread>

#include <TMath.h>
#include <TView.h>
#include <TEveManager.h>
#include <TEveGeoNode.h>
#include <TEveWindow.h>
#include <TEveBrowser.h>
#include <TGComboBox.h>

#include "visualization/gui.h"

#include <TGLAutoRotator.h>

using bdm::visualization::GUI;

GUI::GUI() : init(false), update(false), lastID(0), maxVizNodes(1e6) {}

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
  // double worldRadius = 10000.0;
  // top = geom->MakeBox("World", medEmptySpace, worldRadius, worldRadius,
  //                    worldRadius);
  top = new TGeoVolumeAssembly("WORLD");

  geom->SetTopVolume(top);
  geom->SetMultiThread(true);

  // connect geom to eve
  TGeoNode *node = geom->GetTopNode();
  eveTop = new TEveGeoTopNode(geom, node);
  gEve->AddGlobalElement(eveTop);
  gEve->AddElement(eveTop);

  // number of visualized nodes inside one volume. If you exceed this number,
  // ROOT will draw nothing.
  geom->SetMaxVisNodes(maxVizNodes);

  gEve->GetBrowser()->GetMainFrame()->SetWindowName("Biodynamo Visualization");

  init = true;
}

void GUI::Update(bool resetCamera) {
  if (!init)
    throw std::runtime_error("Call GUI::getInstance().Init() first!");

  top->ClearNodes();

  for (auto sphere : ecm->getPhysicalSphereList()) {
    auto container = new TGeoVolumeAssembly("A");
    addBranch(sphere, container);
    top->AddNode(container, top->GetNdaughters());
  }
  // geom->CloseGeometry();
  gEve->FullRedraw3D(resetCamera);

  update = true;
}

TGeoCombiTrans *GUI::cylinderTransformation(const PhysicalCylinder *cylinder) {
  auto length = cylinder->getActualLength();
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

void GUI::addBranch(PhysicalSphere *sphere, TGeoVolume *container) {
  addSphereToVolume(sphere, container);

  for (auto cylinder : sphere->getDaughters()) {
    addCylinderToVolume(cylinder, container);
    preOrderTraversalCylinder(cylinder, container);
  }
}

void GUI::preOrderTraversalCylinder(PhysicalCylinder *cylinder,
                                    TGeoVolume *container) {
  auto left = cylinder->getDaughterLeft();
  auto right = cylinder->getDaughterRight();

  if (left != nullptr && right != nullptr) {
    // current cylinder is bifurcation
    auto newContainer = new TGeoVolumeAssembly("B");

    addCylinderToVolume(left, newContainer);
    addCylinderToVolume(right, newContainer);

    preOrderTraversalCylinder(left, newContainer);
    preOrderTraversalCylinder(right, newContainer);

    container->AddNode(newContainer, container->GetNdaughters());

    return;
  }

  // add left subtree to container
  if (left != nullptr) {
    addCylinderToVolume(left, container);
    preOrderTraversalCylinder(left, container);
  }

  // add right subtree to container
  if (right != nullptr) {
    addCylinderToVolume(right, container);
    preOrderTraversalCylinder(right, container);
  }
}

void GUI::addCylinderToVolume(PhysicalCylinder *cylinder,
                              TGeoVolume *container) {
  auto id = cylinder->getID();

  // remember last visualized id
  if (lastID < id)
    lastID = id;

  char name[12];
  sprintf(name, "C%d", id);

  auto length = cylinder->getActualLength();
  auto radius = cylinder->getDiameter() / 2;
  auto trans = this->cylinderTransformation(cylinder);

  auto volume = geom->MakeTube(name, medSolid, 0., radius, length / 2);
  volume->SetLineColor(this->translateColor(cylinder->getColor()));

  container->AddNode(volume, container->GetNdaughters(), trans);
}

void GUI::addSphereToVolume(PhysicalSphere *sphere, TGeoVolume *container) {
  auto id = sphere->getID();

  // remember last visualized id
  if (lastID < id)
    lastID = id;

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

  container->AddNode(volume, container->GetNdaughters(), position);
}

void GUI::setMaxVizNodes(int number) { this->maxVizNodes = number; }
