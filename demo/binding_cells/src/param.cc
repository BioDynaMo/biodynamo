#include "my_param.h"
#include "core/util/cpptoml.h"

namespace bdm {

const ModuleParamUid MyParam::kUid = ModuleParamUidGenerator::Get()->NewUid();

ModuleParam* MyParam::GetCopy() const { return new MyParam(*this); }

ModuleParamUid MyParam::GetUid() const { return kUid; }

void MyParam::AssignFromConfig(const std::shared_ptr<cpptoml::table>& config) {}

}  // namespace bdm
