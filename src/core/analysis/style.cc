// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "core/analysis/style.h"
#include <iostream> // TODO remove
#include <TBufferJSON.h> // TODO remove
namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
Style::Style(TRootIOCtor*) : tstyle_(new TStyle(*gStyle)) {
  ToTStyle();
  std::cout << "Style trootioctor" << std::endl;
  std::cout << TBufferJSON::ToJSON(tstyle_, TBufferJSON::kMapAsObject).Data() << std::endl;
  std::cout << std::endl;
}

// -----------------------------------------------------------------------------
Style::Style() : tstyle_(new TStyle(*gStyle)) {
  std::cout << "Style ctor" << std::endl;
  ToTStyle();
}

// -----------------------------------------------------------------------------
Style::Style(TStyle* tstyle) : tstyle_(new TStyle(*tstyle)) {
  std::cout << "Style tstyle ctor" << std::endl;
  FromTStyle();
}

// -----------------------------------------------------------------------------
Style::~Style() {
  if (tstyle_) {
    delete tstyle_;
  }
}

// -----------------------------------------------------------------------------
const TStyle* Style::GetTStyle() const {
  return tstyle_;
}

// -----------------------------------------------------------------------------
void Style::ToTStyle() {
   // delete gROOT->GetStyle("Modern");
   //
   auto set_axis = [this](TAttAxis& axis, Option_t* o) {
     this->tstyle_->SetNdivisions(axis.GetNdivisions(), o);
     this->tstyle_->SetAxisColor(axis.GetAxisColor(), o);
     this->tstyle_->SetLabelColor(axis.GetLabelColor(), o);
     this->tstyle_->SetLabelFont(axis.GetLabelFont(), o);
     this->tstyle_->SetLabelOffset(axis.GetLabelOffset(), o);
     this->tstyle_->SetLabelSize(axis.GetLabelSize(), o);
     this->tstyle_->SetTickLength(axis.GetTickLength(), o);
     this->tstyle_->SetTitleOffset(axis.GetTitleOffset(), o);
     this->tstyle_->SetTitleSize(axis.GetTitleSize(), o);
     this->tstyle_->SetTitleColor(axis.GetTitleColor(), o);
     this->tstyle_->SetTitleFont(axis.GetTitleFont(), o);
   };
   set_axis(fXaxis, "x"); 
   set_axis(fYaxis, "y"); 
   set_axis(fZaxis, "z"); 

   tstyle_->SetBarWidth(fBarWidth);
   tstyle_->SetBarOffset(fBarOffset);
   tstyle_->SetColorModelPS(fColorModelPS);
   tstyle_->SetDrawBorder(fDrawBorder);
   tstyle_->SetOptLogx(fOptLogx);
   tstyle_->SetOptLogy(fOptLogy);
   tstyle_->SetOptLogz(fOptLogz);
   tstyle_->SetOptDate(fOptDate);
   tstyle_->SetOptStat(fOptStat);
   tstyle_->SetOptTitle(fOptTitle);
   tstyle_->SetOptFit(fOptFit);
   tstyle_->SetNumberContours(fNumberContours);
   tstyle_->GetAttDate()->SetTextFont(fAttDate.GetTextFont());
   tstyle_->GetAttDate()->SetTextSize(fAttDate.GetTextSize());
   tstyle_->GetAttDate()->SetTextAngle(fAttDate.GetTextAngle());
   tstyle_->GetAttDate()->SetTextAlign(fAttDate.GetTextAlign());
   tstyle_->GetAttDate()->SetTextColor(fAttDate.GetTextColor());
   tstyle_->SetDateX(fDateX);
   tstyle_->SetDateY(fDateY);
   tstyle_->SetEndErrorSize(fEndErrorSize);
   tstyle_->SetErrorX(fErrorX);
   tstyle_->SetFuncColor(fFuncColor);
   tstyle_->SetFuncStyle(fFuncStyle);
   tstyle_->SetFuncWidth(fFuncWidth);
   tstyle_->SetGridColor(fGridColor);
   tstyle_->SetGridStyle(fGridStyle);
   tstyle_->SetGridWidth(fGridWidth);
   tstyle_->SetLegendBorderSize(fLegendBorderSize);
   tstyle_->SetLegendFillColor(fLegendFillColor);
   tstyle_->SetLegendFont(fLegendFont);
   tstyle_->SetLegendTextSize(fLegendTextSize);
   tstyle_->SetHatchesLineWidth(fHatchesLineWidth);
   tstyle_->SetHatchesSpacing(fHatchesSpacing);
   tstyle_->SetFrameFillColor(fFrameFillColor);
   tstyle_->SetFrameLineColor(fFrameLineColor);
   tstyle_->SetFrameFillStyle(fFrameFillStyle);
   tstyle_->SetFrameLineStyle(fFrameLineStyle);
   tstyle_->SetFrameLineWidth(fFrameLineWidth);
   tstyle_->SetFrameBorderSize(fFrameBorderSize);
   tstyle_->SetFrameBorderMode(fFrameBorderMode);
   tstyle_->SetHistFillColor(fHistFillColor);
   tstyle_->SetHistLineColor(fHistLineColor);
   tstyle_->SetHistFillStyle(fHistFillColor);
   tstyle_->SetHistLineStyle(fHistLineColor);
   tstyle_->SetHistLineWidth(fHistLineWidth);
   tstyle_->SetHistMinimumZero(fHistMinimumZero);
   tstyle_->SetCanvasPreferGL(fCanvasPreferGL);
   tstyle_->SetCanvasColor(fCanvasColor);
   tstyle_->SetCanvasBorderSize(fCanvasBorderSize);
   tstyle_->SetCanvasBorderMode(fCanvasBorderMode);
   tstyle_->SetCanvasDefH(fCanvasDefH);
   tstyle_->SetCanvasDefW(fCanvasDefW);
   tstyle_->SetCanvasDefX(fCanvasDefX);
   tstyle_->SetCanvasDefY(fCanvasDefY);
   tstyle_->SetPadColor(fPadColor);
   tstyle_->SetPadBorderSize(fPadBorderSize);
   tstyle_->SetPadBorderMode(fPadBorderMode);
   tstyle_->SetPadBottomMargin(fPadBottomMargin);
   tstyle_->SetPadTopMargin(fPadTopMargin);
   tstyle_->SetPadLeftMargin(fPadLeftMargin);
   tstyle_->SetPadRightMargin(fPadRightMargin);
   tstyle_->SetPadGridX(fPadGridX);
   tstyle_->SetPadGridY(fPadGridY);
   tstyle_->SetPadTickX(fPadTickX);
   tstyle_->SetPadTickY(fPadTickY);
   tstyle_->SetPaperSize(fPaperSizeX, fPaperSizeY);
   tstyle_->SetScreenFactor(fScreenFactor);
   tstyle_->SetStatColor(fStatColor);
   tstyle_->SetStatTextColor(fStatTextColor);
   tstyle_->SetStatBorderSize(fStatBorderSize);
   tstyle_->SetStatFont(fStatFont);
   tstyle_->SetStatFontSize(fStatFontSize);
   tstyle_->SetStatStyle(fStatStyle);
   tstyle_->SetStatFormat(fStatFormat);
   tstyle_->SetStatX(fStatX);
   tstyle_->SetStatY(fStatY);
   tstyle_->SetStatW(fStatW);
   tstyle_->SetStatH(fStatH);
   tstyle_->SetStripDecimals(fStripDecimals);
   tstyle_->SetTitleAlign(fTitleAlign);
   // FIXME
   // tstyle_->SetTitleFillColor(GetTitleFillColor());
   tstyle_->SetTitleTextColor(fTitleTextColor);
   tstyle_->SetTitleBorderSize(fTitleBorderSize);
   tstyle_->SetTitleFont(fTitleFont);
   tstyle_->SetTitleFontSize(fTitleFontSize);
   tstyle_->SetTitleStyle(fTitleStyle);
   tstyle_->SetTitleX(fTitleX);
   tstyle_->SetTitleY(fTitleY);
   tstyle_->SetTitleW(fTitleW);
   tstyle_->SetTitleH(fTitleH);
   tstyle_->SetLegoInnerR(fLegoInnerR);

   tstyle_->SetHeaderPS(fHeaderPS);
   tstyle_->SetTitlePS(fTitlePS);
   tstyle_->SetFitFormat(fFitFormat);
   tstyle_->SetPaintTextFormat(fPaintTextFormat);
   tstyle_->SetLineScalePS(fLineScalePS);
   tstyle_->SetJoinLinePS(fJoinLinePS);
   tstyle_->SetCapLinePS(fCapLinePS);
   tstyle_->SetColorModelPS(fColorModelPS);
   tstyle_->SetTimeOffset(fTimeOffset);

   tstyle_->SetLineColor(fLineColor);
   tstyle_->SetLineStyle(fLineStyle);
   tstyle_->SetLineWidth(GetLineWidth());
   tstyle_->SetFillColor(GetFillColor());
   tstyle_->SetFillStyle(GetFillStyle());
   tstyle_->SetMarkerColor(GetMarkerColor());
   tstyle_->SetMarkerSize(GetMarkerSize());
   tstyle_->SetMarkerStyle(GetMarkerStyle());
   tstyle_->SetTextAlign(GetTextAlign());
   tstyle_->SetTextAngle(GetTextAngle());
   tstyle_->SetTextColor(GetTextColor());
   tstyle_->SetTextFont(GetTextFont());
   tstyle_->SetTextSize(GetTextSize());
}

