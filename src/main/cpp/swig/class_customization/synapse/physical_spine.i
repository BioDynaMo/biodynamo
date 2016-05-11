/**
 * This file contains code generation customizations for class PhysicalSpine.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (PHYSICALSPINE_NATIVE and PHYSICALSPINE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/synapse/physical_spine.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"

%define %PhysicalSpine_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalSpine,
                   ini/cx3d/synapses/interfaces/PhysicalSpine,
                   cx3d::synapse::PhysicalSpine);
%enddef

%define %PhysicalSpine_java()
  %java_defined_class(cx3d::synapse::PhysicalSpine,
                      PhysicalSpine,
                      PhysicalSpine,
                      ini.cx3d.synapses.interfaces.PhysicalSpine,
                      ini/cx3d/synapses/interfaces/PhysicalSpine);
%enddef

%define %PhysicalSpine_native()
  %native_defined_class(cx3d::synapse::PhysicalSpine,
                    PhysicalSpine,
                    ini.cx3d.synapses.interfaces.PhysicalSpine,
                    PhysicalSpine,;);
%enddef

/**
 * apply customizations
 */
%PhysicalSpine_cx3d_shared_ptr();
#ifdef PHYSICALSPINE_NATIVE
  %PhysicalSpine_native();
#else
  %PhysicalSpine_java();
#endif
%typemap(javainterfaces) cx3d::synapse::PhysicalSpine "ini.cx3d.synapses.interfaces.PhysicalSpine"
%typemap(javaimports) cx3d::synapse::PhysicalSpine %{
  import ini.cx3d.swig.NativeStringBuilder;
  import ini.cx3d.swig.simulation.PhysicalObject;
%}
