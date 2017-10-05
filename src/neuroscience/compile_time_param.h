#ifndef NEUROSCIENCE_COMPILE_TIME_PARAM_H_
#define NEUROSCIENCE_COMPILE_TIME_PARAM_H_

#include "neuroscience/neuron.h"
#include "neuroscience/neurite.h"

namespace bdm {
namespace neuroscience {

// TODO
template <typename TBackend = Soa>
struct DefaultCompileTimeParam {
  using TNeuron = Neuron;
  using TNeurite = Neurite;
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_COMPILE_TIME_PARAM_H_
