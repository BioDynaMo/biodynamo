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

#include "gui/view/grid_dialog.h"
#include "gui/view/log.h"

namespace gui {

GridDialog::GridDialog(const TGWindow *p, const TGWindow *main,
                       UInt_t w, UInt_t h, UInt_t options) :
     TGTransientFrame(p, main, w, h, options) {
  fL1 = std::make_unique<TGLayoutHints>(kLHintsExpandX | kLHintsExpandY);
  fL2 = std::make_unique<TGLayoutHints>(kLHintsExpandX | kLHintsExpandY | kLHintsTop);
  fL3 = std::make_unique<TGLayoutHints>(kLHintsExpandX | kLHintsExpandY | kLHintsBottom);
  fL4 = std::make_unique<TGLayoutHints>(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2);
  fL5 = std::make_unique<TGLayoutHints>(kLHintsBottom | kLHintsCenterX, 2, 2, 2, 2);
  fL6 = std::make_unique<TGLayoutHints>( kLHintsTop | kLHintsLeft);
  fLTitle = std::make_unique<TGLayoutHints>(kLHintsExpandX);

  TString fontTitle("-*-times-bold-r-*-*-24-*-*-*-*-*-*-*");
  TString fontBottom("-*-times-*-r-*-*-18-*-*-*-*-*-*-*");

  fVOverall = std::make_unique<TGVerticalFrame>(this, w, h);

  fHSelectionLayout = std::make_unique<TGHorizontalFrame>(fVOverall.get(), w * 0.8, h);

  /// 1st Main Vertical Frame (Cell position)
  fVNumberCells = std::make_unique<TGVerticalFrame>(fHSelectionLayout.get(), w/2, h);
  
  fTitleNumberCells = std::make_unique<TGLabel>(fVNumberCells.get(), "Number of Cells:");
  fTitleNumberCells->SetTextFont(fontTitle.Data());

  fVNumberCells->AddFrame(fTitleNumberCells.get(), fLTitle.get());

  fCanvasNumberCells = std::make_unique<TGCanvas>(fVNumberCells.get());
  Pixel_t white;
  gClient->GetColorByName("white", white);
  
  fNumberCellsCompFrame = std::make_unique<TGCompositeFrame>(fCanvasNumberCells->GetViewPort());
  fCanvasNumberCells->SetContainer(fNumberCellsCompFrame.get());
  fVNumberCells->AddFrame(fCanvasNumberCells.get(), fL1.get());
  fCanvasNumberCells->ChangeBackground(white);
  fNumberCellsCompFrame->ChangeBackground(white);

  TString pictureFilename = StrDup(gProgPath);
  pictureFilename.Append("/icons/coordinate_black.png");

  fNumberCellsPicture = gClient->GetPicture(pictureFilename);
  fNumberCellsIcon = std::make_unique<TGIcon>(fNumberCellsCompFrame.get(), fNumberCellsPicture, fNumberCellsPicture->GetWidth(), fNumberCellsPicture->GetHeight());
  fNumberCellsCompFrame->AddFrame(fNumberCellsIcon.get(), fL1.get());

  fNumberCellsBottom = std::make_unique<TGLabel>(fVNumberCells.get(), "Total Cell Count:");

  fVNumberCells->AddFrame(fNumberCellsBottom.get(), fLTitle.get());
  fNumberCellsBottom->SetTextFont(fontBottom.Data());

  fHSelectionLayout->AddFrame(fVNumberCells.get(), fL2.get());

  // Inner vertical frame for Cell position
  fVNumberCellsInner = std::make_unique<TGVerticalFrame>(fNumberCellsCompFrame.get(), w/3, h/10);

  fHNumberCellsInnerX = std::make_unique<TGHorizontalFrame>(fVNumberCellsInner.get());
  fHNumberCellsInnerY = std::make_unique<TGHorizontalFrame>(fVNumberCellsInner.get());
  fHNumberCellsInnerZ = std::make_unique<TGHorizontalFrame>(fVNumberCellsInner.get());

  fEntryNumberX = std::make_unique<TGNumberEntry>(fHNumberCellsInnerX.get());
  fEntryNumberX->Associate(this);
  fEntryNumberX->SetNumStyle(TGNumberFormat::kNESInteger);
  fEntryNumberX->SetNumAttr(TGNumberFormat::kNEAPositive);
  fNumberTitleX = std::make_unique<TGLabel>(fHNumberCellsInnerX.get(), "X:");
  fHNumberCellsInnerX->AddFrame(fEntryNumberX.get());
  fHNumberCellsInnerX->AddFrame(fNumberTitleX.get());

  fEntryNumberY = std::make_unique<TGNumberEntry>(fHNumberCellsInnerY.get());
  fEntryNumberY->Associate(this);
  fEntryNumberY->SetNumStyle(TGNumberFormat::kNESInteger);
  fEntryNumberY->SetNumAttr(TGNumberFormat::kNEAPositive);

  fNumberTitleY = std::make_unique<TGLabel>(fHNumberCellsInnerY.get(), "Y:");
  fHNumberCellsInnerY->AddFrame(fEntryNumberY.get());
  fHNumberCellsInnerY->AddFrame(fNumberTitleY.get());

  fEntryNumberZ = std::make_unique<TGNumberEntry>(fHNumberCellsInnerZ.get());
  fEntryNumberZ->Associate(this);
  fEntryNumberZ->SetNumStyle(TGNumberFormat::kNESInteger);
  fEntryNumberZ->SetNumAttr(TGNumberFormat::kNEAPositive);

  fNumberTitleZ = std::make_unique<TGLabel>(fHNumberCellsInnerZ.get(), "Z:");
  fHNumberCellsInnerZ->AddFrame(fEntryNumberZ.get());
  fHNumberCellsInnerZ->AddFrame(fNumberTitleZ.get());

  fVNumberCellsInner->AddFrame(fHNumberCellsInnerX.get());
  fVNumberCellsInner->AddFrame(fHNumberCellsInnerY.get());
  fVNumberCellsInner->AddFrame(fHNumberCellsInnerZ.get());

  fNumberCellsCompFrame->AddFrame(fVNumberCellsInner.get(), fL6.get());

  /// 2nd Main Vertical Frame (Distance between cells)
  pictureFilename = StrDup(gProgPath);
  pictureFilename.Append("/icons/coordinate_original.png");

  fVDistanceCells = std::make_unique<TGVerticalFrame>(fHSelectionLayout.get(), w/2, h);

  fTitleDistanceCells = std::make_unique<TGLabel>(fVDistanceCells.get(), "Distance between cells:");
  fTitleDistanceCells->SetTextFont(fontTitle.Data());
  fVDistanceCells->AddFrame(fTitleDistanceCells.get(), fLTitle.get());

  fCanvasDistanceCells = std::make_unique<TGCanvas>(fVDistanceCells.get());

  fDistanceCellsCompFrame = std::make_unique<TGCompositeFrame>(fCanvasDistanceCells->GetViewPort());
  fCanvasDistanceCells->SetContainer(fDistanceCellsCompFrame.get());

  fDistanceCellsPicture = gClient->GetPicture(pictureFilename);
  fDistanceCellsIcon = std::make_unique<TGIcon>(fDistanceCellsCompFrame.get(), fDistanceCellsPicture, fDistanceCellsPicture->GetWidth(), fDistanceCellsPicture->GetHeight());
  fDistanceCellsCompFrame->AddFrame(fDistanceCellsIcon.get(), fL1.get());

  fVDistanceCells->AddFrame(fCanvasDistanceCells.get(), fL1.get());
  fCanvasDistanceCells->ChangeBackground(white);
  fDistanceCellsCompFrame->ChangeBackground(white);

  fDistanceCellsBottom = std::make_unique<TGLabel>(fVDistanceCells.get(), " ");
  fDistanceCellsBottom->SetTextFont(fontBottom.Data());

  fVDistanceCells->AddFrame(fDistanceCellsBottom.get(), fLTitle.get());

  fHSelectionLayout->AddFrame(fVDistanceCells.get(), fL1.get());

  fVOverall->AddFrame(fHSelectionLayout.get(), fL2.get());

  // Inner vertical frame for Cell distance
  fVDistanceCellsInner = std::make_unique<TGVerticalFrame>(fDistanceCellsCompFrame.get(), w/3, h/10);

  fHDistanceCellsInnerX = std::make_unique<TGHorizontalFrame>(fVDistanceCellsInner.get());
  fHDistanceCellsInnerY = std::make_unique<TGHorizontalFrame>(fVDistanceCellsInner.get());
  fHDistanceCellsInnerZ = std::make_unique<TGHorizontalFrame>(fVDistanceCellsInner.get());

  fEntryDistanceX = std::make_unique<TGNumberEntry>(fHDistanceCellsInnerX.get());
  fEntryDistanceX->SetNumStyle(TGNumberFormat::kNESInteger);
  fEntryDistanceX->SetNumAttr(TGNumberFormat::kNEAPositive);
  fEntryDistanceX->Associate(this);

  fDistanceTitleX = std::make_unique<TGLabel>(fHDistanceCellsInnerX.get(), "X:");
  fHDistanceCellsInnerX->AddFrame(fEntryDistanceX.get());
  fHDistanceCellsInnerX->AddFrame(fDistanceTitleX.get());

  fEntryDistanceY = std::make_unique<TGNumberEntry>(fHDistanceCellsInnerY.get());
  fEntryDistanceY->SetNumStyle(TGNumberFormat::kNESInteger);
  fEntryDistanceY->SetNumAttr(TGNumberFormat::kNEAPositive);
  fEntryDistanceY->Associate(this);

  fDistanceTitleY = std::make_unique<TGLabel>(fHDistanceCellsInnerY.get(), "Y:");
  fHDistanceCellsInnerY->AddFrame(fEntryDistanceY.get());
  fHDistanceCellsInnerY->AddFrame(fDistanceTitleY.get());

  fEntryDistanceZ = std::make_unique<TGNumberEntry>(fHDistanceCellsInnerZ.get());
  fEntryDistanceZ->SetNumStyle(TGNumberFormat::kNESInteger);
  fEntryDistanceZ->SetNumAttr(TGNumberFormat::kNEAPositive);
  fEntryDistanceZ->Associate(this);
  fDistanceTitleZ = std::make_unique<TGLabel>(fHDistanceCellsInnerZ.get(), "Z:");
  fHDistanceCellsInnerZ->AddFrame(fEntryDistanceZ.get());
  fHDistanceCellsInnerZ->AddFrame(fDistanceTitleZ.get());

  fVDistanceCellsInner->AddFrame(fHDistanceCellsInnerX.get());
  fVDistanceCellsInner->AddFrame(fHDistanceCellsInnerY.get());
  fVDistanceCellsInner->AddFrame(fHDistanceCellsInnerZ.get());

  fDistanceCellsCompFrame->AddFrame(fVDistanceCellsInner.get(), fL6.get());

  /// Create button frame
  fHButtons = std::make_unique<TGHorizontalFrame>(fVOverall.get());

  fCreateButton = std::make_unique<TGTextButton>(fHButtons.get(), " &Create ", 1);
  fCreateButton->Associate(this);
  fCancelButton = std::make_unique<TGTextButton>(fHButtons.get(), " &Cancel ", 2);
  fCancelButton->Associate(this);

  fHButtons->AddFrame(fCreateButton.get(), fL4.get());
  fHButtons->AddFrame(fCancelButton.get(), fL4.get());
  fHButtons->Resize(150, fCreateButton->GetDefaultHeight());
  fVOverall->AddFrame(fHButtons.get(), fL5.get());

  AddFrame(fVOverall.get(), fL1.get());
  MapSubwindows();
  Resize(w, h);
  MapWindow();
  Resize(w + 1, h + 1);
  MapSubwindows();
  Resize(w, h);

  SetWindowName("Cell Grid Creation");

  // make the message box non-resizable
  //UInt_t width = w;
  //UInt_t height = h;
  //SetWMSize(width, height);
  //SetWMSizeHints(width, height, width, height, 0, 0);
  //SetMWMHints(
  //    kMWMDecorAll | kMWMDecorResizeH | kMWMDecorMaximize | kMWMDecorMinimize |
  //        kMWMDecorMenu,
  //    kMWMFuncAll | kMWMFuncResize | kMWMFuncMaximize | kMWMFuncMinimize,
  //    kMWMInputModeless);

  fClient->WaitFor(this);
}

void GridDialog::OnCreate() {
  gModelCreator->SetGrid(fEntryNumberX->GetIntNumber(), 
                         fEntryNumberY->GetIntNumber(),
                         fEntryNumberZ->GetIntNumber(),
                         fEntryDistanceX->GetIntNumber(),
                         fEntryDistanceY->GetIntNumber(),
                         fEntryDistanceZ->GetIntNumber());
  CloseWindow();
}

/// Checks entry values updates corresponding 
///   cell total and distance labels
void GridDialog::VerifyNumberEntries() {
  /// Represents number of cells in X, Y, Z
  Long_t numberX = fEntryNumberX->GetIntNumber();
  Long_t numberY = fEntryNumberY->GetIntNumber();
  Long_t numberZ = fEntryNumberZ->GetIntNumber();

  /// Represents distances between cells in X, Y, Z
  Long_t distanceX = fEntryDistanceX->GetIntNumber();
  Long_t distanceY = fEntryDistanceY->GetIntNumber();
  Long_t distanceZ = fEntryDistanceZ->GetIntNumber();

  /// Check to ensure all entries are > 0
  if ((numberX && numberY && numberZ && 
      distanceX && distanceY && distanceZ) == 0) {
    Log::Warning("Cannot have any value at 0!");
    return;
  }

  /// Build strings for labels
  std::string cellCountStr = bdm::Concat("Total Cell Count:", std::to_string(numberX * numberY * numberZ));
  fNumberCellsBottom->SetText(cellCountStr.c_str());

  std::string cellDistanceStr("Grid size:");
  cellDistanceStr.append("(X) - ");
  cellDistanceStr.append(std::to_string(numberX * distanceX - distanceX));
  cellDistanceStr.append(", (Y) - ");
  cellDistanceStr.append(std::to_string(numberY * distanceY - distanceY));
  cellDistanceStr.append(", (Z) - ");
  cellDistanceStr.append(std::to_string(numberZ * distanceZ - distanceZ));

  fDistanceCellsBottom->SetText(cellDistanceStr.c_str());
}

void GridDialog::OnCancel() { CloseWindow(); }

Bool_t GridDialog::ProcessMessage(Long_t msg, Long_t param1, Long_t param2) {
  switch (GET_MSG(msg)) {
    case kC_COMMAND:

      switch (GET_SUBMSG(msg)) {
        case kCM_BUTTON:
          switch (param1) {
            case 1:
              Log::Info("Clicked create!\n");
              OnCreate();
              break;
            case 2:
              Log::Info("Clicked cancel!\n");
              OnCancel();
              break;
          }
          break;
        case kCM_TAB:
          break;
        default:
          break;
      }
      break;
    default:
      VerifyNumberEntries();
      break;
  }
  return kTRUE;
}

GridDialog::~GridDialog() {
  //gClient->FreePicture(fNumberCellsPicture);
  //gClient->FreePicture(fDistanceCellsPicture);
}

void GridDialog::CloseWindow() { DeleteWindow(); }

}  // namespace gui
