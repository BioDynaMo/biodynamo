// Define empty function bodies if Vtune wasn't found on your system.
// Functions will emit a warning on the first invocation.

#ifndef VTUNE_H_
#define VTUNE_H_

#ifdef USE_VTUNE
#include <ittnotify.h>
#else

#include "log.h"

/// This macro should be inserted in the function body of each wrapper.
/// It emits a Warning the first time it is called.
#define BDM_VTUNE_LOG_WARNING()                                          \
  static bool warning_logged = false;                                    \
  if (!warning_logged) {                                                 \
    bdm::Log::Warning(                                                   \
        "Vtune",                                                         \
        "Vtune was not found on your system. Call to %s won't have any " \
        "effect",                                                        \
        __FUNCTION__);                                                   \
    warning_logged = true;                                               \
  }

struct __itt_domain {};         // NOLINT
struct __itt_string_handle {};  // NOLINT
struct __itt_null_t {};         // NOLINT

static constexpr __itt_null_t __itt_null;  // NOLINT

inline __itt_domain* __itt_domain_create(const char*) {  // NOLINT
  BDM_VTUNE_LOG_WARNING();
  return nullptr;
}

inline __itt_string_handle* __itt_string_handle_create(const char*) {  // NOLINT
  BDM_VTUNE_LOG_WARNING();
  return nullptr;
}

inline void __itt_task_begin(__itt_domain*, __itt_null_t,  // NOLINT
                             __itt_null_t, __itt_string_handle*) {
  BDM_VTUNE_LOG_WARNING();
}

inline void __itt_task_end(__itt_domain*) {  // NOLINT
  BDM_VTUNE_LOG_WARNING();
}  // NOLINT

inline void __itt_pause() { BDM_VTUNE_LOG_WARNING(); }  // NOLINT

inline void __itt_resume() { BDM_VTUNE_LOG_WARNING(); }  // NOLINT

#endif  // USE_VTUNE

#endif  // VTUNE_H_
