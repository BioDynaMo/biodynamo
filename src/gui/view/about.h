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

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Definition of the "About" message box for the Model Creator application  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef GUI_ABOUT_H
#define GUI_ABOUT_H

#include <TSystem.h>
#include <TROOT.h>
#include <TRootHelpDialog.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGPicture.h>

namespace gui {

class ModelCreatorAbout : public TGTransientFrame {
 public:
  /// Constructor and destructor
  ModelCreatorAbout(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h,
                    UInt_t options = kMainFrame | kVerticalFrame);
  ~ModelCreatorAbout() = default;

  virtual void       CloseWindow();
  virtual Bool_t     ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

 private:
  std::unique_ptr<TGVerticalFrame>    fVFrame;
  std::unique_ptr<TGHorizontalFrame>  fHFrame;
  std::unique_ptr<TGTextButton>       fOkButton;
  std::unique_ptr<TGLabel>            fLabel1;
  std::unique_ptr<TGLabel>            fLabel2;
  std::unique_ptr<TGLabel>            fLabel4;
  std::unique_ptr<TGLayoutHints>      fLogoLayout;
  std::unique_ptr<TGLayoutHints>      fL1;
  std::unique_ptr<TGLayoutHints>      fL2;
  std::unique_ptr<TGLayoutHints>      fBly;
  std::unique_ptr<TGLayoutHints>      fBfly;

};

}  // namespace gui

#endif // GUI_ABOUT_H
