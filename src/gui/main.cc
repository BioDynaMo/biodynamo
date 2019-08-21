// Author: Lukasz Stempniewicz 21/08/19

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

#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <TApplication.h>
#include "gui/view/model_creator.h"
#include "gui/constants.h"
#include "gui/view/log.h"

void Gui()
{
  new gui::ModelCreator(gClient->GetRoot(), MAIN_WIDTH, MAIN_HEIGHT);
}

//---- Main program ------------------------------------------------------------
int main(int argc, char **argv)
{
  if (argc != 2) {
   throw std::invalid_argument("Invalid number of args, please supply log file location");
  }
  gui::Log::SetLogFile(argv[1]);

  TApplication theApp("App", &argc, argv);
  Gui();
  theApp.Run();
  return 0;
}