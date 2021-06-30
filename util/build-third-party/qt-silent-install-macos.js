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

function Controller() {
  installer.autoRejectMessageBoxes();
  installer.installationFinished.connect(function() {
      gui.clickButton(buttons.NextButton);
  })
}

Controller.prototype.WelcomePageCallback = function() {
  gui.clickButton(buttons.NextButton, 2000);
}

Controller.prototype.CredentialsPageCallback = function() {
  gui.clickButton(buttons.NextButton);
}

Controller.prototype.IntroductionPageCallback = function() {
  gui.clickButton(buttons.NextButton);
}

Controller.prototype.TargetDirectoryPageCallback = function() {
  gui.currentPageWidget().TargetDirectoryLineEdit.setText(installer.value("HomeDir") + "/bdm-build-third-party/qt");
  gui.clickButton(buttons.NextButton);
}

Controller.prototype.ComponentSelectionPageCallback = function() {
  var widget = gui.currentPageWidget();

  widget.deselectAll();
  widget.selectComponent("qt.qt5.5110.clang_64");

  gui.clickButton(buttons.NextButton);
}

Controller.prototype.LicenseAgreementPageCallback = function() {
  gui.currentPageWidget().AcceptLicenseRadioButton.setChecked(true);
  gui.clickButton(buttons.NextButton);
}

Controller.prototype.StartMenuDirectoryPageCallback = function() {
  gui.clickButton(buttons.NextButton);
}

Controller.prototype.ReadyForInstallationPageCallback = function() {
  gui.clickButton(buttons.NextButton);
}

Controller.prototype.FinishedPageCallback = function() {
  var checkBoxForm = gui.currentPageWidget().LaunchQtCreatorCheckBoxForm
  if (checkBoxForm && checkBoxForm.launchQtCreatorCheckBox) {
    checkBoxForm.launchQtCreatorCheckBox.checked = false;
  }
  gui.clickButton(buttons.FinishButton);
}
