/**
 * This file contains code generation customizations for class PhysicalSomaticSpine.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (PHYSICALSOMATICSPINE_NATIVE and PHYSICALSOMATICSPINE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/synapse/physical_somatic_spine.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"

%define %PhysicalSomaticSpine_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalSomaticSpine,
                   ini/cx3d/synapses/interfaces/PhysicalSomaticSpine,
                   cx3d::synapse::PhysicalSomaticSpine);
%enddef

%define %PhysicalSomaticSpine_java()
  %java_defined_class(cx3d::synapse::PhysicalSomaticSpine,
                      PhysicalSomaticSpine,
                      PhysicalSomaticSpine,
                      ini.cx3d.synapses.interfaces.PhysicalSomaticSpine,
                      ini/cx3d/synapses/interfaces/PhysicalSomaticSpine);
%enddef

%define %PhysicalSomaticSpine_native()
  %native_defined_class(cx3d::synapse::PhysicalSomaticSpine,
                    PhysicalSomaticSpine,
                    ini.cx3d.synapses.interfaces.PhysicalSomaticSpine,
                    PhysicalSomaticSpine,;);
%enddef

/**
 * apply customizations
 */
%PhysicalSomaticSpine_cx3d_shared_ptr();
#ifdef PHYSICALSPINE_NATIVE
  %PhysicalSomaticSpine_native();
#else
  %PhysicalSomaticSpine_java();
#endif
%typemap(javainterfaces) cx3d::synapse::PhysicalSomaticSpine "ini.cx3d.synapses.interfaces.PhysicalSomaticSpine"
%typemap(javaimports) cx3d::synapse::PhysicalSomaticSpine %{
  import ini.cx3d.swig.NativeStringBuilder;
  import ini.cx3d.swig.physics.PhysicalObject;
%}
