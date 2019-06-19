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
  TGVerticalFrame        *fV1;
  TGLabel                *fLtitle; 

  ///-----Simulation-Entities-----///
  TGLabel                *fLentities;
  TGButton               *fBcell;       // Cell

  ///----------Modules------------///
  TGLabel                *fLmodules;
  TGButton               *fBgrowth;     // Growth Module
  TGButton               *fBchemotaxis; // Chemotaxis
  TGButton               *fBsubstance;  // Substance Secretion

  ///----------General------------///
  TGLabel                *fLgeneral; 
  TGButton               *fBvariable;   // Variable
  TGButton               *fBfunction;   // Function
  TGButton               *fBformula;    // Formula

  /// Layout hints
  TGLayoutHints          *fL1;
  TGLayoutHints          *fL2;
  TGLayoutHints          *fL3;
  TGLayoutHints          *fL4;
  TGLayoutHints          *fL5;
  TGLayoutHints          *fL6;
  TGLayoutHints          *fL7;
  TGLayoutHints          *fL8;

  std::vector<ModelTab*> fModelTabs;
  ModelTab              *fCurTab;

  TGWindow*             fButtonHandler;
};


} // namespace gui

#endif