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

#ifndef GUI_MODEL_CREATOR_H_
#define GUI_MODEL_CREATOR_H_

#include <KeySymbols.h>
#include <TEnv.h>
#include <TROOT.h>
#include <TVirtualX.h>
#include <TMethodArg.h>

#include <TF1.h>
#include <TFile.h>
#include <TFrame.h>
#include <TH1.h>
#include <TTree.h>
#include <TGFrame.h>
#include <TBrowser.h>
#include <TColor.h>
#include <TContextMenu.h>
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
#include <TRootEmbeddedCanvas.h>
#include <TRootHelpDialog.h>
#include <TSystem.h>
#include <TView.h>
#include <TMethod.h>
#include <TGMsgBox.h>
#include <TG3DLine.h>

#include <TPluginManager.h>
#include <TVirtualGL.h>

namespace gui {

class ButtonModelFrame;
class ButtonProjectFrame;
class TitleFrame;
class TreeManager;
class ModelFrame;

extern TGListTree       *gProjectListTree;  // Project  TGListTree
extern TGListTreeItem   *gTmpLTI;           // Temporary ListTree item
extern TGListTreeItem   *gLTI[];            // Array of ListTree items (Model elements)

class ModelCreator : public TGMainFrame {
 private:
  /// Statics
  static Int_t        fgDefaultXPosition;   // default X position of top left corner
  static Int_t        fgDefaultYPosition;   // default Y position of top left corner

  Bool_t              fOk;                  // Return code from settings dialog
  Bool_t              fIsRunning;           // Simulation running flag
  Bool_t              fModified;            // kTRUE if setting mods not saved
  Bool_t              fInterrupted;         // Interrupts current simulation
  Bool_t              fIsNewProject;        // kTRUE if new project is set
  Bool_t              fIsNewModel;          // kTRUE if new model is set
  Bool_t              fIsGridSet;

  std::string         fProjectName;      // Set by NewProjectDialog
  std::string         fProjectPath;      // Set by NewProjectDialog
  std::string         fModelName;        // Set by NewModelDialog

  std::unique_ptr<TGMenuBar>          fMenuBar;            // Main menu bar
  std::unique_ptr<TEnv>               fModelCreatorEnv;    // ModelCreator environment variables  

  /// MenuBar Frame
  std::unique_ptr<TGPopupMenu>        fMenuFile;           // "File" popup menu
  std::unique_ptr<TGPopupMenu>        fMenuSimulation;     // "Simulation" popup menu
  std::unique_ptr<TGPopupMenu>        fMenuTools;          // "Tools" popup menu
  std::unique_ptr<TGPopupMenu>        fMenuView;           // "View" popup menu
  std::unique_ptr<TGPopupMenu>        fMenuSamples;        // "Samples" popup menu
  std::unique_ptr<TGPopupMenu>        fMenuHelp;           // "Help" popup menu
  std::unique_ptr<TGLayoutHints>      fMenuBarLayout;
  std::unique_ptr<TGLayoutHints>      fMenuBarItemLayout;
  std::unique_ptr<TGLayoutHints>      fMenuBarHelpLayout;
  void  MakeMenuBarFrame();
  void  CloseMenuBarFrame();

  /// ToolBar Frame
  std::unique_ptr<TGToolBar>           fToolBar;
  void  ShowToolBar(Bool_t show = kTRUE);

  /// Layout hints
  std::unique_ptr<TGLayoutHints>       fL1;
  std::unique_ptr<TGLayoutHints>       fL2;
  std::unique_ptr<TGLayoutHints>       fL3;

  /// Title Frame
  std::unique_ptr<TitleFrame>          fTitleFrame;

  /// Main Frame
  std::unique_ptr<TGCompositeFrame>    fMainFrame;

  /// Selection Frame
  std::unique_ptr<TGCompositeFrame>    fSelectionFrame;     // Frame containing list tree and button frame
  std::unique_ptr<ButtonModelFrame>    fButtonModelFrame;   // Frame containing control buttons
  std::unique_ptr<ButtonProjectFrame>  fButtonProjectFrame; // Frame containing control buttons
  void  ChangeSelectionFrame(Bool_t createdProject=kTRUE);

  std::unique_ptr<TGCanvas>            fTreeView;           // Canvas containing model selection list
  std::unique_ptr<TGListTree>          fProjectListTree;      // Project selection TGListTree
  TGListTreeItem*  AddToTree(const char *name = nullptr);

  std::unique_ptr<TreeManager>         fTreeManager;

  std::unique_ptr<ModelFrame>          fModelFrame;

  // Statusbar
  std::unique_ptr<TGStatusBar>         fStatusBar;          // Status bar reporting model info

  Long_t gridNumberX, gridNumberY, gridNumberZ, gridDistanceX, gridDistanceY, gridDistanceZ;
  void  CreateGrid();

 public:
  /// Constructors & destructor
  ModelCreator(const TGWindow *p, UInt_t w, UInt_t h);
  virtual ~ModelCreator() = default;

  /// Statics
  static void         SetDefaultPosition(Int_t x, Int_t y);

  void                SetOk(Bool_t ok=kTRUE) { fOk = ok; }
  void                Modified(Bool_t modified=kTRUE) { fModified = modified; }
  virtual void        Initialize();
  virtual void        CloseWindow();
  virtual Bool_t      ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
  void                CreateNewProject();
  void                CreateNewModel();
  void                LoadProject(std::string fileName);
  void                ClearProject();
  Bool_t              AskForProject(Bool_t loading=kFALSE);
  void                DisplayProjectTree();
  void                CreateNewElement(Int_t type);
  void                HandleTreeInput();
  void                EnableSaving(Bool_t enable=kTRUE);
  Bool_t              GenerateModelCode();
  void                SimulateModel();
  std::string         RunCmd(const char* cmd);

  void                SetGrid(Long_t numberX, Long_t numberY, Long_t numberZ, 
                              Long_t distanceX, Long_t distanceY, Long_t distanceZ);

  template <typename T>
  void NewProjectSet(T& name, T& path) { 
    fProjectName.assign(name); 
    fProjectPath.assign(path);
    fIsNewProject = kTRUE;
  }

  template <typename T>
  void NewModelSet(T& name) { 
    fModelName.assign(name);
    fIsNewModel = kTRUE;
  }
};

extern ModelCreator  *gModelCreator;

}  // namespace gui

#endif // GUI_MODEL_CREATOR_H_