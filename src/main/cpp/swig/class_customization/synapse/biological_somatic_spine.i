/**
 * This file contains code generation customizations for class BiologicalSomaticSpine.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (BIOLOGICALSOMATICSPINE_NATIVE and BIOLOGICALSOMATICSPINE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/synapse/biological_somatic_spine.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %BiologicalSomaticSpine_cx3d_shared_ptr()
  %cx3d_shared_ptr(BiologicalSomaticSpine,
                   ini/cx3d/synapses/interfaces/BiologicalSomaticSpine,
                   cx3d::synapse::BiologicalSomaticSpine);
%enddef

%define %BiologicalSomaticSpine_native()
  %native_defined_class(cx3d::synapse::BiologicalSomaticSpine,
                      BiologicalSomaticSpine,
                      ini.cx3d.synapses.interfaces.BiologicalSomaticSpine,
                      BiologicalSomaticSpine,
                      ;);
%enddef

/**
 * apply customizations
 */
%BiologicalSomaticSpine_cx3d_shared_ptr();
%BiologicalSomaticSpine_native();
%typemap(javaimports) cx3d::synapse::BiologicalSomaticSpine "import ini.cx3d.swig.NativeStringBuilder;"
%typemap(javainterfaces) cx3d::synapse::BiologicalSomaticSpine "ini.cx3d.synapses.interfaces.BiologicalSomaticSpine"
