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

#include "gui/constants.h"
#include "gui/view/visualization_frame.h"

namespace gui {

VisFrame::VisFrame(const TGWindow* p) 
    : TGCompositeFrame(p, 600, 400) {
  fL1 = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsRight | kLHintsExpandX | kLHintsExpandY, 5, 2, 2, 2);
  fEmbeddedCanvas = new TRootEmbeddedCanvas("fEmbeddedCanvas", this, 580, 360);
  AddFrame(fEmbeddedCanvas, fL1.get());
  fEmbeddedCanvas->GetCanvas()->SetBorderMode(0);
  fCanvas = fEmbeddedCanvas->GetCanvas();
  fCanvas->SetFillColor(1);

  fHFrame = std::make_unique<TGHorizontalFrame>(this, 0, 0, 0);
  // Create Zoom Buttons
  fZoomPlusButton = std::make_unique<TGTextButton>(fHFrame.get(), "&Zoom Forward", M_ZOOM_PLUS);
  fZoomPlusButton->Associate(this);
  fZoomPlusButton->SetToolTipText("Zoom forward");
  fHFrame->AddFrame(fZoomPlusButton.get(), new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 5, 2, 2, 2));
  fZoomMinusButton = std::make_unique<TGTextButton>(fHFrame.get(), "Zoom &Backward", M_ZOOM_MINUS);
  fZoomMinusButton->Associate(this);
  fZoomMinusButton->SetToolTipText("Zoom backward");
  fHFrame->AddFrame(fZoomMinusButton.get(), new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 5, 2, 2, 2));
  AddFrame(fHFrame.get(), new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
}

VisFrame::~VisFrame() {}

Bool_t VisFrame::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
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
               } // switch parm1
               break; // M_MENU
            } // switch submsg
            break; // case kC_COMMAND
   } // switch msg

   return kTRUE;
}

}  // namespace gui