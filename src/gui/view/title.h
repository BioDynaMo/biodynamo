// Original Author: Bertrand Bellenot   22/08/02
// Modified by: Lukasz Stempniewicz 25/05/19

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
//  TitleFrame                                                          //
//                                                                      //
// This File contains the declaration of the  TitleFrame-class for      //
// the ModelCreator application                                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef GUI_TITLE_H
#define GUI_TITLE_H

#include "TGFrame.h"

class TGLabel;
class TGButton;
class TGPicture;
class TGIcon;

namespace gui {

class TitleFrame : public TGCompositeFrame {
 private:
  TGLayoutHints    *fRightLogoLayout;      // Right logo layout
  TGLayoutHints    *fLeftLogoLayout;       // Left logo layout

  TGPicture        *fRightIconPicture;     // Right icon's picture
  TGIcon           *fRightIcon;            // Right icon (logo)
  TGPicture        *fLeftIconPicture;      // Left icon's picture
  TGIcon           *fLeftIcon;             // Right icon (logo)

  TGLayoutHints    *fTextFrameLayout;
  TGCompositeFrame *fTextFrame;
  TGLayoutHints    *fTextLabelLayout;
  TGLabel          *fTextLabel1;           // First line title's label
  TGLabel          *fTextLabel2;           // Second line title's label

 public:
  // Constructor & destructor
  TitleFrame(const TGWindow *p, const char *mainText, const char *subText,
             UInt_t w, UInt_t h, UInt_t options = kHorizontalFrame | kRaisedFrame);
  virtual ~TitleFrame();
};

} // namespace gui

#endif  // GUI_TITLE_H
