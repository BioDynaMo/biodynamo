/**
 * SWIG modules that use the class simply include it:
 * %include "class_customization/synapse/connection_maker.i"
 */

%typemap(javaimports) cx3d::synapse::ConnectionMaker %{
  import ini.cx3d.swig.biology.ECM;
%}
