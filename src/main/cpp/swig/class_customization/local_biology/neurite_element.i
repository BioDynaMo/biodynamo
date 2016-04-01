/**
 * This file contains code generation customizations for class NeuriteElement.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (NEURITEELEMENT_NATIVE and NEURITEELEMENT_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/local_biology/neurite_element.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"

%define %NeuriteElement_cx3d_shared_ptr()
  %cx3d_shared_ptr(NeuriteElement,
                   ini/cx3d/localBiology/NeuriteElement,
                   cx3d::local_biology::NeuriteElement);
%enddef

%define %NeuriteElement_java()
  %java_defined_class(cx3d::local_biology::NeuriteElement,
                      NeuriteElement,
                      NeuriteElement,
                      ini.cx3d.localBiology.NeuriteElement,
                      ini/cx3d/localBiology/NeuriteElement);
%enddef

%define %NeuriteElement_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::local_biology::NeuriteElement>,
                   NeuriteElement,
                   ini.cx3d.localBiology.NeuriteElement);
%enddef

/**
 * apply customizations
 */
%NeuriteElement_cx3d_shared_ptr();
%NeuriteElement_java();
%NeuriteElement_stdlist();
%typemap(javaimports) cx3d::local_biology::NeuriteElement "import ini.cx3d.swig.NativeStringBuilder; import ini.cx3d.swig.biology.LocalBiologyModule;"
