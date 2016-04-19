/**
 * SWIG modules that use the class simply include it:
 * %include "class_customization/synapse/test_synapses.i"
 */

%typemap(javaimports) cx3d::synapse::TestSynapses %{
  import ini.cx3d.swig.physics.ECM;
%}
