#include "neuroscience/neuron.h"
#include "gtest/gtest.h"

#include "compile_time_param.h"
#include "neuroscience/compile_time_param.h"

namespace bdm {

template <typename TBackend>
struct CompileTimeParam : public DefaultCompileTimeParam<TBackend>,
                          public neuroscience::DefaultCompileTimeParam<TBackend> {
 using TNeuron = SpecializedNeuron;
                          };

TEST(NeuronTest, Scalar) {
  Neuron neuron;
  SpecializedNeuron sneuron;
}

TEST(NeuronTest, Soa) {
  SoaNeuron neuron;
  SoaSpecializedNeuron sneuron;
}

}  // namespace bdm
