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

/// ROOT's BDM_CLASS_DEF macro had to be modified due to an issue with SoaRef
/// backend. Since all data members are references, they cannot be initialized
/// from ROOT I/O. As a consequence, classes with a `SoaRef` backend are
/// excluded from dictionary generation. With the default ROOT `BDM_CLASS_DEF`
/// macro
/// this would lead to a linking error, because there is no implementation for
/// the methods `Class()` and `Streamer()`. To mitigate this problem, this
/// macro inserts a default implementation of these methods. For other backends,
/// these implementation is replaced by the dictionary using template
/// specialization. See also ROOT-8784
/// @param class_name class name without template specifier. e.g. \n
///        `class Foo {};` \n
///         -> class_name: `Foo` \n
///        `template <typename T> class Foo {};` \n
///         -> class_name: `Foo` \n
/// @param class_version_id required for ROOT I/O (see ROOT BDM_CLASS_DEF
/// Macro).
///        Every time the layout of the class is changed, class_version_id
///        must be incremented by one. The class_version_id should be greater
///        or equal to 1.
#define BDM_ROOT_CLASS_DEF(class_name, class_version_id)                   \
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

/// See documentation of BDM_ROOT_CLASS_DEF.
#define BDM_ROOT_CLASS_DEF_OVERRIDE(class_name, class_version_id)          \
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
  TClass* IsA() const override { return class_name::Class(); }             \
  void ShowMembers(TMemberInspector& insp) const override {                \
    ::ROOT::Class_ShowMembers(class_name::Class(), this, insp);            \
  }                                                                        \
  void Streamer(TBuffer&) override {                                       \
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

/// This modification makes it possible to use templated classes with custom
/// streamers without the generation of a dictionary for template instantiations
/// that do not require I/O. Using ROOT's BDM_CLASS_DEF macro would lead to a
/// linking
/// error since the function Class would not be defined.
#define BDM_TEMPLATE_CLASS_DEF_CUSTOM_STREAMER(class_name, class_version_id) \
 private:                                                                    \
  static atomic_TClass_ptr fgIsA;                                            \
                                                                             \
 public:                                                                     \
  static TClass* Class() {                                                   \
    throw "This method should have been replaced by the ROOT dictionary.";   \
  }                                                                          \
  static const char* Class_Name();                                           \
  static Version_t Class_Version() { return class_version_id; }              \
  static TClass* Dictionary();                                               \
  TClass* IsA() const { return class_name::Class(); }                        \
  void ShowMembers(TMemberInspector& insp) const {                           \
    ::ROOT::Class_ShowMembers(class_name::Class(), this, insp);              \
  }                                                                          \
  void Streamer(TBuffer&);                                                   \
  void StreamerNVirtual(TBuffer& ClassDef_StreamerNVirtual_b) {              \
    class_name::Streamer(ClassDef_StreamerNVirtual_b);                       \
  }                                                                          \
  static const char* DeclFileName() { return __FILE__; }                     \
  static int ImplFileLine();                                                 \
  static const char* ImplFileName();                                         \
  static int DeclFileLine() { return __LINE__; }                             \
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

#define BDM_NULL_CLASS_DEF_CUSTOM_STREAMER(class_name, class_version_id) \
 public:                                                                 \
  static TClass* Class() {                                               \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                        \
    return nullptr;                                                      \
  }                                                                      \
  static const char* Class_Name() {                                      \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                        \
    return nullptr;                                                      \
  }                                                                      \
  static Version_t Class_Version() {                                     \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                        \
    return class_version_id;                                             \
  }                                                                      \
  static TClass* Dictionary() {                                          \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                        \
    return nullptr;                                                      \
  }                                                                      \
  TClass* IsA() const {                                                  \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                        \
    return class_name::Class();                                          \
  }                                                                      \
  void ShowMembers(TMemberInspector& insp) const {                       \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                        \
  }                                                                      \
  void Streamer(TBuffer&);                                               \
  void StreamerNVirtual(TBuffer& ClassDef_StreamerNVirtual_b) {          \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                        \
  }                                                                      \
  static const char* DeclFileName() {                                    \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                        \
    return nullptr;                                                      \
  }                                                                      \
  static int ImplFileLine() {                                            \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                        \
    return -1;                                                           \
  }                                                                      \
  static const char* ImplFileName() {                                    \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                        \
    return nullptr;                                                      \
  }                                                                      \
  static int DeclFileLine() {                                            \
    Log::Fatal("Dictionary", BDM_DICT_ERROR_MSG);                        \
    return __LINE__;                                                     \
  }                                                                      \
                                                                         \
 private:
// NOLINT

/// Forward all calls to BDM_NULL_CLASS_DEF
#define BDM_CLASS_DEF_NV(class_name, class_version_id) \
  BDM_NULL_CLASS_DEF_NV(class_name, class_version_id)
#define BDM_CLASS_DEF(class_name, class_version_id) \
  BDM_NULL_CLASS_DEF(class_name, class_version_id)
#define BDM_ROOT_CLASS_DEF(class_name, class_version_id) \
  BDM_NULL_CLASS_DEF(class_name, class_version_id)
#define BDM_ROOT_CLASS_DEF_OVERRIDE(class_name, class_version_id) \
  BDM_NULL_CLASS_DEF(class_name, class_version_id)
#define BDM_TEMPLATE_CLASS_DEF_CUSTOM_STREAMER(class_name, class_version_id) \
  BDM_NULL_CLASS_DEF_CUSTOM_STREAMER(class_name, class_version_id)

#endif  // defined(USE_DICT)

#endif  // CORE_UTIL_ROOT_H_
