#ifndef CORE_OPERATION_OPERATION_REGISTRY_H_
#define CORE_OPERATION_OPERATION_REGISTRY_H_

#include "core/operation/operation.h"

#include <unordered_map>

namespace bdm {

/// A registry of operation implementations that can be scheduled for a
/// simulation. Since an operation can have multiple implementation (e.g. for
/// execution on CPU, CUDA, OpenCL), we need to register them separately.
struct OperationRegistry {
  /// Singleton class - returns the static instance
  static OperationRegistry *GetInstance();

  /// Gets the operation
  ///
  /// @param[in]  op_name  The operation's name
  ///
  /// @return     The operation pointer
  ///
  Operation *GetOperation(const std::string &op_name);

  /// Adds an operation implementation to the registry
  ///
  /// @param[in]  op_name    The operation's name
  /// @param[in]  target     The compute target
  /// @param      impl       The implementation for the compute target
  /// @param[in]  frequency  The frequency at which the operation is executed
  ///
  /// @return     Returns true when the operation is successfully added to
  ///             registry
  ///
  bool AddOperationImpl(const std::string &op_name, OpComputeTarget target,
                        OperationImpl *impl, size_t frequency = 1);

 private:
  /// The map containing the operations; accessible by their name
  std::unordered_map<std::string, Operation *> operations_;

  OperationRegistry();
  ~OperationRegistry();
};

/// A convenient macro to register a new operation implemented. To be used as:
/// BDM_REGISTER_OP(MyOp, "my operation", kCpu)
/// MyOp is required to have member: `static bool registered_`
#define BDM_REGISTER_OP(op, name, target)                                    \
  bool op::registered_ = OperationRegistry::GetInstance()->AddOperationImpl( \
      name, OpComputeTarget::target, new op());

/// \see BDM_REGISTER_OP
/// Adds parameter to specigy default execution frequency (\see
/// Operation::frequency_)
#define BDM_REGISTER_OP_WITH_FREQ(op, name, target, frequency)               \
  bool op::registered_ = OperationRegistry::GetInstance()->AddOperationImpl( \
      name, OpComputeTarget::target, new op(), frequency);

/// A convenient function to get a new operation from the registry by its name
inline Operation *NewOperation(const std::string &name) {
  return OperationRegistry::GetInstance()->GetOperation(name)->Clone();
}

/// A convenient macro to hide some of the boilerplate code from the user in
/// implementing new operations.
#define BDM_OP_HEADER(class_name) \
 private:                         \
  static bool registered_;        \
                                  \
 public:                          \
  class_name *Clone() override { return new class_name(*this); }

}  // namespace bdm

#endif  // CORE_OPERATION_OPERATION_REGISTRY_H_
