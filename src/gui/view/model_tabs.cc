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

#include "gui/view/model_tabs.h"
#include "gui/controller/visualization_manager.h"

namespace gui {

ModelTabs::ModelTabs(const TGWindow* p) 
       : TGCompositeFrame(p, 1000, 1000) {
  SetLayoutManager(new TGMatrixLayout(this, 2, 2));
  fTabTL = std::make_unique<TGTab>(this, 100, 100);
  fTabTR = std::make_unique<TGTab>(this, 100, 100);
  fTabBL = std::make_unique<TGTab>(this, 100, 100);
  fTabBR = std::make_unique<TGTab>(this, 100, 100);

  fL1 = std::make_unique<TGLayoutHints>(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY , 2, 2, 2, 2);
  fL2 = std::make_unique<TGLayoutHints>(kLHintsRight | kLHintsTop | kLHintsExpandX | kLHintsExpandY , 2, 2, 2, 2);
  fL3 = std::make_unique<TGLayoutHints>(kLHintsLeft | kLHintsBottom | kLHintsExpandX | kLHintsExpandY , 2, 2, 2, 2);
  fL4 = std::make_unique<TGLayoutHints>(kLHintsRight | kLHintsBottom | kLHintsExpandX | kLHintsExpandY , 2, 2, 2, 2);

  IsFrameInit = kFALSE;

  fTabs.push_back(fTabTL.get());
  fTabs.push_back(fTabTR.get());
  fTabs.push_back(fTabBL.get());
  fTabs.push_back(fTabBR.get());
}

/// Used for debugging
void ModelTabs::PrintTabNames(){
  Size_t tabSize = fTabs.size();
  std::string tmpName;
  for(Int_t i = 0; i < tabSize; i++) {
     tmpName.assign(fTabs[i]->GetName());
     Log::Debug("Tab #", i, " - ", tmpName);
  }
}

Int_t ModelTabs::GetElementTabIdx(const char* name) {
  Size_t tabSize = fTabs.size();
  for(Int_t i = 0; i < tabSize; i++) {
     if(fTabs[i]->SetTab(name)) {
       return i;
     }  
  }
  return -1;
}

TGTab* ModelTabs::GetElementTab(const char* name) {
  Int_t idx = GetElementTabIdx(name);
  if(idx > -1) {
    return fTabs[idx];
  }
  return nullptr;
}

void ModelTabs::UpdateTabPriority(const char* name) {
  Log::Debug("Updating tab priority for:", name);
  PrintTabNames();
  Int_t idx = GetElementTabIdx(name);
  if(idx > -1) {
    Int_t lastIdx = fTabs.size() - 1;
    if(idx < lastIdx) {
      fTabs.push_back(fTabs[idx]);
      fTabs.erase(fTabs.begin() + idx);
    }
  } else {
    Log::Error("Could not find element `", name, "` when updating tab priority!");
  }
  Log::Debug("DONE Updating tab priority for:", name);
  PrintTabNames();
}

void ModelTabs::UpdateTabContents(TGTab* tab, const char* elementName) {
  Inspector* inspector = 
    new Inspector(tab->GetCurrentContainer(), fModelName.c_str(), elementName);
  fInspectors.push_back(inspector);

  std::string name(elementName);
  Pixel_t pxl;
  if(name.find("Cell") != std::string::npos) {
    gClient->GetColorByName("#ffa3a3", pxl);
  } else {
    gClient->GetColorByName("#00ff00", pxl);
  }

  tab->GetCurrentContainer()->ChangeSubframesBackground(pxl);
}

void ModelTabs::OverwriteLowestPriorityTab(const char* name) {
  TGTab* lowestTab = fTabs[0];
  lowestTab->RemoveTab();
  lowestTab->AddTab(name);
  UpdateTabContents(lowestTab, name);
  //lowestTab->Print();
  
  lowestTab->SetTab(name);
  lowestTab->MapSubwindows();
  lowestTab->Resize();  
  lowestTab->MapWindow();
  MapSubwindows();
  Resize();  
  MapWindow();
}

void ModelTabs::ShowElement(const char* name) {
  TGTab* elementTab = GetElementTab(name);
  if(elementTab == nullptr) {
    Log::Debug("Element tab is nullptr, overwriting lowest priority");
    OverwriteLowestPriorityTab(name);
  }
  UpdateTabPriority(name);
}

void ModelTabs::ShowElementTab(const char* name) {
  Log::Debug("Showing element tab:", name);
  if(!IsFrameInit) {
    IsFrameInit = kTRUE;
    AddFrame(fTabTL.get(), fL1.get());
    AddFrame(fTabTR.get(), fL2.get());
    AddFrame(fTabBL.get(), fL3.get());
    AddFrame(fTabBR.get(), fL4.get());
  }
  ShowElement(name);
}

void ModelTabs::ClearAllTabs() {
  //IsFrameInit = kFALSE;
  RemoveFrame(fTabTL.get());
  RemoveFrame(fTabTR.get());
  RemoveFrame(fTabBL.get());
  RemoveFrame(fTabBR.get());
  fTabs.clear();
  MapSubwindows();
  Resize();  
  MapWindow();
}

}