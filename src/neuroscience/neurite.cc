#include "neuroscience/neuron.h"

namespace bdm {
namespace neuroscience {

const BmEvent gNeuriteElongation = UniqueBmEventFactory::Get()->NewUniqueBmEvent();
const BmEvent gNeuriteBranching = UniqueBmEventFactory::Get()->NewUniqueBmEvent();
const BmEvent gNeuriteBifurcation = UniqueBmEventFactory::Get()->NewUniqueBmEvent();
const BmEvent gNeuriteSideCylinderExtension = UniqueBmEventFactory::Get()->NewUniqueBmEvent();

}  // namespace neuroscience
}  // namespace bdm
