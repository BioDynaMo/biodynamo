// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#ifdef USE_LIBGIT2
#ifndef GIT_TRACKER_H_
#define GIT_TRACKER_H_

#include <iostream>
#include <string>

namespace bdm {

/// @brief GitTracker class to save git information of the BioDynaMo
/// installation and the derived simulation.
class GitTracker {
 public:
  GitTracker() { ConstructFolderNames(); };
  GitTracker(const std::string& simulation_output) : GitTracker() {
    simulation_output_ = GetAbsolutePath(simulation_output);
  }
  ~GitTracker() = default;

  /// @brief  Saves the git info of the BioDynaMo installation and the derived
  /// simulation as well as their diffs. Places the files in the simulation
  /// output folder.
  void SaveGitDetails();

  /// @brief  Saves the git info of the given repository to the given file
  /// @param file absolute path to the info file
  /// @param repository_path absolute path to the repository
  void SaveGitInfo(const std::string& file, const std::string& repository_path);

  /// @brief Print the git info of the given repository to the given stream
  /// @param repository_path path to the repository
  /// @param out stream to which the git info is printed
  void PrintGitInfo(const std::string& repository_path,
                    std::ostream& out = std::cout);

  /// @brief  Saves the git diff of the given repository to the given file
  /// @param file absolute path to the diff file
  /// @param repository_path absolute path to the repository
  void SaveGitDiff(const std::string& file, const std::string& repository_path);

  /// Sets the path to the BioDynaMo installation
  void SetBdmInstallation(const std::string& path) {
    bdm_installation_ = GetAbsolutePath(path);
  }

  /// Sets the path to the simulation output
  void SetSimulationOutput(const std::string& path) {
    simulation_output_ = GetAbsolutePath(path);
  }

  /// Sets the current working directory (location where the command was
  /// executed)
  void SetCwd(const std::string& path) { cwd_ = GetAbsolutePath(path); }

 private:
  /// Returns the absolute path of the given path (via experimantal/filesystem).
  std::string GetAbsolutePath(const std::string& path);

  /// Constructs the folder names for the bdm_installation_ and the cwd_.
  void ConstructFolderNames();

  /// Path to the BioDynaMo installation
  std::string bdm_installation_ = "";
  /// Path to the simulation output
  std::string simulation_output_ = "";
  /// Current working directory (location where the command was executed)
  std::string cwd_ = "";
};

}  // namespace bdm

#endif  // GIT_TRACKER_H_
#endif  // USE_LIBGIT2
