// Original Author: Bertrand Bellenot   22/08/02
// Modified by: Lukasz Stempniewicz 21/08/19

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

#ifndef GUI_TITLE_H
#define GUI_TITLE_H

#include <TGFrame.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGPicture.h>
#include <TGIcon.h>

namespace gui {

class TitleFrame : public TGCompositeFrame {
 public:
  // Constructor & destructor
  TitleFrame(const TGWindow *p, const char *mainText, const char *subText,
             UInt_t w, UInt_t h, UInt_t options = kHorizontalFrame | kRaisedFrame);
  virtual ~TitleFrame();
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
};

}  // namespace gui

#endif // GUI_TITLE_H
