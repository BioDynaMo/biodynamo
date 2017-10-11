#ifndef NEUROSCIENCE_NEURON_H_
#define NEUROSCIENCE_NEURON_H_

#include <typeinfo>  // TODO remove
#include "cell.h"
#include "simulation_object_util.h"

namespace bdm {

BDM_SIM_OBJECT(Neuron, Cell) {
  BDM_SIM_OBJECT_HEADER(NeuronExt, 1, daughters_, foo_, aa_);

 public:
  using SimBackend = typename TCompileTimeParam::SimulationBackend;
  using TNeurite = typename TCompileTimeParam::TNeurite;

  NeuronExt() {
    std::cout << typeid(MostDerived).name() << std::endl;
    std::cout << "   Neurite Soa " << typeid(typename TNeurite::template Self<Soa>).name() << std::endl;
    std::cout << "   Neurite Sca " << typeid(typename TNeurite::template Self<Scalar>).name() << std::endl;
  }

 private:
  // vec<SoPointer<typename ToBackend<TNeurite, SimBackend>::type, SimBackend>>
  // daughters_;
  vec<SoPointer<TNeurite, SimBackend>> daughters_;
  // TNeurite* bar_;

  vec<SoPointer<typename TNeurite::template Self<Scalar>, SimBackend>> aa_;

  // vec<SoPointer<TNeuron<SimBackend>, SimBackend>> foo_;
  vec<SoPointer<MostDerived, SimBackend>> foo_;
  // vec<MostDerived*> foo_;
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
  vec<SoPointer<MostDerived, SimBackend>> me_;
};

}  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_H_
