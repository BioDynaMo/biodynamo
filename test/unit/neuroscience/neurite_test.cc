#include "neuroscience/neurite.h"
#include "gtest/gtest.h"

#include "compile_time_param.h"
#include "neuroscience/compile_time_param.h"

namespace bdm {

template <typename TBackend>
struct CompileTimeParam
    : public DefaultCompileTimeParam<TBackend>,
      public neuroscience::DefaultCompileTimeParam<TBackend> {};

TEST(NeuriteTest, Scalar) {
  Neurite neurite;
  // neurite.BifurcationPermitted();
}

TEST(NeuriteTest, Soa) { SoaNeurite neurite; }

}  // namespace bdm
