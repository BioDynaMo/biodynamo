// Original Author: Bertrand Bellenot   22/08/02
// Modified by: Lukasz Stempniewicz 21/08/19

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


#include "gui/view/button_project.h"
#include "gui/constants.h"

namespace gui {

////////////////////////////////////////////////////////////////////////////////
/// Create ButtonProjectFrame object, with TGWindow parent *p.
///
/// buttonHandler = pointer to button handler TGWindow
/// newProjectId = id of M_FILE_NEWPROJECT
/// loadProjectId = id of M_FILE_OPENPROJECT

ButtonProjectFrame::ButtonProjectFrame(const TGWindow* p, TGWindow* buttonHandler, Int_t newProjectId, Int_t loadProjectId) : TGCompositeFrame(p, 100, 100, kVerticalFrame)
{
   // Create Layout hints
   fButtonLayout = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 2, 2, 2);

   // Create Model Buttons
   fCreateProjectButton = std::make_unique<TGTextButton>(this, "&New Project", newProjectId);
   fCreateProjectButton->Associate(buttonHandler);
   fCreateProjectButton->SetToolTipText("Creates new GUI Project");
   AddFrame(fCreateProjectButton.get(), fButtonLayout.get());
   fLoadProjectButton = std::make_unique<TGTextButton>(this, "&Load Project", loadProjectId);
   fLoadProjectButton->Associate(buttonHandler);
   fLoadProjectButton->SetToolTipText("Loads in an already created project");
   AddFrame(fLoadProjectButton.get(), fButtonLayout.get());

   fCreateProjectButton->Resize(150,GetDefaultHeight());
   fLoadProjectButton->Resize(150,GetDefaultHeight());

   SetState(M_ALL_ACTIVE);
}

////////////////////////////////////////////////////////////////////////////////
/// Destroy ButtonProjectFrame object. Delete all created widgets

ButtonProjectFrame::~ButtonProjectFrame()
{
   //delete fButtonLayout;
   //delete fCreateProjectButton;
   //delete fLoadProjectButton;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the state of the ButtonProjectFrame. This sets the state of
/// the TGButton's in this frame.

void ButtonProjectFrame::SetState(int state)
{
   switch (state) {
      case M_ALL_ACTIVE:
         fCreateProjectButton->SetState(kButtonUp);
         fLoadProjectButton->SetState(kButtonUp);
         break;

      case M_NONE_ACTIVE:
         fCreateProjectButton->SetState(kButtonDisabled);
         fLoadProjectButton->SetState(kButtonDisabled);
         break;

   } // switch state
   // make sure window gets updated...
   gClient->HandleInput();
}

}  // namespace gui

