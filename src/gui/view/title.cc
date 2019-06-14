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
  theLeftLogoFilename.Append("/icons/logo_bdm.xpm");
  fLeftIconPicture = (TGPicture *)gClient->GetPicture(theLeftLogoFilename);
  fLeftIcon = new TGIcon(this, fLeftIconPicture, fLeftIconPicture->GetWidth(),
                         fLeftIconPicture->GetHeight());
  fLeftLogoLayout = new TGLayoutHints(kLHintsLeft, 0, 0, 0, 0);
  AddFrame(fLeftIcon, fLeftLogoLayout);

  TString theRightLogoFilename = StrDup(gProgPath);
  theRightLogoFilename.Append("/icons/logo_bdm.xpm");
  fRightIconPicture = (TGPicture *)gClient->GetPicture(theRightLogoFilename);
  fRightIcon =
      new TGIcon(this, fRightIconPicture, fRightIconPicture->GetWidth(),
                 fRightIconPicture->GetHeight());
  fRightLogoLayout = new TGLayoutHints(kLHintsRight, 0, 0, 0, 0);
  AddFrame(fRightIcon, fRightLogoLayout);

  // add text
  fTextFrameLayout =
      new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0);
  fTextLabelLayout =
      new TGLayoutHints(kLHintsTop | kLHintsExpandX, 10, 10, 10, 10);
  fTextFrame = new TGCompositeFrame(this, 0, 0, kVerticalFrame);
  fTextLabel1 = new TGLabel(fTextFrame, mainText);
  fTextLabel1->SetTextFont(fontname.Data());
  fTextLabel1->SetTextColor(col);

  fTextLabel2 = new TGLabel(fTextFrame, subText);
  fTextLabel2->SetTextFont(fontname.Data());
  fTextLabel2->SetTextColor(col);
  fTextFrame->AddFrame(fTextLabel1, fTextLabelLayout);
  fTextFrame->AddFrame(fTextLabel2, fTextLabelLayout);

  AddFrame(fTextFrame, fTextFrameLayout);
}

////////////////////////////////////////////////////////////////////////////////
/// Destroy TitleFrame object. Delete all created widgets.

TitleFrame::~TitleFrame() {
  gClient->FreePicture(fLeftIconPicture);
  gClient->FreePicture(fRightIconPicture);
  delete fTextLabel1;
  delete fTextLabel2;
  delete fTextFrame;
  delete fTextLabelLayout;
  delete fTextFrameLayout;
  delete fLeftLogoLayout;
  delete fRightLogoLayout;
}

}  // namespace gui
