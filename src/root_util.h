#ifndef ROOT_UTIL_H_
#define ROOT_UTIL_H_

#include <Rtypes.h>
#include <TBuffer.h>

/// ROOT's ClassDef macro had to be modified due to an issue with SoaRef
/// backend. Since all data members are references, they cannot be initialized
/// from ROOT I/O. As a consequence, classes with a `SoaRef` backend are
/// excluded from dictionary generation. With the default ROOT `ClassDef` macro
/// this would need to a linking error, because there is no implementation for
/// the methods `Class()` and `Streamer()`. To mitigate this problem, this
/// macro inserts a default implementation of these methods. For other backends,
/// these implementation is replaced by the dictionary using template
/// specialization. See also ROOT-8784
/// @param class_name class name without template specifier. e.g. \n
///        `class Foo {};` \n
///         -> class_name: `Foo` \n
///        `template <typename T> class Foo {};` \n
///         -> class_name: `Foo` \n
/// @param class_version_id required for ROOT I/O (see ROOT ClassDef Macro).
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

// TODO(lukas) document
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
  virtual TClass* IsA() const { return class_name::Class(); }                \
  virtual void ShowMembers(TMemberInspector& insp) const {                   \
    ::ROOT::Class_ShowMembers(class_name::Class(), this, insp);              \
  }                                                                          \
  virtual void Streamer(TBuffer&);                                           \
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

#endif  // ROOT_UTIL_H_
