#ifndef VERSION_H_
#define VERSION_H_

/// Version string as obtained from `git describe --tags`
/// e.g.: v0.1.0-10-g91aade4
/// vMAJOR.MINOR.PATH-ADDITIONAL_COMMITS-gCOMMIT_ID
#define BDM_RELEASE "@VERSION@"

/// These macros can be used in the following way:
///
///    #if BDM_VERSION_CODE >= BDM_VERSION(1,0,0,0)
///       #include <newheader.h>
///    #else
///       #include <oldheader.h>
///    #endif
#define BDM_VERSION(major, minor, patch, additional_commits) \
  (static_cast<uint64_t>(major) << 48) +                     \
      (static_cast<uint64_t>(minor) << 32) +                 \
      (static_cast<uint64_t>(patch) << 16) +                 \
      static_cast<uint64_t>(additional_commits)
#define BDM_VERSION_CODE                                         \
  BDM_VERSION(@VERSION_MAJOR@, @VERSION_MINOR@, @VERSION_PATCH@, \
              @VERSION_ADDITIONAL_COMMITS@)

namespace bdm {

class Version {
 public:
  /// Returns version string as obtained from `git describe --tags`
  /// e.g.: v0.1.0-10-g91aade4`
  /// vMAJOR.MINOR.PATH-ADDITIONAL_COMMITS-gCOMMIT_ID
  static const char *String() { return "@VERSION@"; }
  static uint64_t Code() { return BDM_VERSION_CODE; }
};

}  // namespace bdm

#endif  // VERSION_H_
