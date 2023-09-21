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
#ifndef CORE_STDFILESYSTEM_H_
#define CORE_STDFILESYSTEM_H_

// Use this header only in source files, not other headers

// Hide headers from Cling
#if (!defined(__CLING__) && !defined(__ROOTCLING__))

#ifdef __APPLE__
#ifdef _LIBCPP_DEPRECATED_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::__fs::filesystem;
#endif
#else
#include <filesystem>
namespace fs = std::experimental::filesystem;
#endif

#endif  // (!defined(__CLING__) && !defined(__ROOTCLING__))

#endif  // CORE_STDFILESYSTEM_H_
