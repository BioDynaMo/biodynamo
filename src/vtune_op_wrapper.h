#ifndef VTUNE_OP_WRAPPER_H_
#define VTUNE_OP_WRAPPER_H_

#include <iostream>
#include <ittnotify.h>

namespace bdm {

template <typename Op>
class VTuneOpWrapper : public Op {
 public:
  /// perfect forwarding ctor
  template <typename... Args>
  explicit VTuneOpWrapper(Args&&... args) : Op{std::forward<Args>(args)...} {
    domain_ = __itt_domain_create("MyTraces.MyDomain");
    task_ = __itt_string_handle_create(typeid(Op).name());
  }

  template <typename daosoa>
  void Compute(daosoa* cells) const {
    __itt_task_begin(domain_, __itt_null, __itt_null, task_);
    Op::Compute(cells);
    __itt_task_end(domain_);
  }

 private:
  __itt_domain* domain_;
  __itt_string_handle* task_;
};

}  // namespace bdm

#endif  // VTUNE_OP_WRAPPER_H_
