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

#ifndef GUI_VISUALIZATION_FRAME_H_
#define GUI_VISUALIZATION_FRAME_H_

#include <iostream>

#include <TCanvas.h>
#include <TColor.h>
#include <TEveBrowser.h>
#include <TEveGedEditor.h>
#include <TEveGeoNode.h>
#include <TEveManager.h>
#include <TEveViewer.h>
#include <TGButton.h>
#include <TGeoManager.h>
#include <TRootEmbeddedCanvas.h>
#include <TSystem.h>
#include <TView.h>
#include "TGClient.h"

#include "biodynamo.h"
#include "gui/constants.h"
#include "gui/controller/project.h"

namespace gui {

class VisFrame : public TGCompositeFrame {
 public:
  // Constructor and destructor
  // VisFrame(const TGWindow* p);
  VisFrame(const TGWindow* p) : TGCompositeFrame(p, 600, 400) {
    fL1 = std::make_unique<TGLayoutHints>(
        kLHintsTop | kLHintsRight | kLHintsExpandX | kLHintsExpandY, 5, 2, 2, 2);
    fEmbeddedCanvas =
        new TRootEmbeddedCanvas("fEmbeddedCanvas", this, 580, 360);
    AddFrame(fEmbeddedCanvas, fL1.get());
    fEmbeddedCanvas->GetCanvas()->SetBorderMode(0);
    fCanvas = fEmbeddedCanvas->GetCanvas();
    fCanvas->SetFillColor(1);

    fHFrame = std::make_unique<TGHorizontalFrame>(this, 0, 0, 0);
    // Create Zoom Buttons
    fZoomPlusButton = std::make_unique<TGTextButton>(
        fHFrame.get(), "&Zoom Forward", M_ZOOM_PLUS);
    fZoomPlusButton->Associate(this);
    fZoomPlusButton->SetToolTipText("Zoom forward");
    fHFrame->AddFrame(
        fZoomPlusButton.get(),
        new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 5, 2, 2,
                          2));
    fZoomMinusButton = std::make_unique<TGTextButton>(
        fHFrame.get(), "Zoom &Backward", M_ZOOM_MINUS);
    fZoomMinusButton->Associate(this);
    fZoomMinusButton->SetToolTipText("Zoom backward");
    fHFrame->AddFrame(
        fZoomMinusButton.get(),
        new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 5, 2, 2,
                          2));
    AddFrame(fHFrame.get(),
             new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 5,
                               5, 5, 5));

    EnableButtons(kFALSE);
  }
  ~VisFrame() = default;

  /**
   * Creates TGeoManager, initializes members
   */
  void Init() {
    new TGeoManager("Visualization", "BioDynaMo");

    // Set number of segments for approximating circles in drawing.
    // Keep it low for better performance.
    gGeoManager->SetNsegments(8);

    mat_solid_ = new TGeoMaterial("Solid", .938, 1., 10000.);
    med_solid_ = new TGeoMedium("Solid", 1, mat_solid_);

    // Another way to make top volume for world. In this way it will be
    // unbounded.
    top_ = new TGeoVolumeAssembly("WORLD");

    gGeoManager->SetTopVolume(top_);

    // number of visualized nodes inside one volume. If you exceed this number,
    // ROOT will draw nothing.
    SetMaxVizNodes(100000);
    gGeoManager->SetMaxVisNodes(max_viz_nodes_);

    is_geometry_closed_ = kFALSE;
    init_ = kTRUE;
    cell_count = 1;
    updateCount = 0;
    axisShown = kFALSE;
  }

  void EnableButtons(Bool_t flag = kTRUE) {
    EButtonState buttonState = (flag == kTRUE) ? kButtonUp : kButtonDisabled;
    fZoomPlusButton->SetState(buttonState);
    fZoomMinusButton->SetState(buttonState);
  }

  /**
   * Updates GLViewer of TGeoManager according to current state of ECM.
   */
  void Update() {
    std::cout << "In VisFrame Update()! #" << updateCount++ << "\n";
    if (!init_) {
      throw std::runtime_error("Call Init() first!");
    }
    if (is_geometry_closed_) {
      throw std::runtime_error(
          "Geometry is already closed! Don't call Update() after "
          "CloseGeometry()!");
    }
    top_->ClearNodes();
    cell_count = 1;
    auto* sim = Project::GetInstance().GetSimulation();
    auto* rm = sim->GetResourceManager();
    rm->ApplyOnAllElements([&](bdm::SimObject* cells) {
      auto so_name = cells->GetTypeName();
      auto container = new TGeoVolumeAssembly("A");
      this->AddBranch((*cells), container, so_name);
      top_->AddNode(container, top_->GetNdaughters());
    });

    /// TODO: Show axis in the view
    if (!axisShown) {
      // TView* view = viewPad->GetView();
      // view->ShowAxis();
      axisShown = !axisShown;
    }
    fCanvas->SetFillColor(1);
    fCanvas->Clear();
    gGeoManager->CloseGeometry();
    gGeoManager->GetTopVolume()->Draw();
    update_ = kTRUE;
  }

  void Reset() {
    updateCount = 0;
    top_->ClearNodes();
    gGeoManager->GetTopVolume()->Draw();
  }

  /**
   * Setter for this->MaxVizNodes
   * @param number - number of visualizable nodes per volume
   */
  void SetMaxVizNodes(int number) { this->max_viz_nodes_ = number; }

  /**
   * After building the full geometry tree, geometry must be closed.
   * Closing geometry implies checking the geometry validity, fixing shapes with
   * negative parameters (run-time shapes) building the cache manager,
   * voxelizing
   * all volumes, counting the total number of physical nodes and registering
   * the manager class to the browser.
   */
  void CloseGeometry() {
    if (!update_) {
      throw std::runtime_error("Call `gui::VisManager::GetInstance().Update()` first!");
    }
    gGeoManager->CloseGeometry();
    is_geometry_closed_ = kTRUE;
  }

  void SetRedCell(Long_t cellNumber, Bool_t flag) {
    auto fIt = std::find(fRedCells.begin(), fRedCells.end(), cellNumber);
    if (fIt != fRedCells.end()) {
      if (!flag) {
        fRedCells.erase(fIt);
      }
    } else if (flag) {
      fRedCells.push_back(cellNumber);
    }
  }

  virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2) {
    switch (GET_MSG(msg)) {
      case kC_COMMAND:
        switch (GET_SUBMSG(msg)) {
          case kCM_BUTTON:
          case kCM_MENU:
            switch (parm1) {
              case M_ZOOM_PLUS:
                fCanvas->cd();
                fCanvas->GetView()->ZoomView(0, 1.25);
                fCanvas->Modified();
                fCanvas->Update();
                break;
              case M_ZOOM_MINUS:
                fCanvas->cd();
                fCanvas->GetView()->UnzoomView(0, 1.25);
                fCanvas->Modified();
                fCanvas->Update();
                break;
            }       // switch parm1
            break;  // M_MENU
        }       // switch submsg
        break;  // case kC_COMMAND
    }  // switch msg

    return kTRUE;
  }

 private:
  /**
   * Recursively adds sphere and its daughters to the container.
   */
  template <typename SO>
  void AddBranch(SO&& cell, TGeoVolume* container, std::string name) {
    AddCellToVolume(cell, container, name);
    // to be extended for other object
  }

  /**
   * Adds sphere to the volume, its name will be: "S%d", sphereID
   */
  template <typename SO>
  void AddCellToVolume(SO&& cell, TGeoVolume* container, std::string so_name) {
    std::string name = so_name + std::to_string(cell_count);
    auto radius = cell.GetDiameter() / 2;
    auto massLocation = cell.GetPosition();
    auto x = massLocation[0];
    auto y = massLocation[1];
    auto z = massLocation[2];
    auto position = new TGeoTranslation(x, y, z);
    auto volume = gGeoManager->MakeSphere(name.c_str(), med_solid_, 0., radius);
    /// Only the first rendered cell will be red, the rest blue
    auto fIt = std::find(fRedCells.begin(), fRedCells.end(), cell_count);
    if (fIt == fRedCells.end()) {
      volume->SetLineColor(kBlue);
    } else {
      volume->SetLineColor(kRed);
    }
    cell_count++;
    container->AddNode(volume, container->GetNdaughters(), position);
  }

  ///----Visualization-Canvas----///
  TRootEmbeddedCanvas* fEmbeddedCanvas;
  TCanvas* fCanvas;

  std::unique_ptr<TGHorizontalFrame> fHFrame;
  std::unique_ptr<TGTextButton> fZoomPlusButton;
  std::unique_ptr<TGTextButton> fZoomMinusButton;
  std::unique_ptr<TGLayoutHints> fL1;
  std::vector<Long_t> fRedCells;

  /// Top volumes for TGeo and TEve (world)
  TGeoVolume* top_;
  TEveGeoTopNode* eve_top_;

  /// Eve materials and medium
  TGeoMaterial* mat_empty_space_;
  TGeoMaterial* mat_solid_;
  TGeoMedium* med_empty_space_;
  TGeoMedium* med_solid_;

  // just to ensure that methods were called in correct order
  Bool_t init_;                // true if init_ was called
  Bool_t update_;              // true if update was called
  Bool_t is_geometry_closed_;  // true if geometry is already closed

  /// Max visualized shapes per volume.
  u_int32_t max_viz_nodes_;
  u_int32_t cell_count;
  u_int32_t updateCount;
  Bool_t axisShown;
};

}  // namespace gui

#endif  // GUI_VISUALIZATION_FRAME_H_