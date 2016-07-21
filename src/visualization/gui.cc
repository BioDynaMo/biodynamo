//
// Created by bogdan on 7/13/16.
//

#include <TMath.h>
#include <TView.h>
#include <TEveManager.h>
#include <TEveGeoNode.h>
#include <TEveWindow.h>
#include <TEveBrowser.h>
#include <TVector3.h>

#include "visualization/gui.h"

using visualization::GUI;
using bdm::physics::PhysicalCylinder;

GUI::GUI() : geom(nullptr), top(nullptr), init(false) {}

void GUI::Update() {
  for (auto sphere : ecm->getPhysicalSphereList()) {
    auto container = new TGeoVolumeAssembly("A");
    addBranch(sphere, container);
    top->AddNode(container, top->GetNdaughters());
  }
  geom->CloseGeometry();
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
  double worldRadius = 10000.0;
  top = geom->MakeBox("World", medEmptySpace, worldRadius, worldRadius,
                      worldRadius);
  geom->SetTopVolume(top);
  geom->SetMultiThread(true);

  // connect geom to eve
  TGeoNode *node = geom->GetTopNode();
  eveTop = new TEveGeoTopNode(geom, node);
  gEve->AddGlobalElement(eveTop);
  gEve->AddElement(eveTop);

  // number of visualized nodes inside one volume. If you exceed this number,
  // ROOT will draw nothing.
  geom->SetMaxVisNodes((Int_t)1e6);
  init = true;

  createAnimationTab();
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
  /**
   * This is the fastest way to create formatted string, according to my
   * benchmark:
   * http://pastebin.com/YRwyECMH
   *  sprintf:	613151.000000 us
   *  string: 	733208.000000 us
   *  sstream:	3179678.000000 us
   */
  char name[12];
  sprintf(name, "C%d", cylinder->getID());

  auto length = cylinder->getActualLength();
  auto radius = cylinder->getDiameter() / 2;
  auto trans = this->cylinderTransformation(cylinder);

  auto volume = geom->MakeTube(name, medSolid, 0., radius, length / 2);
  volume->SetLineColor(this->translateColor(cylinder->getColor()));

  container->AddNode(volume, container->GetNdaughters(), trans);
}

void GUI::addSphereToVolume(PhysicalSphere *sphere, TGeoVolume *container) {
  char name[12];
  sprintf(name, "S%d", sphere->getID());

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

void GUI::createAnimationTab() {

  auto browser = gEve->GetBrowser();
  browser->StartEmbedding(TRootBrowser::kLeft);

  TGMainFrame *mainFrame = browser->GetMainFrame();
  mainFrame->SetWindowName("Biodynamo Visualization");

  browser->StopEmbedding();
  browser->SetTabTitle("Animation", 0);
}

PhysicalCylinder *GUI::mergeCylinders(PhysicalCylinder *cyl1,
                                      PhysicalCylinder *cyl2, double maxAngle) {
  auto c1loc = cyl1->getSpringAxis();
  auto c2loc = cyl2->getSpringAxis();

  TVector3 v1(c1loc[0], c1loc[1], c1loc[2]);
  TVector3 v2(c2loc[0], c2loc[1], c2loc[2]);

  double angle = v1.Angle(v2) * 180. / M_PI;

  if (angle < maxAngle) {
    auto newCylinder = cyl2->getCopy().get();
    newCylinder->setColor(bdm::Param::kGreen);

    newCylinder->setMassLocation(cyl2->getMassLocation());
    auto ml1 = cyl1->getMassLocation();
    auto ml2 = cyl2->getMassLocation();

    newCylinder->setSpringAxis({ml2[0] + c2loc[0] - ml1[0],
                                ml2[1] + c2loc[1] - ml1[1],
                                ml2[2] + c2loc[2] - ml1[2]});

    auto sa = newCylinder->getSpringAxis();
    newCylinder->setActualLength(sqrt(sa[0] * sa[0] + sa[1] * sa[1] + sa[2] * sa[2]));

    return newCylinder;
  } else {
    return nullptr;
  }
}
