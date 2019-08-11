// Author: Bertrand Bellenot   22/08/02
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

#include <TGButton.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGPicture.h>
#include <TGResourcePool.h>
#include <TSystem.h>

#include "title.h"

namespace gui {

//______________________________________________________________________________
//
// TitleFrame
//______________________________________________________________________________

////////////////////////////////////////////////////////////////////////////////
/// Create TitleFrame object, with TGWindow parent 'p', text 'mainText'
/// with sub text 'subText'.

TitleFrame::TitleFrame(const TGWindow *p, const char *mainText,
                       const char *subText, UInt_t w, UInt_t h, UInt_t options)
    : TGCompositeFrame(p, w, h, options) {
  Pixel_t col;
  TString fontname("-*-times-bold-r-*-*-24-*-*-*-*-*-*-*");
  gClient->GetColorByName("black", col);

  // add pictures
  TString theLeftLogoFilename = StrDup(gProgPath);
  theLeftLogoFilename.Append("/icons/logo_bdm.png");
  fLeftIconPicture = gClient->GetPicture(theLeftLogoFilename);
  fLeftIcon = std::make_unique<TGIcon>(this, fLeftIconPicture, fLeftIconPicture->GetWidth(),
                         fLeftIconPicture->GetHeight());
  fLeftLogoLayout = std::make_unique<TGLayoutHints>(kLHintsLeft, 0, 0, 0, 0);
  AddFrame(fLeftIcon.get(), fLeftLogoLayout.get());

  TString theRightLogoFilename = StrDup(gProgPath);
  theRightLogoFilename.Append("/icons/logo_bdm.png");
  fRightIconPicture = gClient->GetPicture(theRightLogoFilename);
  fRightIcon = std::make_unique<TGIcon>(this, fRightIconPicture, fRightIconPicture->GetWidth(),
                 fRightIconPicture->GetHeight());
  fRightLogoLayout = std::make_unique<TGLayoutHints>(kLHintsRight, 0, 0, 0, 0);
  AddFrame(fRightIcon.get(), fRightLogoLayout.get());

  // add text
  fTextFrameLayout = std::make_unique<TGLayoutHints>(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0);
  fTextLabelLayout = std::make_unique<TGLayoutHints>(kLHintsTop | kLHintsExpandX, 10, 10, 10, 10);
  fTextFrame = std::make_unique<TGCompositeFrame>(this, 0, 0, kVerticalFrame);
  fTextLabel1 = std::make_unique<TGLabel>(fTextFrame.get(), mainText);
  fTextLabel1->SetTextFont(fontname.Data());
  fTextLabel1->SetTextColor(col);

  fTextLabel2 = std::make_unique<TGLabel>(fTextFrame.get(), subText);
  fTextLabel2->SetTextFont(fontname.Data());
  fTextLabel2->SetTextColor(col);
  fTextFrame->AddFrame(fTextLabel1.get(), fTextLabelLayout.get());
  fTextFrame->AddFrame(fTextLabel2.get(), fTextLabelLayout.get());

  AddFrame(fTextFrame.get(), fTextFrameLayout.get());
}

////////////////////////////////////////////////////////////////////////////////
/// Destroy TitleFrame object. Delete all created widgets.

TitleFrame::~TitleFrame() {
  gClient->FreePicture(fLeftIconPicture);
  gClient->FreePicture(fRightIconPicture);
}

}  // namespace gui
