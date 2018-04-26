#ifndef NEUROSCIENCE_COMPILE_TIME_PARAM_H_
#define NEUROSCIENCE_COMPILE_TIME_PARAM_H_

#include "neuroscience/neurite.h"
#include "neuroscience/neuron.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

/// Default compile time parameter for neuroscience module.
/// Users need to specify the type of Neuron and Neurite they are using.
template <typename TBackend = Soa>
struct DefaultCompileTimeParam {
  using Neuron = ::bdm::experimental::neuroscience::Neuron;
  using Neurite = ::bdm::experimental::neuroscience::Neurite;
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_COMPILE_TIME_PARAM_H_
