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

#ifndef GUI_MODEL_ELEMENT_H_
#define GUI_MODEL_ELEMENT_H_

#include <memory>
#include <string>
#include <vector>

#include <RQ_OBJECT.h>
#include <TApplication.h>
#include <TClass.h>
#include <TEnv.h>
#include <TFile.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TVirtualX.h>
#include "TObject.h"
#include "TString.h"
#include "model.h"

class ModelElement : public TObject {
 public:
  ModelElement(ModelElement* parent, std::string& name, Model modelRef) {}

  // have template type?
  ModelElement* SearchChildren(const char* elementName) {
    TObjLink* lnk = fChildren->FirstLink();
    while (lnk) {
      ModelElement* tmp = (ModelElement*)lnk->GetObject();
      if (tmp->fName.compare(elementName) == 0)
        return tmp;
      lnk = lnk->Next();
    }
    return nullptr;
  }
  virtual std::string GenerateCode() = 0;
  virtual void        Save() = 0;
  
  std::string        fName;
  std::string        fPathName;

 private:
  ModelElement*      fParent;
  Model              *gModel;
  TList              *fChildren;  
};

#endif  // GUI_MODEL_ELEMENT_H_