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

#ifndef GUI_SIMULATION_ENTITY_H_
#define GUI_SIMULATION_ENTITY_H_

#include <TClass.h>
#include <TEnv.h>
#include <TFile.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TVirtualX.h>
#include <TMethod.h>
#include <TMethodArg.h>
#include <TMethodCall.h>

#include "biodynamo.h"
#include "core/container/math_array.h"

namespace gui {

/// Represent `simulation objects`
class SimulationEntity {

 public:
  SimulationEntity() {
    fElement = new bdm::Cell(10.0);
    fSecretion = kFALSE;
  };
  ~SimulationEntity() = default;

  void PrintData() {
    std::cout << "\t\tType:" << "Simulation Entity" << '\n';
    std::cout << "\t\tName:" << fName << '\n';
  } 

  void SetName(const char* name) {
    fName.assign(name);
  }

  const char* GetName() {
    return fName.c_str();
  }

  bdm::Cell* GetElement() {
    return fElement;
  }

  void SetPosition(bdm::Double3 pos) {
    fPosition = pos;
    Log::Debug("In simulation_entity: Setting position to:", pos[0], ", ", pos[1], ", ", pos[2]);
    Log::Debug("Sanity check position to:", fPosition[0], ", ", fPosition[1], ", ", fPosition[2]);
  }

  bdm::Double3 GetPosition() {
    Log::Debug("In simulation_entity: Getting position:", fPosition[0], ", ", fPosition[1], ", ", fPosition[2]);
    return fPosition;
  }

  void SetTractorForce(bdm::Double3 force) {
    fTractorForce = force;
  }

  bdm::Double3 GetTractorForce() {
    return fTractorForce;
  }

  void SetSecretion(Bool_t flag) {
    fSecretion = flag;
  }

  Bool_t GetSecretion() {
    return fSecretion;
  }

  double GetAttributeValDouble(const char* attributeName) {
    bdm::Cell* cellPtr = fElement;
    TClass *cl = cellPtr->IsA();
    std::string methodName(attributeName);
    methodName.insert(0, "Get");
    TMethodCall call(cl, methodName.c_str(), "");

    Double_t retValDouble = -1;
    call.Execute((void*)cellPtr, nullptr, 0, &retValDouble);
    return retValDouble;
  }

  uint32_t GetAttributeValUInt(const char* attributeName) {
    bdm::Cell* cellPtr = fElement;
    TClass *cl = cellPtr->IsA();
    std::string methodName(attributeName);
    methodName.insert(0, "Get");
    TMethodCall call(cl, methodName.c_str(), "");

    uint32_t retValUInt = 0;
    call.Execute((void*)cellPtr, nullptr, 0, &retValUInt);
    return retValUInt;
  }

 private:
  std::string        fName;
  bdm::Cell*         fElement;
  Bool_t             fSecretion;

  // Setting within element does not work, 
  //   so we use these vars
  bdm::Double3            fPosition;
  bdm::Double3            fTractorForce;

  ClassDefNV(SimulationEntity,1)
};

}  // namespace gui

#endif // GUI_SIMULATION_ENTITY_H_