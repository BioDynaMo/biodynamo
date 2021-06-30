---
title: "Release instructions"
date: "2020-04-29"
path: "/docs/devguide/release/"
meta_title: "BioDynaMo Dev Guide"
meta_description: "Instructions on how to release a new BioDynaMo version."
toc: true
image: ""
sidebar: "devguide"
keywords:
  -release
---

These instructions show the necessary steps to release a new BioDynaMo version.
The following steps demonstrate the 1.0 release

Production releases have an even minor version number (e.g. 1.0, 1.02, 1.04, ...). 
Development versions use odd minor version numbers.

* Update the version in `CMakeLists.txt` including a "0" as patch version number
  ```
  project(BioDynaMo LANGUAGES VERSION "1.0.0")
  ```
  If the project is built without a github repository, the automatic version number generation does not work. This is the fallback solution.

* Create release notes in file `doc/user_guide/release_notes_v1.0.md`
  and add the page to `doc/sidebars/userguide.yaml`
  Use the following command to obtain a list of contributors
  `git shortlog -sn --no-merges v0.8..HEAD`, where `v0.8` is the last production release. The list is ordered by the number of contributions.

* Consider writing a blog post

* Create a new commit (commit message: "Release v1.0") with these changes, run it trough CI and add it to the master branch if all lights are green.

* Go to BioDynaMo's github project and create a new release
  ```
  Tag version: v1.0
  Release Title: BioDynaMo 1.0
  Description: <insert the release notes here> 
  ```

* Create a new branch `v1.0-patches` branching off from the release commit and push it.

* Update the one-line install script in file `util/install`
  ``` 
   BDM_BRANCH=v1.0-patches
  ```
  and commit it directly to master (commit message "Update branch of one-line install script").

Now we finished releasing `1.0`.
The only part missing now is to tag the development release (`v1.01`).

* Update the version in `CMakeLists.txt` including a "0" as patch version number
  ```
  project(BioDynaMo LANGUAGES VERSION "1.01.0")
  ```

* Create a new commit (commit message: "Release development version 1.01") and add it to the master branch.

* Create a tag `v1.01` for the previous commit and push it to github
  ```
  git tag v1.01
  git push origin v1.01 
  ```

Now we have successfully released `v1.0` and prepared the repository for new developments.
The one-line install script will install the latest production release with all hotfixes in the corresponding patch branch. Commits for the next release will be added to the `master` branch. 
Commits to the patch branch should always be cherry picked. Do not rebase it on master beyond the development tag! Otherwise the automatic versioning will be wrong.
