/**
 * This file contains code generation customizations for class BiologicalBouton.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (BIOLOGICALBOUTON_NATIVE and BIOLOGICALBOUTON_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/synapse/biological_bouton.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %BiologicalBouton_cx3d_shared_ptr()
  %cx3d_shared_ptr(BiologicalBouton,
                   ini/cx3d/synapses/interfaces/BiologicalBouton,
                   cx3d::synapse::BiologicalBouton);
%enddef

%define %BiologicalBouton_native()
  %native_defined_class(cx3d::synapse::BiologicalBouton,
                      BiologicalBouton,
                      ini.cx3d.synapses.interfaces.BiologicalBouton,
                      BiologicalBouton,
                      ;);
%enddef

/**
 * apply customizations
 */
%BiologicalBouton_cx3d_shared_ptr();
%BiologicalBouton_native();
%typemap(javaimports) cx3d::synapse::BiologicalBouton "import ini.cx3d.swig.NativeStringBuilder;"
%typemap(javainterfaces) cx3d::synapse::BiologicalBouton "ini.cx3d.synapses.interfaces.BiologicalBouton"
