#ifndef NEUROSCIENCE_COMPILE_TIME_PARAM_H_
#define NEUROSCIENCE_COMPILE_TIME_PARAM_H_

#include "neuroscience/neurite.h"
#include "neuroscience/neuron.h"

namespace bdm {
namespace neuroscience {

/// Default compile time parameter for neuroscience module.
/// Users need to specify the type of Neuron and Neurite they are using.
template <typename TBackend = Soa>
struct DefaultCompileTimeParam {
  using Neuron = ::bdm::neuroscience::Neuron;
  using Neurite = ::bdm::neuroscience::Neurite;
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_COMPILE_TIME_PARAM_H_
