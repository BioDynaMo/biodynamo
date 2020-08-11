#include "core/operation/operation_registry.h"

namespace bdm {

OperationRegistry::~OperationRegistry() {
  for (auto &pair : operations_) {
    delete pair.second;
  }
}

OperationRegistry *OperationRegistry::GetInstance() {
  static OperationRegistry operation_registry;
  return &operation_registry;
}

Operation *OperationRegistry::GetOperation(const std::string &op_name) {
  auto search = operations_.find(op_name);
  if (search == operations_.end()) {
    std::string msg = "Operation not found in registry: " + op_name;
    Log::Fatal("OperationRegistry::GetOperation", msg);
  }
  return search->second;
}

bool OperationRegistry::AddOperationImpl(const std::string &op_name,
                                         OpComputeTarget target,
                                         OperationImpl *impl,
                                         size_t frequency) {
  auto *op = operations_[op_name];
  if (op == nullptr) {
    op = new Operation(op_name, frequency);
    operations_[op_name] = op;
  } else if (op->implementations_[target]) {
    Log::Fatal("OperationRegistry::AddOperationImpl", "Operation '", op_name,
               "' with implementation '", OpComputeTargetString(target),
               "' already exists in the registry!");
  }
  op->AddOperationImpl(target, impl);
  return true;
}

OperationRegistry::OperationRegistry() {}

}  // namespace bdm
