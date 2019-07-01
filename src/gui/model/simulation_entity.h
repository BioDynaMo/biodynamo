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

#include "gui/model/model_element.h"
#include "biodynamo.h"

namespace gui {

/// Represent `simulation objects`
class SimulationEntity {

 public:
  SimulationEntity() {};
  ~SimulationEntity() = default;

  void PrintData() {
    std::cout << "\t\tType:" << "Simulation Entity" << '\n';
  } 

  void SetName(const char* name) {
    fName.assign(name);
  }

  const char* GetName() {
    return fName.c_str();
  }

 private:
  std::string        fName;
  bdm::Cell          fElement;

  ClassDef(SimulationEntity,1)
};

}  // namespace gui

#endif // GUI_SIMULATION_ENTITY_H_