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

#ifndef CORE_UTIL_ROOT_H_
#define CORE_UTIL_ROOT_H_

#if defined(USE_DICT)

#include <Rtypes.h>
#include <TBuffer.h>

#define BDM_CLASS_DEF(class_name, class_version_id) \
  ClassDef(class_name, class_version_id)
#define BDM_CLASS_DEF_NV(class_name, class_version_id) \
  ClassDefNV(class_name, class_version_id)
#define BDM_CLASS_DEF_OVERRIDE(class_name, class_version_id) \
  ClassDefOverride(class_name, class_version_id)

#define BDM_TEMPLATE_CLASS_DEF(class_name, class_version_id)               \
 private:                                                                  \
  static atomic_TClass_ptr fgIsA;                                          \
                                                                           \
 public:                                                                   \
  static TClass* Class() {                                                 \
    throw "This method should have been replaced by the ROOT dictionary."; \
  }                                                                        \
  static const char* Class_Name();                                         \
  static Version_t Class_Version() { return class_version_id; }            \
  static TClass* Dictionary();                                             \
  virtual TClass* IsA() const { return class_name::Class(); }              \
  virtual void ShowMembers(TMemberInspector& insp) const {                 \
    ::ROOT::Class_ShowMembers(class_name::Class(), this, insp);            \
  }                                                                        \
  virtual void Streamer(TBuffer&) {                                        \
    throw "This method should have been replaced by the ROOT dictionary."; \
  }                                                                        \
  void StreamerNVirtual(TBuffer& ClassDef_StreamerNVirtual_b) {            \
    class_name::Streamer(ClassDef_StreamerNVirtual_b);                     \
  }                                                                        \
  static const char* DeclFileName() { return __FILE__; }                   \
  static int ImplFileLine();                                               \
  static const char* ImplFileName();                                       \
  static int DeclFileLine() { return __LINE__; }                           \
                                                                           \
 private:
// NOLINT

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
#else

#include "core/util/log.h"

#define BDM_DICT_ERROR_MSG                            \
  "You tried to use a ROOT dictionary function, but " \
  "compiled BioDynaMo without dictionary support. "   \
  "Please configure with cmake -Ddict=on .. and recompile."

/// Macro that inserts empty definitions of each function used for compilations
/// without dictionaries. \n
/// The compiler won't complain about missing functions. \n
/// However, if ROOT functions are used that rely on dictionaries (e.g. backup)
/// a runtime error will be thrown.
#define BDM_NULL_CLASS_DEF(class_name, class_version_id)        \
 public:                                                        \
  static TClass* Class() {                                      \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
    return nullptr;                                             \
  }                                                             \
  static const char* Class_Name() {                             \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
    return nullptr;                                             \
  }                                                             \
  static Version_t Class_Version() {                            \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
    return class_version_id;                                    \
  }                                                             \
  static TClass* Dictionary() {                                 \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
    return nullptr;                                             \
  }                                                             \
  virtual TClass* IsA() const {                                 \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
    return class_name::Class();                                 \
  }                                                             \
  virtual void ShowMembers(TMemberInspector& insp) const {      \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
  }                                                             \
  virtual void Streamer(TBuffer&) {                             \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
  }                                                             \
  void StreamerNVirtual(TBuffer& ClassDef_StreamerNVirtual_b) { \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
  }                                                             \
  static const char* DeclFileName() {                           \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
    return nullptr;                                             \
  }                                                             \
  static int ImplFileLine() {                                   \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
    return -1;                                                  \
  }                                                             \
  static const char* ImplFileName() {                           \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
    return nullptr;                                             \
  }                                                             \
  static int DeclFileLine() {                                   \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);               \
    return __LINE__;                                            \
  }                                                             \
                                                                \
 private:
// NOLINT

#define BDM_NULL_CLASS_DEF_NV(class_name, class_version_id)                 \
 public:                                                                    \
  static TClass* Class() {                                                  \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                           \
    return nullptr;                                                         \
  }                                                                         \
  static const char* Class_Name() {                                         \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                           \
    return nullptr;                                                         \
  }                                                                         \
  static Version_t Class_Version() {                                        \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                           \
    return class_version_id;                                                \
  }                                                                         \
  static TClass* Dictionary() {                                             \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                           \
    return nullptr;                                                         \
  }                                                                         \
  TClass* IsA() const { return class_name::Class(); }                       \
  void ShowMembers(TMemberInspector& insp) const {                          \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                           \
  }                                                                         \
  void Streamer(TBuffer&) { Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG); } \
  void StreamerNVirtual(TBuffer& ClassDef_StreamerNVirtual_b) {             \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                           \
  }                                                                         \
  static const char* DeclFileName() {                                       \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                           \
    return nullptr;                                                         \
  }                                                                         \
  static int ImplFileLine() {                                               \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                           \
    return -1;                                                              \
  }                                                                         \
  static const char* ImplFileName() {                                       \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                           \
    return nullptr;                                                         \
  }                                                                         \
  static int DeclFileLine() {                                               \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                           \
    return __LINE__;                                                        \
  }                                                                         \
                                                                            \
 private:
// NOLINT

#define BDM_NULL_CLASS_DEF_OVERRIDE(class_name, class_version_id) \
 public:                                                          \
  static TClass* Class() {                                        \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
    return nullptr;                                               \
  }                                                               \
  static const char* Class_Name() {                               \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
    return nullptr;                                               \
  }                                                               \
  static Version_t Class_Version() {                              \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
    return class_version_id;                                      \
  }                                                               \
  static TClass* Dictionary() {                                   \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
    return nullptr;                                               \
  }                                                               \
  TClass* IsA() const override {                                  \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
    return class_name::Class();                                   \
  }                                                               \
  void ShowMembers(TMemberInspector& insp) const override {       \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
  }                                                               \
  void Streamer(TBuffer&) override {                              \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
  }                                                               \
  void StreamerNVirtual(TBuffer& ClassDef_StreamerNVirtual_b) {   \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
  }                                                               \
  static const char* DeclFileName() {                             \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
    return nullptr;                                               \
  }                                                               \
  static int ImplFileLine() {                                     \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
    return -1;                                                    \
  }                                                               \
  static const char* ImplFileName() {                             \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
    return nullptr;                                               \
  }                                                               \
  static int DeclFileLine() {                                     \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                 \
    return __LINE__;                                              \
  }                                                               \
                                                                  \
 private:
// NOLINT

/// Forward all calls to BDM_NULL_CLASS_DEF
#define BDM_CLASS_DEF(class_name, class_version_id) \
  BDM_NULL_CLASS_DEF(class_name, class_version_id)
#define BDM_CLASS_DEF_NV(class_name, class_version_id) \
  BDM_NULL_CLASS_DEF_NV(class_name, class_version_id)
#define BDM_CLASS_DEF_OVERRIDE(class_name, class_version_id) \
  BDM_NULL_CLASS_DEF_OVERRIDE(class_name, class_version_id)
#define BDM_TEMPLATE_CLASS_DEF(class_name, class_version_id) \
  BDM_NULL_CLASS_DEF(class_name, class_version_id)

#endif  // defined(USE_DICT)

#endif  // CORE_UTIL_ROOT_H_
