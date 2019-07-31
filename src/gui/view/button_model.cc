// Original Author: Bertrand Bellenot   22/08/02
// Modified by: Lukasz Stempniewicz 25/05/19

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
#include "gui/view/button_model.h"

namespace gui {

//______________________________________________________________________________
// ButtonModelFrame
//
// A ButtonModelFrame is a frame containing the ModelCreator buttons.
//______________________________________________________________________________


////////////////////////////////////////////////////////////////////////////////
/// Create ButtonModelFrame object, with TGWindow parent *p.
///
/// buttonHandler = pointer to button handler TGWindow
/// newModelId = id of M_MODEL_NEW
/// simulateModelId = id of M_MODEL_SIMULATE
/// interruptSimId = id of M_INTERRUPT_SIMUL

ButtonModelFrame::ButtonModelFrame(const TGWindow* p, TGWindow* buttonHandler,
                           Int_t newModelId, Int_t simulateModelId,
                           Int_t interruptSimId) :
                           TGCompositeFrame(p, 100, 100, kVerticalFrame)
{
   // Create Layout hints
   fButtonLayout = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 2, 2, 2);

   // Create Model Buttons
   fNewModelButton = std::make_unique<TGTextButton>(this, "Create &New Model", newModelId);
   fNewModelButton->Associate(buttonHandler);
   fNewModelButton->SetToolTipText("Creates new BioDynaMo model");
   AddFrame(fNewModelButton.get(), fButtonLayout.get());
   fStopSimButton = std::make_unique<TGTextButton>(this, "&Interrupt Simulation", interruptSimId);
   fStopSimButton->Associate(buttonHandler);
   fStopSimButton->SetToolTipText("Interrupts the current simulation");
   AddFrame(fStopSimButton.get(), fButtonLayout.get());
   fSimulateModelButton = std::make_unique<TGTextButton>(this, "&Simulate Model", simulateModelId);
   fSimulateModelButton->Associate(buttonHandler);
   fSimulateModelButton->SetToolTipText("Simulates the selected model");
   AddFrame(fSimulateModelButton.get(), fButtonLayout.get());

   fNewModelButton->Resize(150,GetDefaultHeight());
   fStopSimButton->Resize(150,GetDefaultHeight());
   fSimulateModelButton->Resize(150,GetDefaultHeight());

   SetState(M_ALL_ACTIVE);
   fSimulateModelButton->SetState(kButtonDisabled);
   fStopSimButton->SetState(kButtonDisabled);
}

////////////////////////////////////////////////////////////////////////////////
/// Destroy ButtonModelFrame object. Delete all created widgets

ButtonModelFrame::~ButtonModelFrame() {}

////////////////////////////////////////////////////////////////////////////////
/// Set the state of the ButtonModelFrame. This sets the state of
/// the TGButton's in this frame.

void ButtonModelFrame::SetState(int state)
{
   switch (state) {
      case M_ALL_ACTIVE:
         fNewModelButton->SetState(kButtonUp);
         fSimulateModelButton->SetState(kButtonUp);
         fStopSimButton->SetState(kButtonDisabled);
         break;

      case M_NONE_ACTIVE:
         fNewModelButton->SetState(kButtonDisabled);
         fSimulateModelButton->SetState(kButtonDisabled);
         fStopSimButton->SetState(kButtonUp);
         break;

   } // switch state
   // make sure window gets updated...
   gClient->HandleInput();
}

}  // namespace gui