// -----------------------------------------------------------------------------
void Style::FromTStyle() {
   auto set_axis = [this](TAttAxis& axis, Option_t* o) {
     axis.SetNdivisions(this->tstyle_->GetNdivisions(o));
     axis.SetAxisColor(this->tstyle_->GetAxisColor(o));
     axis.SetLabelColor(this->tstyle_->GetLabelColor(o));
     axis.SetLabelFont(this->tstyle_->GetLabelFont(o));
     axis.SetLabelOffset(this->tstyle_->GetLabelOffset(o));
     axis.SetTickLength(this->tstyle_->GetTickLength(o));
     axis.SetTitleOffset(this->tstyle_->GetTitleOffset(o));
     axis.SetTitleSize(this->tstyle_->GetTitleSize(o));
     axis.SetTitleColor(this->tstyle_->GetTitleColor(o));
     axis.SetTitleFont(this->tstyle_->GetTitleFont(o));
   };
   set_axis(fXaxis, "x"); 
   set_axis(fYaxis, "y"); 
   set_axis(fZaxis, "z"); 

   fBarWidth = tstyle_->GetBarWidth();
   fBarOffset = tstyle_->GetBarOffset();
   fColorModelPS = tstyle_->GetColorModelPS();
   fDrawBorder = tstyle_->GetDrawBorder();
   fOptLogx = tstyle_->GetOptLogx();
   fOptLogy = tstyle_->GetOptLogy();
   fOptLogz = tstyle_->GetOptLogz();
   fOptDate = tstyle_->GetOptDate();
   fOptStat = tstyle_->GetOptStat();
   fOptTitle = tstyle_->GetOptTitle();
   fOptFit = tstyle_->GetOptFit();
   fNumberContours = tstyle_->GetNumberContours();
   fAttDate.SetTextFont(tstyle_->GetAttDate()->GetTextFont());
   fAttDate.SetTextSize(tstyle_->GetAttDate()->GetTextSize());
   fAttDate.SetTextAngle(tstyle_->GetAttDate()->GetTextAngle());
   fAttDate.SetTextAlign(tstyle_->GetAttDate()->GetTextAlign());
   fAttDate.SetTextColor(tstyle_->GetAttDate()->GetTextColor());
   fDateX = tstyle_->GetDateX();
   fDateY= tstyle_->GetDateY();
   fEndErrorSize = tstyle_->GetEndErrorSize();
   fErrorX = tstyle_->GetErrorX();
   fFuncColor= tstyle_->GetFuncColor();
   fFuncStyle= tstyle_->GetFuncStyle();
   fFuncWidth = tstyle_->GetFuncWidth();
   fGridColor= tstyle_->GetGridColor();
   fGridStyle=tstyle_->GetGridStyle();
   fGridWidth=tstyle_->GetGridWidth();
   fLegendBorderSize=tstyle_->GetLegendBorderSize();
   fLegendFillColor=tstyle_->GetLegendFillColor();
   fLegendFont=tstyle_->GetLegendFont();
   fLegendTextSize=tstyle_->GetLegendTextSize();
   fHatchesLineWidth=tstyle_->GetHatchesLineWidth();
   fHatchesSpacing=tstyle_->GetHatchesSpacing();
   fFrameFillColor=tstyle_->GetFrameFillColor();
   fFrameLineColor=tstyle_->GetFrameLineColor();
   fFrameFillStyle=tstyle_->GetFrameFillStyle();
   fFrameLineStyle=tstyle_->GetFrameLineStyle();
   fFrameLineWidth=tstyle_->GetFrameLineWidth();
   fFrameBorderSize=tstyle_->GetFrameBorderSize();
   fFrameBorderMode=tstyle_->GetFrameBorderMode();
   fHistFillColor=tstyle_->GetHistFillColor();
   fHistLineColor=tstyle_->GetHistLineColor();
   fHistFillColor=tstyle_->GetHistFillStyle();
   fHistLineColor=tstyle_->GetHistLineStyle();
   fHistLineWidth=tstyle_->GetHistLineWidth();
   fHistMinimumZero=tstyle_->GetHistMinimumZero();
   fCanvasPreferGL=tstyle_->GetCanvasPreferGL();
   fCanvasColor=tstyle_->GetCanvasColor();
   fCanvasBorderSize=tstyle_->GetCanvasBorderSize();
   fCanvasBorderMode=tstyle_->GetCanvasBorderMode();
   fCanvasDefH=tstyle_->GetCanvasDefH();
   fCanvasDefW=tstyle_->GetCanvasDefW();
   fCanvasDefX=tstyle_->GetCanvasDefX();
   fCanvasDefY=tstyle_->GetCanvasDefY();
   fPadColor=tstyle_->GetPadColor();
   fPadBorderSize=tstyle_->GetPadBorderSize();
   fPadBorderMode=tstyle_->GetPadBorderMode();
   fPadBottomMargin=tstyle_->GetPadBottomMargin();
   fPadTopMargin=tstyle_->GetPadTopMargin();
   fPadLeftMargin=tstyle_->GetPadLeftMargin();
   fPadRightMargin=tstyle_->GetPadRightMargin();
   fPadGridX=tstyle_->GetPadGridX();
   fPadGridY=tstyle_->GetPadGridY();
   fPadTickX=tstyle_->GetPadTickX();
   fPadTickY=tstyle_->GetPadTickY();
   tstyle_->GetPaperSize(fPaperSizeX, fPaperSizeY);
   fScreenFactor=tstyle_->GetScreenFactor();
   fStatColor=tstyle_->GetStatColor();
   fStatTextColor=tstyle_->GetStatTextColor();
   fStatBorderSize=tstyle_->GetStatBorderSize();
   fStatFont=tstyle_->GetStatFont();
   fStatFontSize=tstyle_->GetStatFontSize();
   fStatStyle=tstyle_->GetStatStyle();
   fStatFormat=tstyle_->GetStatFormat();
   fStatX=tstyle_->GetStatX();
   fStatY=tstyle_->GetStatY();
   fStatW=tstyle_->GetStatW();
   fStatH=tstyle_->GetStatH();
   fStripDecimals=tstyle_->GetStripDecimals();
   fTitleAlign=tstyle_->GetTitleAlign();
   // FIXME
   // tstyle_->SetTitleFillColor(GetTitleFillColor());
   fTitleTextColor = tstyle_->GetTitleTextColor();
   fTitleBorderSize=tstyle_->GetTitleBorderSize();
   fTitleFont=tstyle_->GetTitleFont();
   fTitleFontSize=tstyle_->GetTitleFontSize();
   fTitleStyle=tstyle_->GetTitleStyle();
   fTitleX=tstyle_->GetTitleX();
   fTitleY=tstyle_->GetTitleY();
   fTitleW=tstyle_->GetTitleW();
   fTitleH=tstyle_->GetTitleH();
   fLegoInnerR=tstyle_->GetLegoInnerR();

   fHeaderPS=tstyle_->GetHeaderPS();
   fTitlePS=tstyle_->GetTitlePS();
   fFitFormat=tstyle_->GetFitFormat();
   fPaintTextFormat=tstyle_->GetPaintTextFormat();
   fLineScalePS=tstyle_->GetLineScalePS();
   fJoinLinePS=tstyle_->GetJoinLinePS();
   fCapLinePS=tstyle_->GetCapLinePS();
   fColorModelPS=tstyle_->GetColorModelPS();
   fTimeOffset=tstyle_->GetTimeOffset();

   fLineColor=tstyle_->GetLineColor();
   fLineStyle=tstyle_->GetLineStyle();
   SetLineWidth(tstyle_->GetLineWidth());
   SetFillColor(tstyle_->GetFillColor());
   SetFillStyle(tstyle_->GetFillStyle());
   SetMarkerColor(tstyle_->GetMarkerColor());
   SetMarkerSize(tstyle_->GetMarkerSize());
   SetMarkerStyle(tstyle_->GetMarkerStyle());
   SetTextAlign(tstyle_->GetTextAlign());
   SetTextAngle(tstyle_->GetTextAngle());
   SetTextColor(tstyle_->GetTextColor());
   SetTextFont(tstyle_->GetTextFont());
   SetTextSize(tstyle_->GetTextSize());
}

}  // namespace experimental
}  // namespace bdm

