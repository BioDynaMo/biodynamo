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

#include <stdio.h>
#include <stdlib.h>

#include <TGLabel.h>
#include <TGMsgBox.h>
#include <TRootHelpDialog.h>

#include <KeySymbols.h>
#include <TEnv.h>
#include <TROOT.h>
#include <TRint.h>
#include <TStyle.h>
#include <TVirtualX.h>

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

#include <TBrowser.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TContextMenu.h>
#include <TG3DLine.h>
#include <TGButton.h>
#include <TGFileDialog.h>
#include <TGListTree.h>
#include <TGMenu.h>
#include <TGSplitter.h>
#include <TGStatusBar.h>
#include <TGTab.h>
#include <TGTextEdit.h>
#include <TGToolBar.h>
#include <TGToolTip.h>
#include <TGeoManager.h>
#include <THtml.h>
#include <TParticle.h>
#include <TRandom.h>
#include <TRootEmbeddedCanvas.h>
#include <TRootHelpDialog.h>
#include <TSystem.h>
#include <TView.h>

#include <TPluginManager.h>
#include <TVirtualGL.h>

#include "core/util/io.h"
#include "core/util/root.h"
#include "log.h"
#include "model_creator.h"
#include "new_dialog.h"
#include "biodynamo.h"

namespace bdm {

/// testing
inline void* GetCell() {
  const std::array<double, 3> position = {1, 2, 3};
  Cell *c1 = new Cell(position);
  return nullptr;
}

}
  

namespace gui {

enum ProjectSettingsTypes { Id1, Id2 };

//////////////////////////////////////////////////////////
///// New Project Dialog /////////////////////////////////
//////////////////////////////////////////////////////////

NewProjectDialog::NewProjectDialog(const TGWindow *p, const TGWindow *main,
                                   UInt_t w, UInt_t h, UInt_t options)
    : TGTransientFrame(p, main, w, h, options) {

  Log::Info("Testing getting cell members");



  //void *ptr = bdm::GetCell();

  
  Log::Info("First cell member is: ");

  UInt_t wh = (UInt_t)(h - (0.6 * h));

  fFrame1 = new TGHorizontalFrame(this, w, wh, 0);

  fCreateButton = new TGTextButton(fFrame1, "     &Create     ", 1);
  fCreateButton->Associate(this);
  fCancelButton = new TGTextButton(fFrame1, "   &Cancel   ", 2);
  fCancelButton->Associate(this);
  fHelpButton = new TGTextButton(fFrame1, "    &Help    ", 3);
  fHelpButton->Associate(this);

  fL1 =
      new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2);
  fL2 = new TGLayoutHints(kLHintsBottom | kLHintsRight | kLHintsExpandX, 2, 2,
                          5, 1);

  fFrame1->AddFrame(fCreateButton, fL1);
  fFrame1->AddFrame(fHelpButton, fL1);
  fFrame1->AddFrame(fCancelButton, fL1);

  fFrame1->Resize(150, fCreateButton->GetDefaultHeight());
  AddFrame(fFrame1, fL2);

