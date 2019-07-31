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

#include "gui/view/inspector.h"
#include "gui/view/log.h"

namespace gui {

class ModelTabs : public TGCompositeFrame {
 public:
  ModelTabs(const TGWindow* p);
  virtual ~ModelTabs() {}

  void   ShowElementTab(const char* name);
  void   SetModelName(const char* name) { fModelName.assign(name); }
  void   ClearAllTabs();

 private:
  Int_t  GetElementTabIdx(const char* name);
  TGTab* GetElementTab(const char* name);
  void   UpdateTabPriority(const char* name);
  void   OverwriteLowestPriorityTab(const char* name);
  void   ShowElement(const char* name);
  void   PrintTabNames();
  void   UpdateTabContents(TGTab* tab, const char* elementName);

  std::unique_ptr<TGTab>  fTabTL;  // top-left tab
  std::unique_ptr<TGTab>  fTabTR;  // top-right tab
  std::unique_ptr<TGTab>  fTabBL;  // bottom-left tab
  std::unique_ptr<TGTab>  fTabBR;  // bottom-right tab

  /// Above 4 tabs will be in this vector
  std::vector<TGTab*>     fTabs;

  std::vector<Inspector*> fInspectors;
  std::string             fModelName;
  Bool_t                  IsFrameInit;

  std::unique_ptr<TGLayoutHints> fL1, fL2, fL3, fL4;
};

}  // namespace gui

#endif // GUI_MODEL_TABS_H_