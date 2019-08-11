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
#ifndef GUI_MODEL_TABS_H_
#define GUI_MODEL_TABS_H_

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <TGCanvas.h>

#include "gui/view/inspector.h"
#include "gui/view/log.h"

namespace gui {

class ModelTabs {
 public:
  // Constructor & destructor
  ModelTabs(const TGWindow* p);
  virtual ~ModelTabs() {}

  void   ShowElementTab(const char* name);
  void   SetModelName(const char* name) { fModelName.assign(name); }
  void   ClearAllTabs();
  void   SetCanvas(TGCanvas *canvas) { fCanvas = canvas; }
  TGFrame *GetFrame() const { return fFrame.get(); }
  void HandleMouseWheel(Event_t *event);
  Bool_t CheckAllSecretionBoxes();

 private:
  Int_t  GetElementTabIdx(const char* name);
  TGTab* GetElementTab(const char* name);
  void   AddATab(const char* name);
  void   ShowElement(const char* name);
  void   SetTabColor(const char* name);
  void   PrintTabNames();
  void   UpdateTabContents(TGTab* tab, const char* elementName);

  std::vector<TGTab*>       fTabs;
  std::vector<TGFrame*>     fTabFrames;

  std::vector<Inspector*>    fInspectors;
  std::string                fModelName;
  TGCanvas*                  fCanvas;
  std::unique_ptr<TGCompositeFrame> fFrame;

  std::unique_ptr<TGLayoutHints> fL1;
  std::unique_ptr<TGLayoutHints> fL2;
  std::unique_ptr<TGLayoutHints> fL3;
  std::unique_ptr<TGLayoutHints> fL4;

  Bool_t                     IsFrameInit;
  
  ClassDefNV(ModelTabs,1)
};

}  // namespace gui

#endif // GUI_MODEL_TABS_H_