  /// Create tab widget
  fTab = new TGTab(this, 300, 300);
  fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 10, 10, 10);

  TGCompositeFrame *tf = fTab->AddTab("Project settings");

  /// Create vertical frame to contain two horizontal frames
  fV1 = new TGVerticalFrame(tf, w, wh, 0);

  fFrame2 = new TGHorizontalFrame(fV1, 100, wh, 0);
  fFrame2->AddFrame(new TGLabel(fFrame2, "Name:"),
                    new TGLayoutHints(0, 2, 26, 2, 2));
  fFrame2->AddFrame(fTxt1 =
                        new TGTextEntry(fFrame2, new TGTextBuffer(200), Id1));
  fFrame2->AddFrame(fLerror = new TGLabel(fFrame2, "Error"),
                    new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  fLerror->SetTextColor(TColor::RGB2Pixel(255, 0, 0));
  fLerror->SetText("          ");
  fV1->AddFrame(fFrame2, fL1);

  fFrame3 = new TGHorizontalFrame(fV1, 100, wh, 0);
  fL4 = new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 2, 2, 2,
                          2);
  fFrame3->AddFrame(new TGLabel(fFrame3, "Location:"),
                    new TGLayoutHints(0, 2, 10, 2, 2));
  fFrame3->AddFrame(
      fTxt2 = new TGTextEntry(fFrame3, new TGTextBuffer(200), Id2),
      new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
  fPictButton =
      new TGPictureButton(fFrame3, gClient->GetPicture("fileopen.xpm"), 4);
  fFrame3->AddFrame(fPictButton, new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));

  fV1->AddFrame(fFrame3, fL4);

  tf->AddFrame(fV1, fL4);

  /// Register with `ProcessMessage`
  fTxt1->Associate(this);
  fTxt2->Associate(this);
  fPictButton->Associate(this);
  fTxt1->Resize(100, fTxt1->GetDefaultHeight());
  fTxt2->Resize(200, fTxt2->GetDefaultHeight());

  /// Finalize
  TGLayoutHints *fL5 = new TGLayoutHints(
      kLHintsBottom | kLHintsExpandX | kLHintsExpandY, 2, 2, 5, 1);
  AddFrame(fTab, fL5);
  MapSubwindows();
  Resize(GetDefaultSize());

  SetWindowName("New Project");

  // make the message box non-resizable
  UInt_t width = GetDefaultWidth();
  UInt_t height = GetDefaultHeight();
  SetWMSize(width, height);
  SetWMSizeHints(width, height, width, height, 0, 0);
  SetMWMHints(
      kMWMDecorAll | kMWMDecorResizeH | kMWMDecorMaximize | kMWMDecorMinimize |
          kMWMDecorMenu,
      kMWMFuncAll | kMWMFuncResize | kMWMFuncMaximize | kMWMFuncMinimize,
      kMWMInputModeless);
  MapWindow();
  fClient->WaitFor(this);
}

////////////////////////////////////////////////////////////////////////////////
/// Delete new project dialog widgets.

NewProjectDialog::~NewProjectDialog() {
  delete fCreateButton; Log::Debug("Deleting 1");
  delete fCancelButton; Log::Debug("Deleting 2");
  delete fHelpButton;   Log::Debug("Deleting 3");
  delete fPictButton;   Log::Debug("Deleting 4");
  delete fLerror;       Log::Debug("Deleting 5");
  delete fTxt1;         Log::Debug("Deleting 6");
  delete fTxt2;         Log::Debug("Deleting 7");
  delete fFrame1;       Log::Debug("Deleting 8");
  delete fFrame2;       Log::Debug("Deleting 9");
  delete fFrame3;       Log::Debug("Deleting 10");
  delete fV1;           Log::Debug("Deleting 11");
  delete fTab;          Log::Debug("Deleting 12");
  delete fL4;           Log::Debug("Deleting 13");
  delete fL3;           Log::Debug("Deleting 14");
  delete fL2;           Log::Debug("Deleting 15");
  delete fL1;           Log::Debug("Deleting 16");
}

////////////////////////////////////////////////////////////////////////////////
/// Called when Cancel button is clicked.

void NewProjectDialog::OnCancel() { CloseWindow(); }

////////////////////////////////////////////////////////////////////////////////
/// Utility function

