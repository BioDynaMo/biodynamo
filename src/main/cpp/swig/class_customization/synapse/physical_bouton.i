/**
 * This file contains code generation customizations for class PhysicalBouton.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (PHYSICALBOUTON_NATIVE and PHYSICALBOUTON_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/synapse/physical_bouton.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"

%define %PhysicalBouton_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalBouton,
                   ini/cx3d/synapses/interfaces/PhysicalBouton,
                   cx3d::synapse::PhysicalBouton);
%enddef

%define %PhysicalBouton_java()
  %java_defined_class(cx3d::synapse::PhysicalBouton,
                      PhysicalBouton,
                      PhysicalBouton,
                      ini.cx3d.synapses.interfaces.PhysicalBouton,
                      ini/cx3d/synapses/interfaces/PhysicalBouton);
%enddef

%define %PhysicalBouton_native()
  %native_defined_class(cx3d::synapse::PhysicalBouton,
                    PhysicalBouton,
                    ini.cx3d.synapses.interfaces.PhysicalBouton,
                    PhysicalBouton,;);
%enddef

/**
 * apply customizations
 */
%PhysicalBouton_cx3d_shared_ptr();
#ifdef PHYSICALBOUTON_NATIVE
  %PhysicalBouton_native();
#else
  %PhysicalBouton_java();
#endif
%typemap(javainterfaces) cx3d::synapse::PhysicalBouton "ini.cx3d.synapses.interfaces.PhysicalBouton"
%typemap(javaimports) cx3d::synapse::PhysicalBouton %{
  import ini.cx3d.swig.NativeStringBuilder;
  import ini.cx3d.swig.simulation.PhysicalObject;
%}
