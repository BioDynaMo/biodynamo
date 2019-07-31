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

#ifndef GUI_MODULE_H_
#define GUI_MODULE_H_

#include "biodynamo.h"

namespace gui {

/// Model actions (list of ModelAction)
class Module {

 public:
  Module() {};
  ~Module() = default;

  void PrintData() {
    std::cout << "\t\tType:" << "Module" << '\n';
  }

  void SetName(const char* name) {
    fName.assign(name);
  }

  const char* GetName() {
    return fName.c_str();
  }

 private:
  std::string            fName;
  bdm::GrowDivide        fElement;
  
  ClassDefNV(Module,1)
};

}  // namespace gui

#endif // GUI_MODULE_H_