#ifndef MAKE_THREAD_SAFE_H_
#define MAKE_THREAD_SAFE_H_

#include <memory>
#include "daosoa.h"

namespace bdm {

/// \brief `make_thread_safe` enables thread safe access of different elements
/// within a SOA container.
///
/// By default SOA container are not thread safe. They all share the same `idx_`
/// data member. `make_thread_safe` creates a reference object with its own
/// `idx_`.
template <typename T>
std::unique_ptr<typename T::template Self<VcSoaRefBackend>> make_thread_safe(
    T* container) {
  return container->GetSoaRef();
}

/// Accesses to AOSOA containers are by default thread safe.
/// Therefore, the purpose for this function overload is to provide a uniform
/// API for SOA and AOSOA memory layouts.
template <typename T>
bdm::daosoa<T>* make_thread_safe(bdm::daosoa<T>* container) {
  return container;
}

/// const version for SOA containers
template <typename T>
const typename T::template Self<VcSoaRefBackend> make_thread_safe(
    const T& container) {
  return container.GetSoaRef();
}

/// const version for AOSOA containers
template <typename T>
const bdm::daosoa<T>& make_thread_safe(const bdm::daosoa<T>& container) {
  return container;
}

}  // namespace bdm

#endif  // MAKE_THREAD_SAFE_H_
