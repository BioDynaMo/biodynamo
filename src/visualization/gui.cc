#include <TMath.h>
#include <TView.h>
#include <TEveManager.h>
#include <TEveGeoNode.h>
#include <TEveWindow.h>
#include <TEveBrowser.h>
#include <TSystem.h>
#include "visualization/gui.h"

using bdm::visualization::Gui;
using bdm::Color;

Gui::Gui() : init_(false), update_(false), last_id_(0), max_viz_nodes_(1e6) {}

void Gui::Init() {
  this->ecm_ = ECM::getInstance();

  TEveManager::Create();

  geom_ = new TGeoManager("Visualization", "Biodynamo");

  // Set number of segments for approximating circles in drawing.
  // Keep it low for better performance.
  geom_->SetNsegments(4);

  mat_empty_space_ = new TGeoMaterial("EmptySpace", 0, 0, 0);
  mat_solid_ = new TGeoMaterial("Solid", .938, 1., 10000.);
  med_empty_space_ = new TGeoMedium("Empty", 1, mat_empty_space_);
  med_solid_ = new TGeoMedium("Solid", 1, mat_solid_);

  // Another way to make top volume for world. In this way it will be unbounded.
  top_ = new TGeoVolumeAssembly("WORLD");

  geom_->SetTopVolume(top_);
  geom_->SetMultiThread(true);

  // connect geom_ to eve
  TGeoNode *node = geom_->GetTopNode();
  eve_top_ = new TEveGeoTopNode(geom_, node);
  gEve->AddGlobalElement(eve_top_);
  gEve->AddElement(eve_top_);

  // number of visualized nodes inside one volume. If you exceed this number,
  // ROOT will draw nothing.
  geom_->SetMaxVisNodes(max_viz_nodes_);

  gEve->GetBrowser()->GetMainFrame()->SetWindowName("Biodynamo Visualization");

  init_ = true;
}

void Gui::Update() {
  if (!init_)
    throw std::runtime_error("Call GUI::getInstance().Init() first!");

  if(is_geometry_closed_)
    throw std::runtime_error("Geometry is already closed! Don't call GUI::Update() after GUI::CloseGeometry()!");

  top_->ClearNodes();

  for (auto &sphere : *ecm_->getPhysicalSphereListCPtr()) {
    auto container = new TGeoVolumeAssembly("A");
    AddBranch(sphere, container);
    top_->AddNode(container, top_->GetNdaughters());
  }

  gEve->FullRedraw3D();
  gSystem->ProcessEvents();

  update_ = true;
}

TGeoCombiTrans *Gui::CylinderTransformation(const PhysicalCylinder *cylinder) {
  auto length = cylinder->getActualLength();
  auto springAxis = cylinder->getSpringAxis();
  auto massLocation = cylinder->getMassLocation();

  auto x1 = massLocation[0];
  auto y1 = massLocation[1];
  auto z1 = massLocation[2];

  auto dx = springAxis[0];
  auto dy = springAxis[1];
  auto dz = springAxis[2];

  auto position = TGeoTranslation(x1 - dx / 2, y1 - dy / 2, z1 - dz / 2);

  auto phiX = 0.0, thetaY = acos(dz / length) * 180. / M_PI, psiZ = 0.0;

  if ((dx < 0 && dy > 0 && dz > 0) || (dx > 0 && dy < 0 && dz < 0)) {
    phiX = 180. - atan2(dx, dy) * 180. / M_PI;
  } else {
    phiX = atan2(dy, dx) * 180. / M_PI + 90.;
  }

  auto rotation = TGeoRotation("rot", phiX, thetaY, psiZ);

  return new TGeoCombiTrans(position, rotation);
}

EColor Gui::TranslateColor(Color color) {
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

void Gui::AddBranch(PhysicalSphere *sphere, TGeoVolume *container) {
  AddSphereToVolume(sphere, container);

  for (auto &cylinder : sphere->getDaughters()) {
    AddCylinderToVolume(cylinder, container);
    PreOrderTraversalCylinder(cylinder, container);
  }
}

void Gui::PreOrderTraversalCylinder(PhysicalCylinder *cylinder,
                                    TGeoVolume *container) {
  auto left = cylinder->getDaughterLeft();
  auto right = cylinder->getDaughterRight();

  if (left != nullptr && right != nullptr) {
    // current cylinder is bifurcation
    auto newContainer = new TGeoVolumeAssembly("B");

    AddCylinderToVolume(left, newContainer);
    AddCylinderToVolume(right, newContainer);

    PreOrderTraversalCylinder(left, newContainer);
    PreOrderTraversalCylinder(right, newContainer);

    container->AddNode(newContainer, container->GetNdaughters());

    return;
  }

  // add left subtree to container
  if (left != nullptr) {
    AddCylinderToVolume(left, container);
    PreOrderTraversalCylinder(left, container);
  }

  // add right subtree to container
  if (right != nullptr) {
    AddCylinderToVolume(right, container);
    PreOrderTraversalCylinder(right, container);
  }
}

void Gui::AddCylinderToVolume(PhysicalCylinder *cylinder,
                              TGeoVolume *container) {
  auto id = cylinder->getID();

  // remember last visualized id
  if (last_id_ < id)
    last_id_ = id;

  char name[12];
  snprintf(name, 12, "C%d", id);

  auto length = cylinder->getActualLength();
  auto radius = cylinder->getDiameter() / 2;
  auto trans = this->CylinderTransformation(cylinder);

  auto volume = geom_->MakeTube(name, med_solid_, 0., radius, length / 2);
  volume->SetLineColor(this->TranslateColor(cylinder->getColor()));

  container->AddNode(volume, container->GetNdaughters(), trans);
}

void Gui::AddSphereToVolume(PhysicalSphere *sphere, TGeoVolume *container) {
  auto id = sphere->getID();

  // remember last visualized id
  if (last_id_ < id)
    last_id_ = id;

  char name[12];
  snprintf(name, 12, "S%d", id);

  auto radius = sphere->getDiameter() / 2;
  auto massLocation = sphere->getMassLocation();
  auto x = massLocation[0];
  auto y = massLocation[1];
  auto z = massLocation[2];
  auto position = new TGeoTranslation(x, y, z);

  auto volume = geom_->MakeSphere(name, med_solid_, 0., radius);
  volume->SetLineColor(this->TranslateColor(sphere->getColor()));

  container->AddNode(volume, container->GetNdaughters(), position);
}

void Gui::SetMaxVizNodes(int number) { this->max_viz_nodes_ = number; }

void Gui::CloseGeometry() {
  if (!update_)
    throw std::runtime_error("Call GUI::getInstance().Update() first!");

  geom_->CloseGeometry();

  is_geometry_closed_ = true;
}
