#ifndef NEUROSCIENCE_NEURON_H_
#define NEUROSCIENCE_NEURON_H_

#include "cell.h"
#include "simulation_object_util.h"
#include <typeinfo> // TODO remove

namespace bdm {

BDM_SIM_OBJECT(Neuron, Cell) {
  BDM_SIM_OBJECT_HEADER(NeuronExt, 1, daughters_, foo_);
 public:
  using SimBackend = typename TCompileTimeParam::SimulationBackend;
  using TNeurite = typename TCompileTimeParam::TNeurite;
  using TNeuron = typename TCompileTimeParam::TNeuron;
  NeuronExt() {
    std::cout << typeid(TMostDerived<SimBackend>).name() << std::endl;
  }

 private:
  vec<SoPointer<TNeurite, SimBackend>> daughters_;
  // vec<SoPointer<TNeuron<SimBackend>, SimBackend>> foo_;
  vec<SoPointer<TMostDerived<SimBackend>, SimBackend>> foo_;
  // vec<TMostDerived<SimBackend>*> foo_;
  // using TNeuron instead of Self<Backend> in case a customized neuron is used
  //  vec<SoPointer<TNeuron, SimBackend>> bar_;
  // TNeuron* bar_;
};

BDM_SIM_OBJECT(SpecializedNeuron, Neuron) {
  BDM_SIM_OBJECT_HEADER(SpecializedNeuronExt, 1, me_);
 public:
  using SimBackend = typename TCompileTimeParam::SimulationBackend;
  SpecializedNeuronExt() { auto&& me = me_[kIdx].Get(); }

 private:
  vec<SoPointer<Self<SimBackend>, SimBackend>> me_;
};

}  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_H_
