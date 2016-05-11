/**
 * This file contains code generation customizations for class BiologicalSpine.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (BIOLOGICALSPINE_NATIVE and BIOLOGICALSPINE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/synapse/biological_spine.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %BiologicalSpine_cx3d_shared_ptr()
  %cx3d_shared_ptr(BiologicalSpine,
                   ini/cx3d/synapses/interfaces/BiologicalSpine,
                   cx3d::synapse::BiologicalSpine);
%enddef

%define %BiologicalSpine_native()
  %native_defined_class(cx3d::synapse::BiologicalSpine,
                      BiologicalSpine,
                      ini.cx3d.synapses.interfaces.BiologicalSpine,
                      BiologicalSpine,
                      ;);
%enddef

/**
 * apply customizations
 */
%BiologicalSpine_cx3d_shared_ptr();
%BiologicalSpine_native();
%typemap(javaimports) cx3d::synapse::BiologicalSpine "import ini.cx3d.swig.NativeStringBuilder;"
%typemap(javainterfaces) cx3d::synapse::BiologicalSpine "ini.cx3d.synapses.interfaces.BiologicalSpine"
