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

#include "model_tab.h"
#include "log.h"

namespace gui {

ModelTab::ModelTab(const TGWindow* p, TGWindow* buttonHandler, const char* modelName) : TGCompositeFrame(p, 200, 200) {
  fTab = nullptr;
  if(modelName != 0)
    fModelName = modelName;
  fInitialLabel = new TGLabel(this, "Please create or load project and model.");
  TString fontname("-*-times-*-r-*-*-20-*-*-*-*-*-*-*");
  fInitialLabel->SetTextColor(0xbdbdbd);
  fInitialLabel->SetTextFont(fontname.Data());
  AddFrame(fInitialLabel, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | kLHintsExpandX | kLHintsExpandY , 5, 5, 5, 5));
}

void ModelTab::ShowTab(const char* elementName) {
  if(fInitialLabel) {
    RemoveFrame(fInitialLabel);
    //delete fInitialLabel;
  }
  if(fTab == nullptr) {
    fTab = new TGTab(this, 300, 300);
    AddFrame(fTab, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | kLHintsExpandX | kLHintsExpandY , 2, 2, 2, 2));
  }
  if(fTab->SetTab(elementName) == kFALSE ) {
    fCurFrame = fTab->AddTab(elementName);
    fTab->SetTab(elementName);
  } else {
    fCurFrame = fTab->GetTabContainer(elementName);
  }
  Resize(1000, 1000);
}

} // namespace gui