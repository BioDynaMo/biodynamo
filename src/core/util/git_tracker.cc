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

#include "git_tracker.h"
#include "core/stdfilesystem.h"
#include "core/util/log.h"
#include "fstream"
#include "git2.h"
#include "iostream"
#include "stdio.h"

// C style callback function for libgit2 - taken from the libgit2 examples.
// See common.h in the libgit2 examples for more information. Definition
// of the callback function at the bottom of this file.
int diff_output(const git_diff_delta* d, const git_diff_hunk* h,
                const git_diff_line* l, void* p);

namespace bdm {

void GitTracker::SaveGitDetails() {
  // Determine the absolute path to the BioDynaMo installation
  bdm_installation_ = GetAbsolutePath(bdm_installation_);
  // Determine the absolute path to the simulation output
  simulation_output_ = GetAbsolutePath(simulation_output_);

  // Write the git info for the BioDynaMo installation
  std::string filename = simulation_output_ + "/bdm_git_info.txt";
  SaveGitInfo(filename, bdm_installation_);

  // Write the git info for the simulation output
  filename = simulation_output_ + "/bdm_sim_git_info.txt";
  SaveGitInfo(filename, cwd_);

  // Write the git diff for the BioDynaMo installation
  filename = simulation_output_ + "/bdm_git_diff.patch";
  SaveGitDiff(filename, bdm_installation_);

  // Write the git diff for the simulation output
  filename = simulation_output_ + "/bdm_sim_git_diff.patch";
  SaveGitDiff(filename, cwd_);
}

void GitTracker::SaveGitInfo(const std::string& file,
                             const std::string& repository_path) const {
  std::ofstream out(file);
  PrintGitInfo(repository_path, out);
}

void GitTracker::PrintGitInfo(const std::string& repository_path,
                              std::ostream& out) const {
  // Absolute path to repository
  std::string abs_repo_path = GetAbsolutePath(repository_path);

  // Use libgit2 to verify that the repository is valid
  git_libgit2_init();
  git_repository* repo = nullptr;
  git_repository_open(&repo, abs_repo_path.c_str());
  if (repo == nullptr) {
    Log::Error("GitTracker", "Error: ", abs_repo_path,
               " is not a valid git repository");
    return;
  }

  // Get the name of the repository via libgit2
  const char* repo_name = git_repository_path(repo);

  // Get the commit hash
  git_reference* head = nullptr;
  git_reference_lookup(&head, repo, "HEAD");
  git_object* head_commit = nullptr;
  git_reference_peel(&head_commit, head, GIT_OBJ_COMMIT);
  const git_oid* oid = git_commit_id((git_commit*)head_commit);
  char oid_str[GIT_OID_HEXSZ + 1];
  git_oid_tostr(oid_str, GIT_OID_HEXSZ + 1, oid);

  // Get the commit message
  const char* commit_message = git_commit_message((git_commit*)head_commit);

  // Get the commit author
  const git_signature* commit_author =
      git_commit_author((git_commit*)head_commit);

  // Get the commit author name
  const char* author_name = commit_author->name;

  // Get the commit author email
  const char* author_email = commit_author->email;

  // Get the branch name
  const char* branch_name = git_reference_shorthand(head);

  // Get reference target
  git_reference* target = nullptr;
  git_reference_resolve(&target, head);
  const char* target_name = git_reference_name(target);

  // Print the git info
  out << "Git repository     : " << repo_name << "\n"
      << "Git branch name    : " << target_name << "\n"
      << "                   : " << branch_name << "\n"
      << "Git commit hash    : " << oid_str << "\n"
      << "Git author email   : " << author_email << "\n"
      << "Git author name    : " << author_name << "\n"
      << "Git commit message : " << commit_message << "\n"
      << std::endl;

  // Cleanup
  git_object_free(head_commit);
  git_reference_free(head);
  git_reference_free(target);
  git_repository_free(repo);
  git_libgit2_shutdown();
}

void GitTracker::SaveGitDiff(const std::string& file,
                             const std::string& repository_path) const {
  // Absolute path to repository
  std::string abs_repo_path = GetAbsolutePath(repository_path);

  // Use libgit2 to verify that the repository is valid
  git_libgit2_init();
  git_repository* repo = nullptr;
  git_repository_open(&repo, abs_repo_path.c_str());
  if (repo == nullptr) {
    Log::Error("GitTracker", "Error: ", abs_repo_path,
               " is not a valid git repository");
    return;
  }

  // Generate the git diff for the repository
  git_diff* diff = nullptr;
  git_diff_index_to_workdir(&diff, repo, nullptr, nullptr);

  // Open C style file
  FILE* fp = fopen(file.c_str(), "w");

  // Print all the deltas to the file. If fopen fails, the diff will be printed
  // to stdout
  git_diff_print(diff, GIT_DIFF_FORMAT_PATCH, diff_output, fp);

  // Close C style file if the pointer is not null
  if (fp) {
    fclose(fp);
  }

  // Cleanup
  git_diff_free(diff);
  git_repository_free(repo);
  git_libgit2_shutdown();
}

std::string GitTracker::GetAbsolutePath(const std::string& path) const {
  // Get absolute path via filesystem
  return fs::absolute(path).string();
};

void GitTracker::ConstructFolderNames() {
  // Determine the current working directory via filesystem
  cwd_ = fs::current_path();
  // Trim the build directory from the current working directory if the string
  // ends with build
  if (cwd_.substr(cwd_.size() - 5, 5) == "build") {
    cwd_ = cwd_.substr(0, cwd_.size() - 6);
  }
  // Load the environment variable BDMSYS
  bdm_installation_ = std::getenv("BDMSYS");
  // Determine the absolute path to the BioDynaMo installation
  bdm_installation_ = GetAbsolutePath(bdm_installation_);
  // Trim the build directory from the BDMSYS environment variable
  bdm_installation_ = fs::path(bdm_installation_).parent_path().string();
};
}  // namespace bdm

// This function is taken from the libgit2 documentation. It is used to print
// the git diff to a file.
int diff_output(const git_diff_delta* d, const git_diff_hunk* h,
                const git_diff_line* l, void* p) {
  FILE* fp = (FILE*)p;

  (void)d;
  (void)h;

  if (!fp)
    fp = stdout;

  if (l->origin == GIT_DIFF_LINE_CONTEXT ||
      l->origin == GIT_DIFF_LINE_ADDITION ||
      l->origin == GIT_DIFF_LINE_DELETION)
    fputc(l->origin, fp);

  fwrite(l->content, 1, l->content_len, fp);

  return 0;
}

#endif  // USE_LIBGIT2
