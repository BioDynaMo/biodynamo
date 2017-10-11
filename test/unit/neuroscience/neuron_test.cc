#include "neuroscience/neuron.h"
#include "gtest/gtest.h"

#include "compile_time_param.h"
#include "neuroscience/compile_time_param.h"

namespace bdm {

BDM_SIM_OBJECT(SpecializedNeurite, Neurite) {
  BDM_SIM_OBJECT_HEADER(SpecializedNeuriteExt, 1, foo_);
public:
  SpecializedNeuriteExt() {}
private:
  vec<int> foo_;
};

template <typename TBackend>
struct CompileTimeParam
    : public DefaultCompileTimeParam<TBackend>,
      public neuroscience::DefaultCompileTimeParam<TBackend> {
  using TNeuron = neuroscience::SpecializedNeuron;
  using TNeurite = SpecializedNeurite;
};

namespace neuroscience {

TEST(NeuronTest, Scalar) {
  Neurite neurite;
  Neuron neuron;
  typename Neuron::template Self<Scalar> neuron1;
  SpecializedNeuron sneuron;
}

TEST(NeuronTest, Soa) {
  SoaNeuron neuron;
  SoaSpecializedNeuron sneuron;
  typename SpecializedNeuron::template Self<Soa> soan;
  typename CompileTimeParam<>::TNeuron soan1;
}

}  // namespace neuroscience
}  // namespace bdm
