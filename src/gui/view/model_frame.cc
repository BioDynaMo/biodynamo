// Author: Lukasz Stempniewicz 21/08/19

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

namespace gui {

ModelFrame::ModelFrame(const TGWindow* p, TGWindow* buttonHandler)
    : TGCompositeFrame(p, 200, 200, kHorizontalFrame) {
  fL1 = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 2, 2);
  fL2 = std::make_unique<TGLayoutHints>(kLHintsExpandX | kLHintsExpandY, 5, 5, 2, 2);
  fL3 = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsLeft | kLHintsExpandY, 5, 5, 2, 2);
  fL4 = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsRight | kLHintsExpandY | kLHintsExpandX, 5, 5, 2, 2);

  fV1 = std::make_unique<TGVerticalFrame>(this, 50, 100, 0);

  ///------Title-------///
  TString fontname("-*-times-bold-r-*-*-16-*-*-*-*-*-*-*");
  fLtitle = std::make_unique<TGLabel>(fV1.get(), "Model Elements");
  fLtitle->SetTextFont(fontname.Data());
  fV1->AddFrame(fLtitle.get(), fL1.get());

  ///-----Simulation-Entities-----///
  fLentities = std::make_unique<TGLabel>(fV1.get(), "Simulation Entities");
  fV1->AddFrame(fLentities.get(), fL1.get());

  fBcell = std::make_unique<TGTextButton>(fV1.get(), "Cell", M_ENTITY_CELL);
  fBcell->Associate(buttonHandler);
  fBcell->SetToolTipText("Create new Cell Body");
  fV1->AddFrame(fBcell.get(), fL1.get());

  ///----------Modules------------///
  fLmodules = std::make_unique<TGLabel>(fV1.get(), "Modules");
  fV1->AddFrame(fLmodules.get(), fL1.get());

  fBgrowth = std::make_unique<TGTextButton>(fV1.get(), "Growth", M_MODULE_GROWTH);
  fBgrowth->Associate(buttonHandler);
  fBgrowth->SetToolTipText("Create new Growth Module");
  fV1->AddFrame(fBgrowth.get(), fL1.get());

  fBchemotaxis = std::make_unique<TGTextButton>(fV1.get(), "Chemotaxis", M_MODULE_CHEMOTAXIS);
  fBchemotaxis->Associate(buttonHandler);
  fBchemotaxis->SetToolTipText("Create new Chemotaxis Module");
  fV1->AddFrame(fBchemotaxis.get(), fL1.get());

  fBsubstance = std::make_unique<TGTextButton>(fV1.get(), "Substance Secretion", M_MODULE_SUBSTANCE);
  fBsubstance->Associate(buttonHandler);
  fBsubstance->SetToolTipText("Create new Substance Secretion Module");
  fV1->AddFrame(fBsubstance.get(), fL1.get());

  ///----------General------------///
  fLgeneral = std::make_unique<TGLabel>(fV1.get(), "General");
  fV1->AddFrame(fLgeneral.get(), fL1.get());

  fBvariable = std::make_unique<TGTextButton>(fV1.get(), "Variable", M_GENERAL_VARIABLE);
  fBvariable->Associate(buttonHandler);
  fBvariable->SetToolTipText("Create new general variable");

  fV1->AddFrame(fBvariable.get(), fL1.get());

  fBfunction = std::make_unique<TGTextButton>(fV1.get(), "Function", M_GENERAL_FUNCTION);
  fBfunction->Associate(buttonHandler);
  fBfunction->SetToolTipText("Create new general function");

  fV1->AddFrame(fBfunction.get(), fL1.get());

  fBformula = std::make_unique<TGTextButton>(fV1.get(), "Formula", M_GENERAL_FORMULA);
  fBformula->Associate(buttonHandler);
  fBformula->SetToolTipText("Create new general formula");
  fV1->AddFrame(fBformula.get(), fL1.get());

  AddFrame(fV1.get(), fL3.get());

  fCanvasWindow = std::make_unique<TGCanvas>(this, 310, 300);
  fTabManager = std::make_unique<ModelTabs>(fCanvasWindow->GetViewPort());
  fTabManager->SetCanvas(fCanvasWindow.get());
  fCanvasWindow->SetContainer(fTabManager->GetFrame());

  fTabManager->GetFrame()->SetCleanup(kDeepCleanup);
  AddFrame(fCanvasWindow.get(), new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,
                                                    2, 2, 2, 2));

  VisFrame* visFrame = VisManager::GetInstance().Init(this);

  AddFrame(visFrame, fL4.get());

  fButtonHandler = buttonHandler;

  MapSubwindows();
  Resize();
  MapWindow();
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
  fClient->NeedRedraw(fLtitle.get());

  fBcell->SetState(buttonState);
  fBgrowth->SetState(buttonState);
  fBchemotaxis->SetState(buttonState);
  fBsubstance->SetState(buttonState);
  fBvariable->SetState(buttonState);
  fBfunction->SetState(buttonState);
  fBformula->SetState(buttonState);
}

ModelFrame::~ModelFrame() {}

Bool_t ModelFrame::CheckAllSecretionBoxes() {
  return fTabManager->CheckAllSecretionBoxes();
}

void ModelFrame::ShowModelElement(const char* modelName,
                                  const char* modelElement) {
  fTabManager->SetModelName(modelName);
  fTabManager->ShowElementTab(modelElement);
  
  MapWindow();
  MapSubwindows();
  Resize();
  fTabManager->GetFrame()->MapSubwindows();
  Resize();
  VisManager::GetInstance().RedrawVisFrame();
}

void ModelFrame::ClearTabs() {
  fTabManager->ClearAllTabs();
}

}  // namespace gui
