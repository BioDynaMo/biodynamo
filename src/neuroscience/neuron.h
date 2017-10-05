#ifndef NEUROSCIENCE_NEURON_H_
#define NEUROSCIENCE_NEURON_H_

#include "cell.h"
#include "simulation_object_util.h"

namespace bdm {

BDM_SIM_OBJECT(Neuron, Cell) {
  BDM_SIM_OBJECT_HEADER(NeuronExt, 1, daughters_, foo_, bar_);
 public:
  using SimBackend = typename TCompileTimeParam::SimulationBackend;
  using TNeurite = typename TCompileTimeParam::TNeurite;
  using TNeuron = typename TCompileTimeParam::TNeuron;
  NeuronExt() {}

 private:
   vec<SoPointer<TNeurite, SimBackend>> daughters_;
   vec<SoPointer<Self<SimBackend>, SimBackend>> foo_;
   vec<SoPointer<TNeuron, SimBackend>> bar_;
  // TNeuron* bar_;
};

BDM_SIM_OBJECT(SpecializedNeuron, Neuron) {
  BDM_SIM_OBJECT_HEADER(SpecializedNeuronExt, 1, me_);
 public:
  using SimBackend = typename TCompileTimeParam::SimulationBackend;
  SpecializedNeuronExt() {}

 private:
   vec<SoPointer<Self<SimBackend>, SimBackend>> me_;
};


}  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_H_
