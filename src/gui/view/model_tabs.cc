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

ModelTabs::ModelTabs(const TGWindow* p) {
  fFrame = std::make_unique<TGCompositeFrame>(p, 1000, 1000);
  fFrame->SetLayoutManager(new TGTileLayout(fFrame.get()));
  //fFrame->Connect("ProcessedEvent(Event_t*)", "ModelTabs", this,
  //                 "HandleMouseWheel(Event_t*)");
  IsFrameInit = kFALSE;
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

void ModelTabs::SetTabColor(const char* name) {
  Size_t tabSize = fTabs.size();
  Pixel_t grey, darkgrey;
  gClient->GetColorByName("#e8e8e8", grey);
  gClient->GetColorByName("#c0c0c0", darkgrey);

  for(Int_t i = 0; i < tabSize; i++) {
     if(fTabs[i]->SetTab(name)) {
       fTabs[i]->GetCurrentContainer()->ChangeSubframesBackground(darkgrey);
     } else {
       fTabs[i]->GetCurrentContainer()->ChangeSubframesBackground(grey);
     }
  }
}

void ModelTabs::UpdateTabContents(TGTab* tab, const char* elementName) {
  Inspector* inspector = 
    new Inspector(tab->GetCurrentContainer(), fModelName.c_str(), elementName);
  fInspectors.push_back(inspector);
}

void ModelTabs::AddATab(const char* name) {
  TGFrame* newFrame = new TGFrame(fFrame.get(), 300, 300);
  TGTab* newTab = new TGTab(newFrame, 290, 300);
  newTab->AddTab(name);
  newTab->SetTab(name);
  UpdateTabContents(newTab, name);
  fTabs.push_back(newTab);
  fTabFrames.push_back(newFrame);
  //newFrame->Move(newFrame->GetX() + 5, newFrame->GetY() + 5);
  fCanvas->AddFrame(newFrame, new TGLayoutHints(kLHintsRight, 5, 5, 5, 5));
  newFrame->DrawBorder();

  newTab->MapSubwindows();
  newTab->Resize();  
  newTab->MapWindow();
  fFrame->MapSubwindows();
  fFrame->Resize();  
  fFrame->MapWindow();
}

Bool_t ModelTabs::CheckAllSecretionBoxes() {
  Bool_t OneSecretion = kFALSE;
  for(auto* inspect : fInspectors) {
    if(inspect->CheckSecretionBox()) {
      OneSecretion = kTRUE;
    }
  }
  return OneSecretion;
}


void ModelTabs::HandleMouseWheel(Event_t *event)
{
   // Handle mouse wheel to scroll.

   if (event->fType != kButtonPress && event->fType != kButtonRelease)
      return;

   Int_t page = 0;
   if (event->fCode == kButton4 || event->fCode == kButton5) {
      if (!fCanvas) return;
      if (fCanvas->GetContainer()->GetHeight())
         page = Int_t(Float_t(fCanvas->GetViewPort()->GetHeight() *
                              fCanvas->GetViewPort()->GetHeight()) /
                              fCanvas->GetContainer()->GetHeight());
   }

   if (event->fCode == kButton4) {
      //scroll up
      Int_t newpos = fCanvas->GetVsbPosition() - page;
      if (newpos < 0) newpos = 0;
      fCanvas->SetVsbPosition(newpos);
   }
   if (event->fCode == kButton5) {
      // scroll down
      Int_t newpos = fCanvas->GetVsbPosition() + page;
      fCanvas->SetVsbPosition(newpos);
   }
}

void ModelTabs::ShowElement(const char* name) {
  TGTab* elementTab = GetElementTab(name);
  if(elementTab == nullptr) {
    Log::Debug("Element tab is nullptr, creating new tab for `", name, "`");
    AddATab(name);
    fCanvas->SetVsbPosition(100000);
  }
  SetTabColor(name);
}

void ModelTabs::ShowElementTab(const char* name) {
  Log::Debug("Showing element tab:", name);
  if(!IsFrameInit) {
    IsFrameInit = kTRUE;
  }
  ShowElement(name);
}

void ModelTabs::ClearAllTabs() {
  //Size_t tabFramesSize = fTabFrames.size();
  //for(Int_t i = 0; i < tabFramesSize; i++) {
  //  fFrame->RemoveFrame(fTabFrames[i]);
  //}
  fTabs.clear();
  fTabFrames.clear();
  fFrame->SetCleanup(kDeepCleanup);
  fFrame->Cleanup();
  fFrame->MapSubwindows();
  fFrame->Resize();  
  fFrame->MapWindow();
  //fCanvas->DestroySubwindows(); // causes insane crash
  fCanvas->ClearViewPort();
}

}  // namespace gui