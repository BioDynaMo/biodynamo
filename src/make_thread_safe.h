#ifndef MAKE_THREAD_SAFE_H_
#define MAKE_THREAD_SAFE_H_

#include <memory>
#include <vector>
#include "cell.h"

namespace bdm {

/// \brief `make_thread_safe` enables thread safe access of different elements
/// within a SOA container.
///
/// By default SOA container are not thread safe. They all share the same `idx_`
/// data member. `make_thread_safe` creates a reference object with its own
/// `idx_`.
inline std::unique_ptr<SoaCellRef> make_thread_safe(SoaCell* container) {
  return std::unique_ptr<SoaCellRef>(new SoaCellRef(container, 0));
}

/// Accesses to AOS containers are by default thread safe.
/// Therefore, the purpose for this function overload is to provide a uniform
/// API for SOA and AOS memory layouts.
inline std::vector<Cell>* make_thread_safe(std::vector<Cell>* container) {
  return container;
}

}  // namespace bdm

#endif  // MAKE_THREAD_SAFE_H_
