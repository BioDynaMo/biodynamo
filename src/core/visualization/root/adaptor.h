// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------
#ifndef VISUALIZATION_ROOT_ADAPTOR_H_
#define VISUALIZATION_ROOT_ADAPTOR_H_

#include <TCanvas.h>
#include <TEveBrowser.h>
#include <TEveGeoNode.h>
#include <TEveManager.h>
#include <TGeoManager.h>
#include <TMath.h>
#include <TSystem.h>
#include <TVector3.h>
#include "Math/AxisAngle.h"
#include "Math/EulerAngles.h"

#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/shape.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/math.h"
#include "neuroscience/neurite_element.h"

using ROOT::Math::AxisAngle;
using ROOT::Math::EulerAngles;
using bdm::experimental::neuroscience::NeuriteElement;

namespace bdm {

/// The class that bridges the simulation code with ROOT Visualization
class RootAdaptor {
 public:
  RootAdaptor() : initialized_(false), max_viz_nodes_(1e6) {}

  /// Visualize one timestep
  void Visualize(uint64_t total_steps) {
    if (!initialized_) {
      Initialize();
      initialized_ = true;
    }

    auto *param = Simulation::GetActive()->GetParam();
    if (total_steps % param->visualization_export_interval_ != 0) {
      return;
    }

    top_->ClearNodes();

    auto *rm = Simulation::GetActive()->GetResourceManager();

    rm->ApplyOnAllElements([&](SimObject *so) {
      auto container = new TGeoVolumeAssembly("A");
      this->AddBranch(so, container);
      top_->AddNode(container, top_->GetNdaughters());
    });

    gSystem->ProcessEvents();
    gGeoManager->Export(outfile_.c_str(), "biodynamo");
  }

  void DrawInCanvas(size_t w = 300, size_t h = 300, std::string opt = "") {
    canvas_->GetListOfPrimitives()->Clear();
    if (opt.empty()) {
      canvas_->GetListOfPrimitives()->Add(gGeoManager->GetTopVolume(), "all");
    } else {
      opt = "all;" + opt;
      canvas_->GetListOfPrimitives()->Add(gGeoManager->GetTopVolume(),
                                          opt.c_str());
    }
    canvas_->SetCanvasSize(w, h);
    canvas_->Update();
    canvas_->Draw();
  }

 private:
  void Initialize() {
    gGeoManager = new TGeoManager("Visualization", "BioDynaMo");
    canvas_ = new TCanvas("BioDynaMo Canvas", "For ROOT Notebooks ", 300, 300);

    outfile_ = Simulation::GetActive()->GetUniqueName() + ".root";

    // Set number of segments for approximating circles in drawing.
    // Keep it low for better performance.
    gGeoManager->SetNsegments(15);

    mat_empty_space_ = new TGeoMaterial("EmptySpace", 0, 0, 0);
    mat_solid_ = new TGeoMaterial("Solid", .938, 1., 10000.);
    med_empty_space_ = new TGeoMedium("Empty", 1, mat_empty_space_);
    med_solid_ = new TGeoMedium("Solid", 1, mat_solid_);

    // Another way to make top volume for world. In this way it will be
    // unbounded.
    top_ = new TGeoVolumeAssembly("WORLD");

    gGeoManager->SetTopVolume(top_);
    gGeoManager->SetVisLevel(4);

    // Number of visualized nodes inside one volume. If you exceed this number,
    // ROOT will draw nothing.
    gGeoManager->SetMaxVisNodes(max_viz_nodes_);

    // Assign the gGeoManager to our canvas to enable resizing of the output
    // display. The "all" option is necessary to prevent ROOT from hiding
    // objects (which it does by default for performance reasons).
    canvas_->GetListOfPrimitives()->Add(gGeoManager->GetTopVolume(), "all");

    initialized_ = true;
  }

  /// Recursively adds sphere and its daughters to the container.
  void AddBranch(const SimObject *so, TGeoVolume *container) {
    switch (so->GetShape()) {
      case Shape::kSphere:
        AddSphere(so, container);
        break;
      case Shape::kCylinder:
        AddCylinder(so, container);
        break;
      default:
        Log::Error("RootAdaptor",
                   "Tried to add a shape to the Root visualization that's not "
                   "one of the supported types : ",
                   so->GetShape());
    }
    // to be extended for other object
  }

  /// Adds a sphere object to the volume
  void AddSphere(const SimObject *so, TGeoVolume *container) {
    std::string name = so->GetTypeName() + std::to_string(so->GetUid());
    auto radius = so->GetDiameter() / 2;
    auto massLocation = so->GetPosition();
    auto x = massLocation[0];
    auto y = massLocation[1];
    auto z = massLocation[2];
    auto position = new TGeoTranslation(x, y, z);
    auto volume = gGeoManager->MakeSphere(name.c_str(), med_solid_, 0, radius);
    volume->SetLineColor(kBlue);
    container->AddNode(volume, container->GetNdaughters(), position);
  }

  /// Adds a cylinder object to the volume
  void AddCylinder(const SimObject *so, TGeoVolume *container) {
    if (auto neurite = dynamic_cast<const NeuriteElement *>(so)) {
      std::string name = so->GetTypeName() + std::to_string(so->GetUid());
      auto radius = neurite->GetDiameter() / 2;
      auto half_length = neurite->GetLength() / 2;
      auto massLocation = neurite->GetPosition();
      auto x = massLocation[0];
      auto y = massLocation[1];
      auto z = massLocation[2];
      auto trans = new TGeoTranslation(x, y, z);

      // The initial orientation of the cylinder
      TVector3 orig(0, 0, 1);

      // The vector that we want to orient the cylinder to (symmetry axis
      // aligns with this vector)
      Double3 d = neurite->GetSpringAxis();
      TVector3 dir(d[0], d[1], d[2]);
      dir = dir.Unit();

      // Compute the Axis-Angle rotation representation
      auto dot_product = dir.Dot(orig);
      auto angle = std::acos(dot_product);
      // TODO: make sure it is `z x dir, and not `dir x z`
      TVector3 n = dir.Cross(orig);
      n = n.Unit();
      auto axis = AxisAngle::AxisVector(n[0], n[1], n[2]);
      AxisAngle aa(axis, angle);
      EulerAngles ea(aa);
      TGeoRotation *rot = new TGeoRotation("rot", Math::ToDegree(ea.Phi()),
                                           Math::ToDegree(ea.Theta()),
                                           Math::ToDegree(ea.Psi()));
      TGeoCombiTrans *transrot = new TGeoCombiTrans(*trans, *rot);
      auto volume = gGeoManager->MakeTube(name.c_str(), med_solid_, 0, radius,
                                          half_length);
      volume->SetLineColor(kBlue);
      container->AddNode(volume, container->GetNdaughters(), transrot);
    } else {
      Log::Error("RootAdaptor", "This is not a cylindrical shaped object!");
    }
  }

  std::string outfile_;

  TCanvas *canvas_ = nullptr;

  /// Top volumes for TGeo and TEve (world)
  TGeoVolume *top_;

  /// Eve materials and medium
  TGeoMaterial *mat_empty_space_;
  TGeoMaterial *mat_solid_;
  TGeoMedium *med_empty_space_;
  TGeoMedium *med_solid_;

  bool initialized_;

  /// Max visualized shapes per volume.
  int max_viz_nodes_;
};

}  // namespace bdm

#endif  // VISUALIZATION_ROOT_ADAPTOR_H_
