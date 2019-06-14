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

#ifndef GUI_MODEL_TAB_H_
#define GUI_MODEL_TAB_H_

#include <KeySymbols.h>
#include <TEnv.h>
#include <TROOT.h>
#include <TRint.h>
#include <TStyle.h>
#include <TVirtualX.h>

#include <TF1.h>
#include <TFile.h>
#include <TFrame.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGTab.h>
#include <TH1.h>
#include <TList.h>
#include <TTree.h>

namespace gui {

class ModelTab : public TGCompositeFrame {

 public:
  // Constructor & destructor
  ModelTab(const TGWindow* p, TGWindow* buttonHandler,
           const char* modelName = 0);
  virtual ~ModelTab() = default;

  void ShowTab(const char* elementName);
  void SetModelName(const char* modelName) { fModelName = modelName; }
  std::string GetModelName() { return fModelName; }

 private:
  std::string       fModelName;
  TGTab*            fTab;
  TGCompositeFrame  *fCurFrame;
  TGLabel           *fInitialLabel;
};

}  // namespace gui

#endif