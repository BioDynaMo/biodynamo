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

#ifndef GUI_MODEL_FRAME_H_
#define GUI_MODEL_FRAME_H_

#include <TROOT.h>
#include <TStyle.h>
#include <TRint.h>
#include <TVirtualX.h>
#include <TEnv.h>
#include <KeySymbols.h>

#include <TFile.h>
#include <TTree.h>
#include <TFrame.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TList.h>
#include <TH1.h>
#include <TF1.h>

#include <unordered_map> 

#include "gui/constants.h"
#include "gui/view/model_tab.h"

namespace gui {

class ModelFrame : public TGCompositeFrame {

 public:
  // Constructor & destructor
  ModelFrame(const TGWindow* p, TGWindow* buttonHandler);
  virtual ~ModelFrame();
  
  void ShowModelElement(const char* modelName, const char* modelElement);
  void ShowEmptyTab();
  void EnableButtons(Int_t state);
  void SwitchModelTab(ModelTab *t);

  Bool_t SwitchModelTab(const char* modelName, const char* modelElement=0);

 private:
  std::unique_ptr<TGVerticalFrame>        fV1;
  std::unique_ptr<TGLabel>                fLtitle; 

  ///-----Simulation-Entities-----///
  std::unique_ptr<TGLabel>                fLentities;
  std::unique_ptr<TGButton>               fBcell;       // Cell

  ///----------Modules------------///
  std::unique_ptr<TGLabel>                fLmodules;
  std::unique_ptr<TGButton>               fBgrowth;     // Growth Module
  std::unique_ptr<TGButton>               fBchemotaxis; // Chemotaxis
  std::unique_ptr<TGButton>               fBsubstance;  // Substance Secretion

  ///----------General------------///
  std::unique_ptr<TGLabel>                fLgeneral; 
  std::unique_ptr<TGButton>               fBvariable;   // Variable
  std::unique_ptr<TGButton>               fBfunction;   // Function
  std::unique_ptr<TGButton>               fBformula;    // Formula

  /// Layout hints
  std::unique_ptr<TGLayoutHints>        fL1;
  std::unique_ptr<TGLayoutHints>        fL2;
  std::unique_ptr<TGLayoutHints>        fL3;
  std::unique_ptr<TGLayoutHints>        fL4;
  std::unique_ptr<TGLayoutHints>        fL5;
  std::unique_ptr<TGLayoutHints>        fL6;
  std::unique_ptr<TGLayoutHints>        fL7;
  std::unique_ptr<TGLayoutHints>        fL8;

  std::vector<ModelTab*> fModelTabs;
  ModelTab               *fCurTab;

  TGWindow*             fButtonHandler;
};


} // namespace gui

#endif