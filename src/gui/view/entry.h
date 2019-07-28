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

#ifndef GUI_ENTRY_H_
#define GUI_ENTRY_H_

#include <KeySymbols.h>
#include <TEnv.h>
#include <TStyle.h>
#include <TVirtualX.h>

#include <TROOT.h>
#include <TClass.h>
#include <TApplication.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTab.h>
#include <TSystem.h>
#include <TGFontDialog.h>

#include "gui/controller/project.h"
#include "gui/model/model_element.h"
#include "core/container/math_array.h"
#include "biodynamo.h"

namespace gui {

enum class EntryType {M_DOUBLE, M_DOUBLE3, M_BOOLEAN, M_UINT, M_OTHER};

class Inspector;

class Entry : public TGHorizontalFrame {
   public:
    Entry(TGCompositeFrame* fMain, Inspector* parentInspector, const char* entryName, const char* entryType);
    ~Entry() = default;
    std::string     GetEntryName();
    Bool_t          CheckIfValid();
    void            Init(ModelElement* modelElement);
    void            EnableEventHandling() { fIsInitialized = kTRUE; }
    void            UpdateValue();
    virtual Bool_t  ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
   private:
    Bool_t          CheckIfValueChanged();
    Bool_t                        fIsInitialized = kFALSE;
    std::string                   fEntryName;
    EntryType                     fEntryType;
    TGButtonGroup*                fButtonGroup;
    TGRadioButton*                fRadioButtonTrue;
    TGRadioButton*                fRadioButtonFalse;
    ModelElement*                 fModelElement;
    std::vector<TGNumberEntry*>   fNumberEntries;
    std::vector<Double_t>         fCurrentValues;
    Inspector*                    fParentInspector;
    bdm::Cell*                    fCellPtr;
};

}  // namespace gui

#endif