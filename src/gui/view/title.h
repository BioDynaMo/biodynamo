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
  std::unique_ptr<TGLayoutHints>    fRightLogoLayout;      // Right logo layout
  std::unique_ptr<TGLayoutHints>    fLeftLogoLayout;       // Left logo layout

  const TGPicture                   *fRightIconPicture;    // Right icon's picture
  const TGPicture                   *fLeftIconPicture;     // Left icon's picture
  std::unique_ptr<TGIcon>           fRightIcon;            // Right icon (logo)
  std::unique_ptr<TGIcon>           fLeftIcon;             // Right icon (logo)

  std::unique_ptr<TGLayoutHints>    fTextFrameLayout;
  std::unique_ptr<TGCompositeFrame> fTextFrame;
  std::unique_ptr<TGLayoutHints>    fTextLabelLayout;
  std::unique_ptr<TGLabel>          fTextLabel1;           // First line title's label
  std::unique_ptr<TGLabel>          fTextLabel2;           // Second line title's label

 public:
  // Constructor & destructor
  TitleFrame(const TGWindow *p, const char *mainText, const char *subText,
             UInt_t w, UInt_t h, UInt_t options = kHorizontalFrame | kRaisedFrame);
  virtual ~TitleFrame();
};

} // namespace gui

#endif  // GUI_TITLE_H
