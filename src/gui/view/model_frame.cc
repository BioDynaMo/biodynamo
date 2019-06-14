// Author: Lukasz Stempniewicz 25/05/19

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

#include "model_frame.h"
#include <TGButton.h>
#include "log.h"

namespace gui {

ModelFrame::ModelFrame(const TGWindow* p, TGWindow* buttonHandler)
    : TGCompositeFrame(p, 200, 200, kHorizontalFrame) {
  fL1 =
      new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 2, 2, 2);
  fL2 = 
      new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 5, 2, 2, 2);
  fL3 =
      new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 5, 2, 2, 2);

  fV1 = new TGVerticalFrame(this, 50, 100, 0);

  ///------Title-------///
  // Pixel_t col;
  // gClient->GetColorByName("green", col);
  TString fontname("-*-times-bold-r-*-*-16-*-*-*-*-*-*-*");
  fLtitle = new TGLabel(fV1, "Model Elements");
  fLtitle->SetTextFont(fontname.Data());
  // fLtitle->SetBackgroundColor(col);
  fV1->AddFrame(fLtitle, fL1);

  ///-----Simulation-Entities-----///
  fLentities = new TGLabel(fV1, "Simulation Entities");
  fV1->AddFrame(fLentities, fL1);

  fBcell = new TGTextButton(fV1, "Cell", M_ENTITY_CELL);
  fBcell->Associate(buttonHandler);
  fBcell->SetToolTipText("Create new custom Cell object");
  fV1->AddFrame(fBcell, fL1);

  ///----------Modules------------///
  fLmodules = new TGLabel(fV1, "Modules");
  fV1->AddFrame(fLmodules, fL1);

  fBgrowth = new TGTextButton(fV1, "Growth", M_MODULE_GROWTH);
  fBgrowth->Associate(buttonHandler);
  fBgrowth->SetToolTipText("Create new Growth Module");
  fV1->AddFrame(fBgrowth, fL1);

  fBchemotaxis = new TGTextButton(fV1, "Chemotaxis", M_MODULE_CHEMOTAXIS);
  fBchemotaxis->Associate(buttonHandler);
  fBchemotaxis->SetToolTipText("Create new Chemotaxis Module");
  fV1->AddFrame(fBchemotaxis, fL1);

  fBsubstance =
      new TGTextButton(fV1, "Substance Secretion", M_MODULE_SUBSTANCE);
  fBsubstance->Associate(buttonHandler);
  fBsubstance->SetToolTipText("Create new Substance Secretion Module");
  fV1->AddFrame(fBsubstance, fL1);

  ///----------General------------///
  fLgeneral = new TGLabel(fV1, "General");
  fV1->AddFrame(fLgeneral, fL1);

  fBvariable = new TGTextButton(fV1, "Variable", M_GENERAL_VARIABLE);
  fBvariable->Associate(buttonHandler);
  fBvariable->SetToolTipText("Create new general variable");

  fV1->AddFrame(fBvariable, fL1);

  fBfunction = new TGTextButton(fV1, "Function", M_GENERAL_FUNCTION);
  fBfunction->Associate(buttonHandler);
  fBfunction->SetToolTipText("Create new general function");

  fV1->AddFrame(fBfunction, fL1);

  fBformula = new TGTextButton(fV1, "Formula", M_GENERAL_FORMULA);
  fBformula->Associate(buttonHandler);
  fBformula->SetToolTipText("Create new general formula");
  fV1->AddFrame(fBformula, fL1);

  AddFrame(fV1, fL3);

  fCurTab = new ModelTab(this, buttonHandler);

  fModelTabs.emplace_back(fCurTab);

  AddFrame(fCurTab, fL2);

  fButtonHandler = buttonHandler;
}

void ModelFrame::EnableButtons(Int_t state) {
  Pixel_t col;
  EButtonState buttonState;
  if (state == M_ALL_ACTIVE) {
    buttonState = kButtonUp;
    gClient->GetColorByName("green", col);
  } else {
    buttonState = kButtonDisabled;
    gClient->GetColorByName("gray", col);
  }
  fLtitle->SetBackgroundColor(col);
  fClient->NeedRedraw(fLtitle);

  fBcell->SetState(buttonState);
  fBgrowth->SetState(buttonState);
  fBchemotaxis->SetState(buttonState);
  fBsubstance->SetState(buttonState);
  fBvariable->SetState(buttonState);
  fBfunction->SetState(buttonState);
  fBformula->SetState(buttonState);
}

ModelFrame::~ModelFrame() {
  delete fLentities;
  delete fBcell;
  delete fLmodules;
  delete fBgrowth;
  delete fBchemotaxis;
  delete fBsubstance;
  delete fLgeneral;
  delete fBvariable;
  delete fBfunction;
  delete fBformula;
  delete fLtitle;
  delete fV1;
}

void ModelFrame::SwitchModelTab(ModelTab* t) {
  HideFrame(fCurTab);
  ShowFrame(t);
  fCurTab = t;
  Log::Info("Switching current tab!");
}

Bool_t ModelFrame::SwitchModelTab(const char* modelName,
                                  const char* modelElement) {
  for (ModelTab* t : fModelTabs) {
    if (t->GetModelName().compare(modelName) == 0) {
      if (t != fCurTab) {
        SwitchModelTab(t);
      }
      if (modelElement != 0)
        fCurTab->ShowTab(modelElement);
      return kTRUE;
    }
  }
  Log::Warning("No such modeltab: ", modelName);
  return kFALSE;
}

void ModelFrame::ShowModelElement(const char* modelName,
                                  const char* modelElement) {
  // First Tab
  if (fCurTab->GetModelName().empty()) {
    Log::Info("Current Tab is empty! Setting up to first model tab.");
    fCurTab->ShowTab(modelElement);
    fCurTab->SetModelName(modelName);
  } else {
    Log::Info("Setting up subsequent tabs.");
    if (SwitchModelTab(modelName, modelElement))
      return;
    /// ModelTab doesn't exist, need to create new one
    ModelTab* t = new ModelTab(this, fButtonHandler, modelName);
    AddFrame(t, fL2);

    fModelTabs.emplace_back(t);
    SwitchModelTab(t);
    fCurTab->ShowTab(modelElement);
  }
  MapWindow();
  MapSubwindows();

  fCurTab->Resize(10000, 10000);
}

}  // namespace gui
