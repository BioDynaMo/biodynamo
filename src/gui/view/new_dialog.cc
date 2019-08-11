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

#include "gui/view/new_dialog.h"

namespace gui {

enum ProjectSettingsTypes { Id1, Id2 };

//////////////////////////////////////////////////////////
///// New Project Dialog /////////////////////////////////
//////////////////////////////////////////////////////////

NewProjectDialog::NewProjectDialog(const TGWindow *p, const TGWindow *main,
                                   UInt_t w, UInt_t h, UInt_t options)
    : TGTransientFrame(p, main, w, h, options) {

  UInt_t wh = (UInt_t)(h - (0.6 * h));

  fFrame1 = std::make_unique<TGHorizontalFrame>(this, w, wh, 0);

  fCreateButton = std::make_unique<TGTextButton>(fFrame1.get(), "     &Create     ", 1);
  fCreateButton->Associate(this);
  fCancelButton = std::make_unique<TGTextButton>(fFrame1.get(), "   &Cancel   ", 2);
  fCancelButton->Associate(this);
  fHelpButton = std::make_unique<TGTextButton>(fFrame1.get(), "    &Help    ", 3);
  fHelpButton->Associate(this);

  fL1 = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2);
  fL2 = std::make_unique<TGLayoutHints>(kLHintsBottom | kLHintsRight | kLHintsExpandX, 2, 2,
                          5, 1);

  fFrame1->AddFrame(fCreateButton.get(), fL1.get());
  fFrame1->AddFrame(fHelpButton.get(), fL1.get());
  fFrame1->AddFrame(fCancelButton.get(), fL1.get());

  fFrame1->Resize(150, fCreateButton->GetDefaultHeight());
  AddFrame(fFrame1.get(), fL2.get());

  /// Create tab widget
  fTab = std::make_unique<TGTab>(this, 300, 300);
  fL3 = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsLeft, 10, 10, 10, 10);

  TGCompositeFrame *tf = fTab->AddTab("Project settings");

  /// Create vertical frame to contain two horizontal frames
  fV1 = std::make_unique<TGVerticalFrame>(tf, w, wh, 0);

  fFrame2 = std::make_unique<TGHorizontalFrame>(fV1.get(), 100, wh, 0);
  fFrame2->AddFrame(new TGLabel(fFrame2.get(), "Name:"),
                    new TGLayoutHints(0, 2, 26, 2, 2));
  fTxt1 = std::make_unique<TGTextEntry>(fFrame2.get(), new TGTextBuffer(200), Id1);
  fLerror = std::make_unique<TGLabel>(fFrame2.get(), "Error");
  fFrame2->AddFrame(fTxt1.get());
  fFrame2->AddFrame(fLerror.get(), new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  fLerror->SetTextColor(TColor::RGB2Pixel(255, 0, 0));
  fLerror->SetText("          ");
  fV1->AddFrame(fFrame2.get(), fL1.get());

  fFrame3 = std::make_unique<TGHorizontalFrame>(fV1.get(), 100, wh, 0);
  fL4 = std::make_unique<TGLayoutHints>(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 2, 2, 2,
                          2);
  fFrame3->AddFrame(new TGLabel(fFrame3.get(), "Location:"),
                    new TGLayoutHints(0, 2, 10, 2, 2));
  fTxt2 = std::make_unique<TGTextEntry>(fFrame3.get(), new TGTextBuffer(200), Id2);
  fFrame3->AddFrame(fTxt2.get(), new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
  TString openFicFilename = StrDup(gProgPath);
  openFicFilename.Append("/icons/file_open.png");
  fPictButton = std::make_unique<TGPictureButton>(fFrame3.get(), gClient->GetPicture(openFicFilename), 4);
  fFrame3->AddFrame(fPictButton.get(), new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));

  fV1->AddFrame(fFrame3.get(), fL4.get());

  tf->AddFrame(fV1.get(), fL4.get());

  /// Register with `ProcessMessage`
  fTxt1->Associate(this);
  fTxt2->Associate(this);
  fPictButton->Associate(this);
  fTxt1->Resize(100, fTxt1->GetDefaultHeight());
  fTxt2->Resize(200, fTxt2->GetDefaultHeight());

  /// Finalize
  AddFrame(fTab.get(), new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsExpandY, 2, 2, 5, 1));
  MapSubwindows();
  Resize(GetDefaultSize());

  SetWindowName("New Project");

  MapWindow();
  fClient->WaitFor(this);
}

////////////////////////////////////////////////////////////////////////////////
/// Delete new project dialog widgets.

NewProjectDialog::~NewProjectDialog() {}

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
              Log::Info("Clicked create!\n");
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
              Log::Debug("Clicked open button!");
              OnOpen();
              fLerror->ChangeText("");
              fLerror->Resize();
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

  fFrame1 = std::make_unique<TGHorizontalFrame>(this, w, wh, 0);

  fCreateButton = std::make_unique<TGTextButton>(fFrame1.get(), "     &Create     ", 1);
  fCreateButton->Associate(this);
  fCancelButton = std::make_unique<TGTextButton>(fFrame1.get(), "   &Cancel   ", 2);
  fCancelButton->Associate(this);
  fHelpButton = std::make_unique<TGTextButton>(fFrame1.get(), "    &Help    ", 3);
  fHelpButton->Associate(this);

  fL1 = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2);
  fL2 = std::make_unique<TGLayoutHints>(kLHintsBottom | kLHintsRight | kLHintsExpandX, 2, 2,
                          5, 1);

  fFrame1->AddFrame(fCreateButton.get(), fL1.get());
  fFrame1->AddFrame(fHelpButton.get(), fL1.get());
  fFrame1->AddFrame(fCancelButton.get(), fL1.get());

  fFrame1->Resize(150, fCreateButton->GetDefaultHeight());
  AddFrame(fFrame1.get(), fL2.get());

  /// Create tab widget
  fTab = std::make_unique<TGTab>(this, 300, 300);
  fL3 = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsLeft, 10, 10, 10, 10);

  TGCompositeFrame *tf = fTab->AddTab("Model settings");

  /// Create vertical frame to contain two horizontal frames
  fV1 = std::make_unique<TGVerticalFrame>(tf, w, wh, 0);

  fFrame2 = std::make_unique<TGHorizontalFrame>(fV1.get(), 100, wh, 0);
  fFrame2->AddFrame(new TGLabel(fFrame2.get(), "Name:"),
                    new TGLayoutHints(0, 2, 26, 2, 2));
  fTxt1 = std::make_unique<TGTextEntry>(fFrame2.get(), new TGTextBuffer(200), Id1);
  fLerror = std::make_unique<TGLabel>(fFrame2.get(), "Error");
  fFrame2->AddFrame(fTxt1.get());
  fFrame2->AddFrame(fLerror.get(), new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  fLerror->SetTextColor(TColor::RGB2Pixel(255, 0, 0));
  fLerror->SetText("          ");
  fV1->AddFrame(fFrame2.get(), fL1.get());

  fL4 = std::make_unique<TGLayoutHints>(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 2, 2, 2,
                          2);

  tf->AddFrame(fV1.get(), fL4.get());

  /// Register with `ProcessMessage`
  fTxt1->Associate(this);
  fTxt1->Resize(100, fTxt1->GetDefaultHeight());

  /// Finalize
  AddFrame(fTab.get(), new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsExpandY, 2, 2, 5, 1));
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

NewModelDialog::~NewModelDialog() {}

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