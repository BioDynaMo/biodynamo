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

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// This File contains the declaration of the ModelCreator-class         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef GUI_MODEL_CREATOR_H_
#define GUI_MODEL_CREATOR_H_

#include "TGFrame.h"
#include "TDatime.h"
#include "TCanvas.h"

class TGMenuBar;
class TGPopupMenu;
class TGButton;
class TGListTree;
class TGListTreeItem;
class TRootEmbeddedCanvas;
class TGCanvas;
class TGStatusBar;
class TGTextEdit;
class TGTab;
class TCanvas;
class TPad;
class MyEvent;
class TEnv;
class TTimer;
class TH1F;
class TGToolBar;
class TContextMenu;

namespace gui {

class ButtonModelFrame;
class ButtonProjectFrame;
class TitleFrame;
class TreeManager;
class ModelFrame;

extern TGListTree       *gProjectListTree;  // Project  TGListTree
extern TGListTreeItem   *gProjectListTreeItem;          // First ListTree item
extern TGListTreeItem   *gTmpLTI;           // Temporary ListTree item
extern TGListTreeItem   *gLTI[];            // Array of ListTree items (Model elements)

class ModelCreator : public TGMainFrame {
 private:
  /// Statics
  static Int_t        fgDefaultXPosition;   // default X position of top left corner
  static Int_t        fgDefaultYPosition;   // default Y position of top left corner

  Bool_t              fOk;                  // Return code from settings dialog
  Bool_t              fModified;            // kTRUE if setting mods not saved
  Bool_t              fSettingsModified;    // kTRUE if settings have been modified
  Bool_t              fIsRunning;           // Simulation running flag
  Bool_t              fInterrupted;         // Interrupts current simulation
  Bool_t              fIsNewProject;        // kTRUE if new project is set
  Bool_t              fIsNewModel;          // kTRUE if new model is set

  std::string         fProjectName;      // Set by NewProjectDialog
  std::string         fProjectPath;      // Set by NewProjectDialog
  std::string         fModelName;        // Set by NewModelDialog

  TGMenuBar           *fMenuBar;            // Main menu bar
  TEnv                *fModelCreatorEnv;    // ModelCreator environment variables  

  /// MenuBar Frame
  TGPopupMenu         *fMenuFile;           // "File" popup menu
  TGPopupMenu         *fMenuSimulation;     // "Simulation" popup menu
  TGPopupMenu         *fMenuTools;          // "Tools" popup menu
  TGPopupMenu         *fMenuView;           // "View" popup menu
  TGPopupMenu         *fMenuSamples;        // "Samples" popup menu
  TGPopupMenu         *fMenuHelp;           // "Help" popup menu
  TGLayoutHints       *fMenuBarLayout;
  TGLayoutHints       *fMenuBarItemLayout;
  TGLayoutHints       *fMenuBarHelpLayout;
  void                MakeMenuBarFrame();
  void                CloseMenuBarFrame();

  /// ToolBar Frame
  TGToolBar           *fToolBar;
  void                ShowToolBar(Bool_t show = kTRUE);

  /// Layout hints
  TGLayoutHints       *fL1;
  TGLayoutHints       *fL2;
  TGLayoutHints       *fL3;
  TGLayoutHints       *fL4;
  TGLayoutHints       *fL5;
  TGLayoutHints       *fL6;
  TGLayoutHints       *fL7;
  TGLayoutHints       *fL8;

  /// Title Frame
  TitleFrame         *fTitleFrame;

  /// Main Frame
  TGCompositeFrame    *fMainFrame;

  /// Selection Frame
  TGCompositeFrame    *fSelectionFrame;     // Frame containing list tree and button frame
  ButtonModelFrame    *fButtonModelFrame;   // Frame containing control buttons
  ButtonProjectFrame  *fButtonProjectFrame; // Frame containing control buttons
  void                 ChangeSelectionFrame(Bool_t createdProject=kTRUE);

  TGCanvas            *fTreeView;           // Canvas containing model selection list
  TGListTree          *fProjectListTree;      // Project selection TGListTree
  TGListTreeItem      *AddToTree(const char *name = nullptr);

  TreeManager         *fTreeManager;

  ModelFrame          *fModelFrame;
 
  TContextMenu        *fContextMenu;        // pointer to context menu

  // Display frame
  TList               *fModelCanvas;        // TList containing TGCompositeFrame

  // Statusbar
  TGStatusBar         *fStatusBar;          // Status bar reporting model info

 public:
  /// Statics
  static void         SetDefaultPosition(Int_t x, Int_t y);

  /// Constructors & destructor
  ModelCreator(const TGWindow *p, UInt_t w, UInt_t h);
  virtual ~ModelCreator();

  void                SetOk(Bool_t ok=kTRUE) { fOk = ok; }
  void                Modified(Bool_t modified=kTRUE) { fModified = modified; }
  void                SettingsModified(Bool_t modified=kTRUE) { fSettingsModified = modified; }
  void                Interrupt(Bool_t inter=kTRUE) { fInterrupted = inter; }
  Bool_t              IsInterrupted() { return fInterrupted; }
  virtual void        Initialize();

  virtual void        CloseWindow();
  virtual Bool_t      ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
  void                CreateNewProject();
  void                CreateNewModel();
  void                DisplayProjectTree();
  void                CreateNewCell();
  void                CreateNewGrowthModule();
  void                HandleTreeInput();

  /// TODO:
  //  virtual void    OnOpenProject(const Char_t *filename);
  //  virtual void    OnSaveProject();
  //  virtual void    OnSaveAsProject(const Char_t *filename);

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

} // namespace gui

#endif // GUI_MODEL_CREATOR_H_