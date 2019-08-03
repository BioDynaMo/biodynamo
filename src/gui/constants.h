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

#ifndef GUI_CONSTANTS_H
#define GUI_CONSTANTS_H

// width and height of main window frame in pixels
#define MAIN_WIDTH   1280 
#define MAIN_HEIGHT  720
#define GUI_VERSION  "0.1"

namespace gui {

enum View {
  ///-------MENU-------///
  M_FILE_NEWPROJECT         = 101,
  M_FILE_OPENPROJECT        = 102,
  M_FILE_SAVE               = 103,
  M_FILE_SAVEAS             = 104,
  M_FILE_IMPORT             = 105,
  M_FILE_EXPORT             = 106,
  M_FILE_PREFERENCES        = 107,
  M_FILE_EXIT               = 108,
  M_SIMULATION_GENERATE     = 201,
  M_SIMULATION_BUILD        = 202,
  M_SIMULATION_RUN          = 203,
  M_SIMULATION_OPENPARAVIEW = 204,
  M_SAMPLES_CELLDIVISION    = 301,
  M_SAMPLES_DIFFUSION       = 302,
  M_SAMPLES_GENEREGULATION  = 303,
  M_SAMPLES_SOMACLUSTERING  = 304,
  M_SAMPLES_TUMORCONCEPT    = 305,
  M_TOOLS_STARTBROWSER      = 401,
  M_VIEW_TOOLBAR            = 501,
  M_HELP_USERGUIDE          = 601,
  M_HELP_DEVGUIDE           = 602,
  M_HELP_LICENSE            = 603,
  M_HELP_ABOUT              = 604,
  
  ///----SELECTION-FRAME-----///
  M_MODEL_NEW               = 701,
  M_MODEL_SIMULATE          = 801,
  M_CREATE_GRID             = 802,
  M_ALL_ACTIVE              = 803,
  M_NONE_ACTIVE             = 804,
  
  ///-----MODEL-FRAME-------///
  M_ENTITY_CELL             = 901,
  M_MODULE_GROWTH           = 1001,
  M_MODULE_CHEMOTAXIS       = 1002,
  M_MODULE_SUBSTANCE        = 1003,
  M_GENERAL_VARIABLE        = 1101,
  M_GENERAL_FUNCTION        = 1102,
  M_GENERAL_FORMULA         = 1103,

  ///-----VIS-FRAME---------///
  M_ZOOM_PLUS               = 1201,
  M_ZOOM_MINUS              = 1202
};

}  // namespace gui


#endif // GUI_CONSTANTS_H
