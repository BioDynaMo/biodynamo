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

//////////////////////////////////////////////
//                                          //
// Definition of a new project dialog box   //
//                                          //
//////////////////////////////////////////////

#ifndef GUI_NEW_DIALOG_H
#define GUI_NEW_DIALOG_H

#include <TColor.h>
#include <TFile.h>
#include <TGFileDialog.h>
#include <TGListBox.h>
#include <TGTab.h>

#include <TGButton.h>
#include <TGFrame.h>
#include <TGTextEntry.h>

namespace gui {

extern const char *filetypes[];

class NewProjectDialog : public TGTransientFrame {
 public:
  NewProjectDialog(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h,
                   UInt_t options = kVerticalFrame);
  virtual ~NewProjectDialog();

  // slots
  void              OnCancel();
  void              OnOpen();
  void              CloseWindow();
  Bool_t            OnCreate();
  Bool_t            ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

 private:
  TGCompositeFrame *fFrame1;
  TGCompositeFrame *fFrame2;
  TGCompositeFrame *fFrame3;
  TGVerticalFrame  *fV1;
  TGButton         *fCreateButton;
  TGButton         *fCancelButton;
  TGButton         *fHelpButton;
  TGPictureButton  *fPictButton;
  TGTab            *fTab;
  TGTextEntry      *fTxt1;
  TGTextEntry      *fTxt2;
  TGLabel          *fLerror;
  TGLayoutHints    *fL1;
  TGLayoutHints    *fL2;
  TGLayoutHints    *fL3;
  TGLayoutHints    *fL4;
};


//////////////////////////////////////////////
//                                          //
// Definition of a new model dialog box     //
//                                          //
//////////////////////////////////////////////

class NewModelDialog : public TGTransientFrame {
 public:
  NewModelDialog(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h,
                 UInt_t options = kVerticalFrame);
  virtual ~NewModelDialog();

  // slots
  void             OnCancel();
  void             CloseWindow();
  Bool_t           OnCreate();
  Bool_t           ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

 private:
  TGCompositeFrame *fFrame1;
  TGCompositeFrame *fFrame2;
  TGVerticalFrame  *fV1;
  TGButton         *fCreateButton, *fCancelButton, *fHelpButton;
  TGTab            *fTab;
  TGTextEntry      *fTxt1;
  TGLabel          *fLerror;
  TGLayoutHints    *fL1, *fL2, *fL3, *fL4;
};

}  // namespace gui

#endif