int EndsWith(const char *str, const char *suffix) {
  if (!str || !suffix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return 0;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Called when window is closed via the window manager.

void NewProjectDialog::OnOpen() {
  TString dir(".");
  TGFileInfo fInfo;
  fInfo.fIniDir = StrDup(dir);
  const char *projectName = fTxt1->GetText();
  if (projectName) {
    fInfo.fFilename = StrDup(projectName);
  } else {
    // TODO: ERROR
  }

  fInfo.fFileTypes = filetypes;
  Log::Debug("fIniDir = ", fInfo.fIniDir);

  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fInfo);

  if (fInfo.fFilename) {
    const char *suffix = ".root";
    if (!EndsWith(fInfo.fFilename, suffix)) {
      fInfo.fFilename = strcat(fInfo.fFilename, suffix);
    }

    Log::Info("Open file: ", fInfo.fFilename, " (dir:", fInfo.fIniDir, ")");

    fTxt2->Clear();
    fTxt2->AppendText(fInfo.fFilename);

    char *ptr = strrchr(fInfo.fFilename, '/') + 1;
    ptr[strlen(ptr) - strlen(".root")] = '\0';

    fTxt1->Clear();
    fTxt1->AppendText(ptr);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Called when window is closed via the window manager.

void NewProjectDialog::CloseWindow() { DeleteWindow(); }

////////////////////////////////////////////////////////////////////////////////
/// Called when Create button is clicked.

Bool_t NewProjectDialog::OnCreate() {
  const char *projectName = fTxt1->GetText();
  const char *projectLocation = fTxt2->GetText();
  Log::Debug("Checking for valid project name");

  // TODO: Add more rigourous name checking
  if (strcmp(projectName, "") == 0) {
    fLerror->ChangeText("Empty project name!");
    fLerror->Resize();
    return kFALSE;
  } else if (strcmp(projectLocation, "") == 0) {
    fLerror->ChangeText("Empty project location!");
    fLerror->Resize();
    return kFALSE;
  }

  Log::Debug("Terminating dialog: Create pressed");
  // Add protection against double-clicks
  fCreateButton->SetState(kButtonDisabled);
  fCancelButton->SetState(kButtonDisabled);
  fHelpButton->SetState(kButtonDisabled);

  // TODO: Display where project will be created?
  // ProjectUtil::NewProject(fTxt2->GetText(), projectName);
  Log::Info("Creating project at: ", projectLocation);
  gModelCreator->NewProjectSet(projectName, projectLocation);
  if (bdm::FileExists(projectLocation)) {
    Log::Info("File already exists! Will overwrite..");
  } else {
    Log::Info("File does not exist!");
  }

  return kTRUE;
  // Emit("Created()");
  // Send a close message to the main frame. This will trigger the
  // emission of a CloseWindow() signal, which will then call
  // TestDialog::CloseWindow(). Calling directly CloseWindow() will cause
  // a segv since the OK button is still accessed after the DoOK() method.
  // This works since the close message is handled synchronous (via
  // message going to/from X server).
  // fMain->SendCloseMessage();
  // The same effect can be obtained by using a singleshot timer:
  // TTimer::SingleShot(50, "TestMainFrame", const_cast<TestMainFrame*>(mFrame),
  // "EnableSaveAndSimulation()"); TTimer::SingleShot(150, "NewProjectDialog",
  // this, "CloseWindow()");
}

////////////////////////////////////////////////////////////////////////////////
/// Process messages coming from widgets associated with the dialog.

Bool_t NewProjectDialog::ProcessMessage(Long_t msg, Long_t param1, Long_t param2) {
  switch (GET_MSG(msg)) {
    case kC_COMMAND:

      switch (GET_SUBMSG(msg)) {
        case kCM_BUTTON:
          switch (param1) {
            case 1:
              printf("Clicked create!\n");
              if (!OnCreate())

                break;
            case 2:
              OnCancel();
              break;
            case 3:
              TRootHelpDialog *hd;
              hd = new TRootHelpDialog(this, "Help on New Project Dialog", 560,
                                       400);
              // hd->SetText(gSettingsHelp);
              hd->Popup();
              fClient->WaitFor(hd);
              break;
            case 4:
              printf("Clicked open button!\n");
              OnOpen();
            default:
              break;
          }
          break;
        case kCM_TAB:
          break;
        default:
          break;
      }
      break;
    case kC_TEXTENTRY:
      switch (GET_SUBMSG(msg)) {
        case kTE_ENTER:
          switch (param1) {
            case Id1:
              fTxt1->SetFocus();
              break;
            case Id2:
              fTxt2->SetFocus();
              break;
          }
          break;
        case kTE_TAB:
          switch (param1) {
            case Id1:
              fTxt2->SetFocus();
              break;
            case Id2:
              fTxt1->SetFocus();
              break;
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  return kTRUE;
}

//////////////////////////////////////////////////////////
///// New Model Dialog ///////////////////////////////////
//////////////////////////////////////////////////////////

NewModelDialog::NewModelDialog(const TGWindow *p, const TGWindow *main,
                               UInt_t w, UInt_t h, UInt_t options)
    : TGTransientFrame(p, main, w, h, options) {
  UInt_t wh = (UInt_t)(h - (0.6 * h));

  fFrame1 = new TGHorizontalFrame(this, w, wh, 0);

  fCreateButton = new TGTextButton(fFrame1, "     &Create     ", 1);
  fCreateButton->Associate(this);
  fCancelButton = new TGTextButton(fFrame1, "   &Cancel   ", 2);
  fCancelButton->Associate(this);
  fHelpButton = new TGTextButton(fFrame1, "    &Help    ", 3);
  fHelpButton->Associate(this);

  fL1 =
      new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2);
  fL2 = new TGLayoutHints(kLHintsBottom | kLHintsRight | kLHintsExpandX, 2, 2,
                          5, 1);

  fFrame1->AddFrame(fCreateButton, fL1);
  fFrame1->AddFrame(fHelpButton, fL1);
  fFrame1->AddFrame(fCancelButton, fL1);

  fFrame1->Resize(150, fCreateButton->GetDefaultHeight());
  AddFrame(fFrame1, fL2);

  /// Create tab widget
  fTab = new TGTab(this, 300, 300);
  fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 10, 10, 10);

  TGCompositeFrame *tf = fTab->AddTab("Model settings");

  /// Create vertical frame to contain two horizontal frames
  fV1 = new TGVerticalFrame(tf, w, wh, 0);

  fFrame2 = new TGHorizontalFrame(fV1, 100, wh, 0);
  fFrame2->AddFrame(new TGLabel(fFrame2, "Name:"),
                    new TGLayoutHints(0, 2, 26, 2, 2));
  fFrame2->AddFrame(fTxt1 =
                        new TGTextEntry(fFrame2, new TGTextBuffer(200), Id1));
  fFrame2->AddFrame(fLerror = new TGLabel(fFrame2, "Error"),
                    new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  fLerror->SetTextColor(TColor::RGB2Pixel(255, 0, 0));
  fLerror->SetText("          ");
  fV1->AddFrame(fFrame2, fL1);

  fL4 = new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 2, 2, 2,
                          2);

  tf->AddFrame(fV1, fL4);

  /// Register with `ProcessMessage`
  fTxt1->Associate(this);
  fTxt1->Resize(100, fTxt1->GetDefaultHeight());

  /// Finalize
  TGLayoutHints *fL5 = new TGLayoutHints(
      kLHintsBottom | kLHintsExpandX | kLHintsExpandY, 2, 2, 5, 1);
  AddFrame(fTab, fL5);
  MapSubwindows();
  Resize(GetDefaultSize());

  SetWindowName("New Model");

  // make the message box non-resizable
  UInt_t width = GetDefaultWidth();
  UInt_t height = GetDefaultHeight();
  SetWMSize(width, height);
  SetWMSizeHints(width, height, width, height, 0, 0);
  SetMWMHints(
      kMWMDecorAll | kMWMDecorResizeH | kMWMDecorMaximize | kMWMDecorMinimize |
          kMWMDecorMenu,
      kMWMFuncAll | kMWMFuncResize | kMWMFuncMaximize | kMWMFuncMinimize,
      kMWMInputModeless);
  MapWindow();
  fClient->WaitFor(this);
}

////////////////////////////////////////////////////////////////////////////////
/// Delete new project dialog widgets.

NewModelDialog::~NewModelDialog() {
  delete fCreateButton;
  delete fCancelButton;
  delete fHelpButton;
  delete fLerror;
  delete fTxt1;
  delete fFrame1;
  delete fFrame2;
  delete fV1;
  delete fTab;
  delete fL4;
  delete fL3;
  delete fL2;
  delete fL1;
}

////////////////////////////////////////////////////////////////////////////////
/// Called when Cancel button is clicked.

void NewModelDialog::OnCancel() { CloseWindow(); }

////////////////////////////////////////////////////////////////////////////////
/// Called when window is closed via the window manager.

void NewModelDialog::CloseWindow() { DeleteWindow(); }

////////////////////////////////////////////////////////////////////////////////
/// Called when Create button is clicked.

Bool_t NewModelDialog::OnCreate() {
  const char *modelName = fTxt1->GetText();
  Log::Debug("Checking for valid Model name");

  // TODO: Add more rigourous name checking
  if (strcmp(modelName, "") == 0) {
    fLerror->ChangeText("Empty Model name!");
    fLerror->Resize();
    return kFALSE;
  }

  Log::Debug("Terminating dialog: Create pressed");
  // Add protection against double-clicks
  fCreateButton->SetState(kButtonDisabled);
  fCancelButton->SetState(kButtonDisabled);
  fHelpButton->SetState(kButtonDisabled);

  // TODO: Display where project will be created?
  // ProjectUtil::NewProject(fTxt2->GetText(), modelName);
  Log::Info("Creating model: ", modelName);
  gModelCreator->NewModelSet(modelName);

  return kTRUE;
  // Emit("Created()");
  // Send a close message to the main frame. This will trigger the
  // emission of a CloseWindow() signal, which will then call
  // TestDialog::CloseWindow(). Calling directly CloseWindow() will cause
  // a segv since the OK button is still accessed after the DoOK() method.
  // This works since the close message is handled synchronous (via
  // message going to/from X server).
  // fMain->SendCloseMessage();
  // The same effect can be obtained by using a singleshot timer:
  // TTimer::SingleShot(50, "TestMainFrame", const_cast<TestMainFrame*>(mFrame),
  // "EnableSaveAndSimulation()"); TTimer::SingleShot(150, "NewModelDialog",
  // this, "CloseWindow()");
}

////////////////////////////////////////////////////////////////////////////////
/// Process messages coming from widgets associated with the dialog.

Bool_t NewModelDialog::ProcessMessage(Long_t msg, Long_t parm1, Long_t) {
  switch (GET_MSG(msg)) {
    case kC_COMMAND:

      switch (GET_SUBMSG(msg)) {
        case kCM_BUTTON:
          switch (parm1) {
            case 1:
              printf("Clicked create!\n");
              if (!OnCreate())
                break;
            case 2:
              OnCancel();
              break;
            case 3:
              TRootHelpDialog *hd;
              hd = new TRootHelpDialog(this, "Help on New Model Dialog", 560,
                                       400);
              // hd->SetText(gSettingsHelp);
              hd->Popup();
              fClient->WaitFor(hd);
              break;
            default:
              break;
          }
          break;
        case kCM_TAB:
          break;
        default:
          break;
      }
      break;
    case kC_TEXTENTRY:
      switch (GET_SUBMSG(msg)) {
        case kTE_ENTER:
          if (OnCreate())
            OnCancel();
        case kTE_TAB:
          // ? todo
          break;
        default:
          break;
      }
      break;

    default:
      break;
  }
  return kTRUE;
}

}  // namespace gui