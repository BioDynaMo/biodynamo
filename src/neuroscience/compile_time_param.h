#ifndef NEUROSCIENCE_COMPILE_TIME_PARAM_H_
#define NEUROSCIENCE_COMPILE_TIME_PARAM_H_

#include "neuroscience/neurite_element.h"
#include "neuroscience/neuron_soma.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

/// Default compile time parameter for neuroscience module.
/// Users need to specify the type of NeuronSoma and NeuriteElement they are
/// using.
template <typename TBackend = Soa>
struct DefaultCompileTimeParam {
  using NeuronSoma = ::bdm::experimental::neuroscience::NeuronSoma;
  using NeuriteElement = ::bdm::experimental::neuroscience::NeuriteElement;
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_COMPILE_TIME_PARAM_H_
