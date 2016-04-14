/**
 * This file contains code generation customizations for class ECM.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (ECM_NATIVE and ECM_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physics/ecm.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %ECM_cx3d_shared_ptr()
  %cx3d_shared_ptr(ECM,
                   ini/cx3d/simulations/ECM,
                   cx3d::physics::ECM);
%enddef

%define %ECM_java()
  %java_defined_class(cx3d::physics::ECM,
                      ECM,
                      ECM,
                      ini.cx3d.simulations.ECM,
                      ini/cx3d/simulations/ECM);
%enddef

/**
 * apply customizations
 */
%ECM_cx3d_shared_ptr();
%ECM_java();
%typemap(javaimports) cx3d::physics::ECM %{
  import ini.cx3d.swig.biology.NeuriteElement;
  import ini.cx3d.swig.biology.SomaElement;
  import ini.cx3d.swig.biology.PhysicalSpine;
  import ini.cx3d.swig.biology.PhysicalBouton;
%}
